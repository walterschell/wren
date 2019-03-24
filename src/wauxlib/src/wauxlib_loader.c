#include <wauxlib.h>
#include <stdio.h>
#include <string.h>

char* wauxlibLoader(WrenVM *vm,
                    const char *name,
                    void* loaderCtx)
{
    (void) vm;
    WauxlibLoaderCtx *ctx = loaderCtx;
    //TODO: Use path logic from cli here
    char path[4096];

    // Try Wren source module
    sprintf(path,"%s/%s.wren", (char *) ctx->wren_module_path, name);
    FILE *fh = fopen(path, "rb");
    if (NULL != fh)
    {
        fseek(fh, 0, SEEK_END);
        long file_size = ftell(fh);
        fseek(fh, 0, SEEK_SET);
        char *result = malloc(file_size+1);
        if (NULL == result)
        {
            fclose(fh);
            return NULL;
        }
        fread(result, file_size, 1, fh);
        result[file_size] = '\0';
        fclose(fh);
        return result;
    }
    // Try checking if a .so exists instead
#ifdef WAUXLIB_DYNAMIC_PLUGINS
    //TODO: Use a better method to check if file exists
    sprintf(path,"%s/libwren_module_%s.so", ctx->wren_module_path, name);
    fh = fopen(path, "rb");
    if (NULL != fh)
    {
        fclose(fh);
        return strdup(path);
    }
#endif //WAUXLIB_DYNAMIC_PLUGINS
    return NULL;
}