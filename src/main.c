
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "sdk.h"

/* Defines that should change depending on the game */
#define CLIENT_SO   "./tf/bin/linux64/client.so"
#define LOG_PATH    "/tmp/source-netvar-dumper.log"
#define ICLIENT_STR "VClient017"

#define PRINT_TO_FILE(...)        \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(log_fd, __VA_ARGS__);

/*----------------------------------------------------------------------------*/

static bool loaded = false;

static FILE* log_fd = 0;

static void* h_client       = NULL; /* client.so handler */
static BaseClient* i_client = NULL; /* IBaseClientDLL* */

/*----------------------------------------------------------------------------*/

/* Called from globals_init for getting IBaseClientDLL interface */
void* get_interface(void* handle, const char* name) {
    typedef void* (*fn)(const char*, int*);
    static fn CreateInterface = NULL;

    if (!handle) {
        fprintf(stderr, "get_interface: invalid handle for interface %s\n",
                name);
        return NULL;
    }

    /* Initialize once */
    if (!CreateInterface) {
        CreateInterface = (fn)dlsym(handle, "CreateInterface");

        /* dlsym failed */
        if (!CreateInterface) {
            fprintf(stderr, "get_interface: dlsym couldn't get "
                            "CreateInterface\n");
            return NULL;
        }
    }

    return CreateInterface(name, NULL);
}

/* Called from load (entry point) */
static bool globals_init(void) {
    /* Handlers */
    h_client = dlopen(CLIENT_SO, RTLD_LAZY | RTLD_NOLOAD);
    if (!h_client) {
        fprintf(stderr, "globals_init: can't open client.so\n");
        return false;
    }

    /* Interfaces */
    i_client = (BaseClient*)get_interface(h_client, ICLIENT_STR);
    if (!i_client || !i_client->vt) {
        fprintf(stderr, "globals_init: couldn't load i_client\n");
        return false;
    }

    return true;
}

/* Called from netvars_init and from itself (recursive) */
static void dump_client_class(const char* network_name, RecvTable* table,
                              uint32_t offset) {
    for (int i = 0; i < table->propsCount; i++) {
        /* Store address of prop at index i in the table->props array */
        const RecvProp* prop = &table->props[i];

        if (!prop || !prop->varName || isdigit(prop->varName[0]))
            continue;

        /* We don't care about base classes */
        if (!strcmp(prop->varName, "baseclass"))
            continue;

        /* If this is a datatable, there are more props so we dump again
         * recursively with the new offset */
        if (prop->recvType == DATATABLE && prop->dataTable &&
            prop->dataTable->tableName[0] == 'D')
            dump_client_class(network_name, prop->dataTable,
                              offset + prop->offset);

        /* Print our shit to stdout and file */
        PRINT_TO_FILE("%s->%s : 0x%X\n", network_name, prop->varName,
                      offset + prop->offset);
    }
}

/* Called from load (entry point) */
static void netvars_dump(void) {
    /* Iterate ClientClass linked list */
    for (ClientClass* client_class  = i_client->vt->GetAllClasses(i_client);
         client_class; client_class = client_class->next)
        /* Make sure it has a valid RecvTable pointer */
        if (client_class)
            /* Starting offset is 0, will change with recursion */
            dump_client_class(client_class->networkName,
                              client_class->recvTable, 0);
}

/*----------------------------------------------------------------------------*/

void self_unload(void) {
    /* NOTE: See 8dcc/tf2-cheat for more info on why this might crash */
    void* self = dlopen("libnetvardumper.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!self)
        return;

    /* Close the call we just made to dlopen() */
    dlclose(self);

    /* Close the call our injector made */
    dlclose(self);
}

__attribute__((constructor)) /* Entry point when injected */
void load(void) {
    /* Get file descriptor for /tmp/source-netvar-dumper.log */
    log_fd = fopen(LOG_PATH, "w+");

    PRINT_TO_FILE("======================================================\n"
                  "https://github.com/8dcc/source-netvar-dumper\n\n"
                  "Dumping offsets for \"" CLIENT_SO "\"\n"
                  "======================================================\n\n");

    if (!globals_init()) {
        fprintf(stderr, "load: error loading globals, aborting\n");
        self_unload();
    }

    netvars_dump();

    puts("[source-netvar-dumper] All done! Check " LOG_PATH);

    fclose(log_fd);

    loaded = true;

    puts("Uninjecting...");
    self_unload();
}

__attribute__((destructor)) /* Entry point when unloaded */
void unload() {
    if (!loaded)
        return;

    printf("source-netvar-dumper unloaded.\n\n");
}
