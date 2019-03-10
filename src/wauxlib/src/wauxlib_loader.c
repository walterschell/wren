#include <wauxlib.h>
char* wauxlibLoader(WrenVM *vm,
                    const char *name,
                    void* loaderCtx)
{
    (void) vm;
    (void) name;
    (void) loaderCtx;
    return NULL;
}