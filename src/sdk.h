#ifndef SDK_H_
#define SDK_H_

#include <stdint.h>

#define GETALLCLASSES_IDX 10

/*
 * See:
 * https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/public/dt_recv.h#L87
 */

typedef enum {
    INT = 0,
    FLOAT,
    VECTOR,
    VECTOR2D,
    STRING,
    ARRAY,
    DATATABLE,
    INT64,
    SENDPROPTYPEMAX
} SendPropType;

typedef struct DataVariant_s { /* struct unions 4ever */
    union {
        float Float;
        long Int;
        char* String;
        void* Data;
        float Vector[3];
        int64_t Int64;
    };

    SendPropType type;
} DataVariant;

typedef struct RecvProp_s RecvProp;
typedef struct RecvTable_s {
    struct RecvProp_s* props;
    int propsCount;
    void* decoder;
    char* tableName;
    bool initialized;
    bool inMainList;
} RecvTable;

typedef struct RecvProp_s {
    char* varName;
    SendPropType recvType;
    int flags;
    int stringBufferSize;
    bool insideArray;
    const void* extraData;
    RecvProp* arrayProp;
    void* arrayLengthProxyFn;
    void* proxyFn;
    void* dataTableProxyFn;
    RecvTable* dataTable;
    int offset;
    int elementStride;
    int elements;
    const char* parentArrayPropName;
} RecvProp;

typedef struct ClientClass_s {
    void* CreateClientClassFn;
    void* CreateEventFn;
    char* networkName;
    RecvTable* recvTable;
    struct ClientClass_s* next;
    int classID;
    const char* mapClassname;
} ClientClass;

/*----------------------------------------------------------------------------*/

#define STR(a, b) a##b
#define PADSTR(n) STR(pad, n)
#define PAD(n)    uint8_t PADSTR(__LINE__)[n]

typedef struct BaseClient BaseClient;

typedef struct {
    PAD(sizeof(void*) * GETALLCLASSES_IDX);
    ClientClass* (*GetAllClasses)(BaseClient* thisptr);
} VT_BaseClient;

struct BaseClient {
    VT_BaseClient* vt;
};

#endif /* SDK_H_ */
