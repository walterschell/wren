#include <wauxlib.h>
#include <string.h>
#include <khash.h>
#include <stdio.h>
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
  WauxlibBinderCtx *result = malloc(sizeof(WauxlibBinderCtx));
  if (NULL == result)
  {
    return result;
  }
  result->modules = kh_init(modules_map);
  return result;
}
void defaultBinderDelete(WauxlibBinderCtx *binderCtx)
{
  if (NULL == binderCtx)
  {
    return;
  }
  WauxlibModule *module;
  kh_foreach_value(binderCtx->modules, module, {free(module);});
  kh_destroy(modules_map, binderCtx->modules);
  free(binderCtx);
}

bool defaultBinderAddModule(WauxlibBinderCtx *binderCtx, 
                            char *moduleName,
                            WrenLoadModuleFn moduleLoadModuleFn,
                            void *moduleLoadModuleFnCtx,
                            WrenBindForeignMethodFn moduleBindForeignMethodFn,
                            WrenBindForeignClassFn moduleBindForeignClassFn,
                            void *moduleBindForeignCtx)
{
  WauxlibModule *module = malloc(sizeof(WauxlibModule));
  if (NULL == module)
  {
    return false;
  }
  module->loadModuleFn = moduleLoadModuleFn;
  module->loadModuleFnCtx = moduleLoadModuleFnCtx;
  module->bindForeignMethodFn = moduleBindForeignMethodFn;
  module->bindForeignClassFn = moduleBindForeignClassFn;
  module->bindForeignCtx = moduleBindForeignCtx;
  int status;
  khint_t itor = kh_put(modules_map, binderCtx->modules, moduleName, &status);
  if (status < 0)
  {
    printf("Unable to put %s\n", moduleName);
    free(module);
    return false;
  }
  printf("Module %s stored in default binder\n", moduleName);
  kh_value(binderCtx->modules, itor) = module;
  return true;
}

char * defaultBinderLoadModuleFn(WrenVM *vm, const char *name, void *loaderCtx)
{
  WauxlibBinderCtx *ctx = loaderCtx;
  khint_t itor = kh_get(modules_map, ctx->modules, name);
  if (kh_end(ctx->modules) == itor)
  {
    printf("module %s not found in default binder\n", name);
    //TODO: Add code to search path for modules here
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