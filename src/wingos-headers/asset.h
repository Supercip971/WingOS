#pragma once 


#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    OBJECT_KIND_UNKNOWN = 0,
    OBJECT_KIND_MEMORY, // memory object, used for memory management
    OBJECT_KIND_MAPPING, // mapping object, used for memory mapping
    OBJECT_KIND_SPACE,
    OBJECT_KIND_TASK,

    OBJECT_KIND_IPC_SERVER, // IPC object, used for inter process communication
    OBJECT_KIND_IPC_CONNECTION, // IPC object, used for inter process communication


} AssetKind;

typedef enum 
{
    ASSET_MAPPING_FLAG_READ = 1 << 0, // if the mapping is readable
    ASSET_MAPPING_FLAG_WRITE = 1 << 1, // if the mapping is writable
    ASSET_MAPPING_FLAG_EXECUTE = 1 << 2, // if the mapping is executable
} AssetMappingFlags;

#ifdef __cplusplus
}
#endif