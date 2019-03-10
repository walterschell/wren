#ifndef wauxlib_h
#define wauxlib_h

#include <wren.h>

char* wauxlibLoader(WrenVM *vm,
                    const char *name,
                    void* loaderCtx);

#endif