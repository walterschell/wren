#ifndef wauxlib_h
#define wauxlib_h
#include <stdint.h>
#include <wren.h>
#ifndef WUAXLIB_BINDER
#define WUAXLIB_BINDER
#endif
#ifdef WAUXLIB_BINDER


//TODO: Move this to a config header file
# define MAX_METHODS_PER_CLASS 14
# define MAX_CLASSES_PER_MODULE 6
typedef struct
{
  bool isStatic;
  const char* signature;
  WrenForeignMethodFn method;
} MethodRegistry;

// Describes one class in a built-in module.
typedef struct
{
  const char* name;

  MethodRegistry methods[MAX_METHODS_PER_CLASS];
} ClassRegistry;

// Describes one built-in module.
typedef struct
{
  // The name of the module.
  const char* name;

// WUAXLIB_MODULE_INLINE_SOURCE protects a construct that was in the cli but is not yet supported
#ifdef WAUXLIB_MODULE_INLINE_SOURCE
  // Pointer to the string containing the source code of the module. We use a
  // pointer here because the string variable itself is not a constant
  // expression so can't be used in the initializer below.
  const char **source;
#endif
  ClassRegistry classes[MAX_CLASSES_PER_MODULE];
} ModuleRegistry;

// To locate foreign classes and modules, we build a big directory for them in
// static data. The nested collection initializer syntax gets pretty noisy, so
// define a couple of macros to make it easier.
#define SENTINEL_METHOD { false, NULL, NULL }
#define SENTINEL_CLASS { NULL, { SENTINEL_METHOD } }
#ifdef WAUXLIB_MODULE_INLINE_SOURCE
# define SENTINEL_MODULE {NULL, NULL, { SENTINEL_CLASS } }
#else
# define SENTINEL_MODULE {NULL, {SENTINEL_CLASS}}
#endif
#ifdef WAUXLIB_MODULE_INLINE_SOURCE
# define MODULE(name) { #name, &name##ModuleSource, {
#else
# define MODULE(name) { #name, {
#endif
#define END_MODULE SENTINEL_CLASS } },

#define CLASS(name) { #name, {
#define END_CLASS SENTINEL_METHOD } },

#define METHOD(signature, fn) { false, signature, fn },
#define STATIC_METHOD(signature, fn) { true, signature, fn },
#define FINALIZER(fn) { true, "<finalize>", (WrenForeignMethodFn)fn },

WrenForeignMethodFn wauxlibBindForeignMethod(
                        WrenVM* vm,
                        const char* moduleName,
                        const char* className,
                        bool isStatic,
                        const char* signature,
                        void* binderCtx);


WrenForeignClassMethods wauxlibBindForeignClass(
                        WrenVM* vm,
                        const char* moduleName,
                        const char* className,
                        void *binderCtx);


typedef struct WauxlibBinderCtx WauxlibBinderCtx;
WauxlibBinderCtx *defaultBinderNew();
void defaultBinderDelete(WauxlibBinderCtx *binderCtx);

#ifdef WAUXLIB_DYNAMIC_PLUGINS
typedef void (*WauxlibPluginCloseFn)(void *);
#endif

typedef struct WauxlibModule
{
  WrenLoadModuleFn loadModuleFn;
  void *loadModuleFnCtx;
  WrenBindForeignMethodFn bindForeignMethodFn; 
  WrenBindForeignClassFn bindForeignClassFn; 
  void *bindForeignCtx;
#ifdef WAUXLIB_DYNAMIC_PLUGINS
  void *plugin_handle;
  WauxlibPluginCloseFn plugin_close;
#endif
} WauxlibModule;

bool defaultBinderAddModuleRaw(WauxlibBinderCtx *binderCtx,
                                      const char *moduleName,
                                      WauxlibModule *module);
bool defaultBinderAddModuleSimple(WauxlibBinderCtx *binderCtx, 
                            const char *moduleName,
                            const char *moduleSource,
                            ModuleRegistry *moduleBindForeignCtx);

char * defaultBinderLoadModuleFn(WrenVM *vm, const char *name, void *loaderCtx);
WrenForeignMethodFn defaultBinderBindForeignMethodFn(WrenVM *vm,
                                                     const char *module,
                                                     const char *className,
                                                     bool isStatic,
                                                     const char *signature,
                                                     void *binderCtx);

WrenForeignClassMethods defaultBinderBindForeignClassFn(WrenVM *vm,
                                                        const char *module,
                                                        const char *className,
                                                        void *binderCtx);

//TODO: Function to allocate a binder ctx and set all of the pointers in a config


typedef struct 
{
  uint32_t wren_version_number;
  WrenLoadModuleFn loadModuleFn;
  void *loadModuleFnCtx;
  WrenBindForeignMethodFn bindForeignMethodFn; 
  WrenBindForeignClassFn bindForeignClassFn; 
  void *bindForeignCtx;
} WrenPluginInfo;


bool wauxlibRegisterPlugin(WauxlibBinderCtx *binderCtx,
                           const char *moduleName,
                           WrenPluginInfo *pluginInfo,
                           void *plugin_handle,
                           WauxlibPluginCloseFn plugin_close);
#endif //WAUXLIB_BINDER

#ifdef WAUXLIB_LOADER
typedef struct WauxlibLoaderCtx
{
  char *wren_module_path;
} WauxlibLoaderCtx;

char* wauxlibLoader(WrenVM *vm,
                    const char *name,
                    void* loaderCtx);

#endif //WAUXLIB_LOADER
#endif