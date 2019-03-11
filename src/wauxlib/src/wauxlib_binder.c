#include <wauxlib.h>
#include <string.h>

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

