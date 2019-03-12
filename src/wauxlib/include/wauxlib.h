#ifndef wauxlib_h
#define wauxlib_h

#include <wren.h>

typedef struct WauxlibLoaderCtx
{

} WauxlibLoaderCtx;

char* wauxlibLoader(WrenVM *vm,
                    const char *name,
                    void* loaderCtx);

//TODO: Move this to a config header file
#define MAX_METHODS_PER_CLASS 14
#define MAX_CLASSES_PER_MODULE 6
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

  #ifdef WAUXLIB_INLINE_MODULE_SOURCE
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
#define SENTINEL_MODULE {NULL, NULL, { SENTINEL_CLASS } }

#define MODULE(name) { #name, &name##ModuleSource, {
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
typedef struct
{
    
} WauxlibBindCtx;

#if 0
void defaultBinderAddModule(WauxlibBindCtx *binderCtx, 
                            char *moduleName,
                            WrenLoadModuleFn moduleLoadModuleFn;
                            void *moduleLoadModuleFnCtx;
                            WrenBindForeignMethodFn moduleBindForeignMethodFn; 
                            WrenBindForeignClassFn moduleBindForeignClassFn; 
                            void *moduleBindForeignCtx);
#endif


#endif