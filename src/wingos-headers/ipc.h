#pragma once 


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint64_t IpcServerHandle;
typedef uint64_t IpcConnectionHandle;


// first IPC server manager, 
// each server is a string and a handle and 
// either need to register or unregister
// and the handle is used to connect to the server
#define IPC_SERVER_HANDLE_INIT 0


struct IpcData 
{
    bool is_asset; 
    union 
    {
        uint64_t data; // the data for the IPC message
        uint64_t asset_handle; // the handle of the asset
    };
};

struct IpcMessage
{
    uint64_t message_id; // unique id of the message
    uint64_t flags; // flags for the message
    IpcData data[8]; // data for the message, can be used for IPC payload
};

#ifdef __cplusplus
}
#endif