
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>

static bool loaded = false;

__attribute__((constructor)) /* Entry point when injected */
void load(void) {
    printf("source-offset-finder injected!\n");

    loaded = true;
}

__attribute__((destructor)) /* Entry point when unloaded */
void unload() {
    if (!loaded)
        return;

    printf("source-offset-finder unloaded.\n\n");
}

void self_unload(void) {
    void* self = dlopen("liboffsetfinder.so", RTLD_LAZY | RTLD_NOLOAD);

    /* Close the call we just made to dlopen() */
    dlclose(self);

    /* Close the call our injector made */
    dlclose(self);
}
