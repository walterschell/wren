#include <wauxlib.h>
#include <string.h>
#include <khash.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

KHASH_MAP_INIT_STR(modules_map, WauxlibModule *);
struct WauxlibBinderCtx
{
  khash_t(modules_map) *modules;
  WrenLoadModuleFn loader;
  void *loaderCtx;
};

#ifdef WAUXLIB_LOADER
static char default_search_path[4096];

//TODO: Use something more portable
__attribute__((constructor)) void fill_default_search_path()
{
 char cwd[2048];
 getcwd(cwd, sizeof(cwd));
 sprintf(default_search_path, "%s/modules", cwd);
}

static WauxlibLoaderCtx defaultLoaderCtx = {
  .wren_module_path = default_search_path,
};

#endif
static ModuleRegistry* findModule(const char* name, ModuleRegistry* modules)
{
  for (int i = 0; modules[i].name != NULL; i++)
  {
    if (strcmp(name, modules[i].name) == 0) return &modules[i];
  }

  return NULL;
}

static ClassRegistry* findClass(ModuleRegistry* module, const char* name)
{
  for (int i = 0; module->classes[i].name != NULL; i++)
  {
    if (strcmp(name, module->classes[i].name) == 0) return &module->classes[i];
  }

  return NULL;
}


// Looks for a method with [signature] in [clas].
static WrenForeignMethodFn findMethod(ClassRegistry* clas,
                                      bool isStatic, const char* signature)
{
  for (int i = 0; clas->methods[i].signature != NULL; i++)
  {
    MethodRegistry* method = &clas->methods[i];
    if (isStatic == method->isStatic &&
        strcmp(signature, method->signature) == 0)
    {
      return method->method;
    }
  }

  return NULL;
}

WrenForeignMethodFn wauxlibBindForeignMethod(
    WrenVM* vm,
    const char* moduleName,
    const char* className,
    bool isStatic,
    const char* signature,
    void* binderCtx)
{
  (void) vm; //Unused parameter
  ModuleRegistry *modules = binderCtx;
  // TODO: Assert instead of return NULL?
  ModuleRegistry* module = findModule(moduleName, modules);
  if (module == NULL) return NULL;

  ClassRegistry* clas = findClass(module, className);
  if (clas == NULL) return NULL;

  return findMethod(clas, isStatic, signature);    
}

WrenForeignClassMethods wauxlibBindForeignClass(
    WrenVM* vm,
    const char* moduleName,
    const char* className,
    void* binderCtx)
{
  (void) vm; //Ignore unused parameter
  ModuleRegistry *modules = binderCtx;
  WrenForeignClassMethods methods = { NULL, NULL };

  ModuleRegistry* module = findModule(moduleName, modules);
  if (module == NULL) return methods;

  ClassRegistry* clas = findClass(module, className);
  if (clas == NULL) return methods;

  methods.allocate = findMethod(clas, true, "<allocate>");
  methods.finalize = (WrenFinalizerFn)findMethod(clas, true, "<finalize>");

  return methods;
}

WauxlibBinderCtx *defaultBinderNew()
{
  WauxlibBinderCtx *result = calloc(1,sizeof(WauxlibBinderCtx));
  if (NULL == result)
  {
    return result;
  }
  result->modules = kh_init(modules_map);
#ifdef WAUXLIB_LOADER
  result->loader = wauxlibLoader;
  result->loaderCtx = &defaultLoaderCtx;
#endif

  return result;
}
void defaultBinderDelete(WauxlibBinderCtx *binderCtx)
{
  if (NULL == binderCtx)
  {
    return;
  }
  WauxlibModule *module;
  kh_foreach_value(binderCtx->modules, module,
    {
#ifdef WAUXLIB_DYNAMIC_PLUGINS
      if (NULL != module->plugin_close)
      {
        module->plugin_close(module->plugin_handle);
      }
#endif
      free(module);
    }
  );
  kh_destroy(modules_map, binderCtx->modules);
  free(binderCtx);
}

bool defaultBinderAddModuleRaw(WauxlibBinderCtx *binderCtx,
                                      const char *moduleName,
                                      WauxlibModule *module)
{
  int status;
  khint_t itor = kh_put(modules_map, binderCtx->modules, moduleName, &status);
  if (status < 0)
  {
    return false;
  }
  kh_value(binderCtx->modules, itor) = module;
  return true;
}


bool defaultBinderAddModuleSimple(WauxlibBinderCtx *binderCtx, 
                            const char *moduleName,
                            const char *moduleSource,
                            ModuleRegistry *moduleBindForeignCtx)
{
  WauxlibModule *module = calloc(1, sizeof(WauxlibModule));
  if (NULL == module)
  {
    return false;
  }
  module->loadModuleFnCtx = (void *) moduleSource;
  module->bindForeignCtx = moduleBindForeignCtx;
  if (defaultBinderAddModuleRaw(binderCtx, moduleName, module))
  {
    return true;
  }
  free(module);
  return false;
}
#ifdef WAUXLIB_DYNAMIC_PLUGINS
static void close_plugin(void *plugin)
{
  dlclose(plugin);
}
#endif
char * defaultBinderLoadModuleFn(WrenVM *vm, const char *name, void *loaderCtx)
{
  WauxlibBinderCtx *ctx = loaderCtx;
  khint_t itor = kh_get(modules_map, ctx->modules, name);
  if (kh_end(ctx->modules) == itor)
  {
    char *result = NULL;
    if (NULL != ctx->loader)
    {
      result = ctx->loader(vm, name, ctx->loaderCtx);
      if (NULL == result)
      {
        return NULL;
      }
#ifdef WAUXLIB_DYNAMIC_PLUGINS
      // If result starts with . or / interpret as path to shared object
      if (result[0] == '.' || result[0] == '/')
      {
        void *plugin = dlopen(result, RTLD_LAZY);
        free(result);
        if (NULL == plugin)
        {
          return NULL;
        }
        WrenPluginInfo *plugin_info = dlsym(plugin, "wren_mod_config");
        if (NULL == plugin_info)
        {
          dlclose(dlsym);
          return NULL;
        }
        else
        {
          if (WREN_VERSION_NUMBER != plugin_info->wren_version_number)
          {
            dlclose(dlsym);
            return NULL;
          }
          else
          {
            // If successful registration we should be good to call ourselves again              
            if (wauxlibRegisterPlugin(ctx, name, plugin_info, plugin, close_plugin))
            {
              return defaultBinderLoadModuleFn(vm, name, loaderCtx);
            }
            else
            {
              return NULL;
            }   
          }
        }
      }
      else
#endif
      {
        return result;
      }
    }
    
    return NULL;
  }
  WauxlibModule *module = kh_value(ctx->modules, itor);
  if (NULL == module->loadModuleFn)
  {
    return strdup(module->loadModuleFnCtx); 
  }
  return module->loadModuleFn(vm, name, module->loadModuleFnCtx);
}

WrenForeignMethodFn defaultBinderBindForeignMethodFn(WrenVM *vm,
                                                         const char *moduleName,
                                                         const char *className,
                                                         bool isStatic,
                                                         const char *signature,
                                                         void *binderCtx)
{
  
  WauxlibBinderCtx *ctx = binderCtx;
  khint_t itor = kh_get(modules_map, ctx->modules, moduleName);
  if (kh_end(ctx->modules) == itor)
  {
    return (WrenForeignMethodFn) {0};
  }
  WauxlibModule *module = kh_value(ctx->modules, itor);
  if (NULL == module->bindForeignMethodFn)
  {
    return wauxlibBindForeignMethod(vm, moduleName, className, isStatic, signature, module->bindForeignCtx);
  }
  return module->bindForeignMethodFn(vm, moduleName, className, isStatic, signature, module->bindForeignCtx);
}
WrenForeignClassMethods defaultBinderBindForeignClassFn(WrenVM *vm,
                                                         const char *moduleName,
                                                         const char *className,
                                                         void *binderCtx)
{
  WauxlibBinderCtx *ctx = binderCtx;
  khint_t itor = kh_get(modules_map, ctx->modules, moduleName);
  if (kh_end(ctx->modules) == itor)
  {
    return (WrenForeignClassMethods) {0};
  }
  WauxlibModule *module = kh_value(ctx->modules, itor);
  if (NULL == module->bindForeignClassFn)
  {
    return wauxlibBindForeignClass(vm, moduleName, className, module->bindForeignCtx);
  }
  return module->bindForeignClassFn(vm, moduleName, className, module->bindForeignCtx);
}

bool wauxlibRegisterPlugin(WauxlibBinderCtx *binderCtx,
                           const char *moduleName,
                           WrenPluginInfo *pluginInfo,
                           void *plugin_handle,
                           WauxlibPluginCloseFn plugin_close)
{
  
  struct WauxlibModule *module = calloc(1, sizeof(WauxlibModule));
  module->loadModuleFn = pluginInfo->loadModuleFn;
  module->loadModuleFnCtx = pluginInfo->loadModuleFnCtx;
  module->bindForeignMethodFn = pluginInfo->bindForeignMethodFn;
  module->bindForeignClassFn = pluginInfo->bindForeignClassFn;
  module->bindForeignCtx = pluginInfo->bindForeignCtx;
  module->plugin_handle = plugin_handle;
  module->plugin_close = plugin_close;
  if (defaultBinderAddModuleRaw(binderCtx, moduleName, module))
  {
    return true;
  }
  free(module);
  return false;
}
