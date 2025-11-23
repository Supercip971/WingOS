#pragma once 


#include <stddef.h>
#include <stdint.h>
#include "libcore/type-utils.hpp"
#include "wingos-headers/ipc.h"
#include "wingos-headers/asset.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SyscallInterface {
    uint32_t id; 
    uint32_t _zero; // This is a zero field to align the structure
    uintptr_t arg1;
    uintptr_t arg2;
    uintptr_t arg3;
    uintptr_t arg4;
    uintptr_t arg5;
    uintptr_t arg6;
} SyscallInterface;


// ------- SYSCALL DEBUG ----

// cause a no-op in release mode, because it is really unsafe 
#define SYSCALL_DEBUG_LOG_ID 0x00000000
typedef struct SyscallDebug 
{
    char *message;
} SyscallDebug;

static inline SyscallInterface syscall_debug_encode(SyscallDebug debug)
{
    return SyscallInterface{SYSCALL_DEBUG_LOG_ID, 0, (uintptr_t)debug.message, 0, 0, 0, 0, 0};
}

static inline SyscallDebug syscall_debug_decode(SyscallInterface interface)
{
    return SyscallDebug{(char *)interface.arg1};
}

// ------- SYSCALL PHYSICAL MEM OWN ----

#define SYSCALL_PHYSICAL_MEM_OWN_ID 0x00000001

typedef struct SyscallMemOwn
{
    uint64_t target_space_handle;
    size_t size;
    size_t addr; // the address of the memory, if 0, it will be allocated by the kernel, will be set to the allocated address
    uint64_t returned_handle;
} SyscallMemOwn;

static inline SyscallInterface syscall_physical_mem_own_encode(SyscallMemOwn* create)
{
    return SyscallInterface{SYSCALL_PHYSICAL_MEM_OWN_ID, 0,(uintptr_t) create, 0, 0, 0, 0, 0};
}

static inline SyscallMemOwn syscall_physical_mem_own_decode(SyscallInterface interface)
{
    SyscallMemOwn *create = (SyscallMemOwn *)interface.arg1;
    return *create;
}

// ------- SYSCALL MAPPING CREATE ----

#define SYSCALL_MAPPING_CREATE_ID 0x00000002

typedef struct SyscallMap
{
    uint64_t target_space_handle;
    size_t start;
    size_t end;
    uint64_t physical_mem_handle; // the handle of the physical memory asset
    uint64_t flags;
    uint64_t returned_handle; // the handle of the created mapping
} SyscallMap;

static inline SyscallInterface syscall_map_encode(SyscallMap* create)
{
    return SyscallInterface{SYSCALL_MAPPING_CREATE_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallMap syscall_map_decode(SyscallInterface interface)
{
    SyscallMap *create = (SyscallMap *)interface.arg1;
    return *create;
}

// ------- SYSCALL TASK CREATE ----

#define SYSCALL_TASK_CREATE_ID 0x00000003

typedef struct SyscallTaskCreate
{
    uint64_t target_space_handle;
    uint64_t launch; // the launch parameters for the task
    
    uint64_t args[4]; // the arguments for the task, can be used to pass data to the task
    uint64_t returned_handle;
} SyscallTaskCreate;

static inline SyscallInterface syscall_task_create_encode(SyscallTaskCreate* create)
{
    return SyscallInterface{SYSCALL_TASK_CREATE_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallTaskCreate syscall_task_create_decode(SyscallInterface interface)
{
    SyscallTaskCreate *create = (SyscallTaskCreate *)interface.arg1;
    return *create;
}

// ------- SYSCALL SPACE CREATE ----

#define SYSCALL_SPACE_CREATE_ID 0x00000004
typedef struct SyscallSpaceCreate
{
    uint64_t parent_space_handle; // the handle of the parent space
    uint64_t flags; // flags for the space creation
    uint64_t rights; // rights for the space creation
    uint64_t returned_handle; // the handle of the created space
} SyscallSpaceCreate;

static inline SyscallInterface syscall_space_create_encode(SyscallSpaceCreate* create)
{
    return SyscallInterface{SYSCALL_SPACE_CREATE_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallSpaceCreate syscall_space_create_decode(SyscallInterface interface)
{
    SyscallSpaceCreate *create = (SyscallSpaceCreate *)interface.arg1;
    return *create;
}

// ------- SYSCALL ASSET RELEASE ----

#define SYSCALL_ASSET_RELEASE_ID 0x00000005

typedef struct SyscallAssetRelease
{
    uint64_t space_handle; // the handle of the space, if null, the asset will be released from the kernel
    uint64_t asset_handle; // the handle of the asset to release
    void* addr; // the address of the asset if it is memory 
    void* end; // the end address of the asset if it is memory
} SyscallAssetRelease;

static inline SyscallInterface syscall_asset_release_encode(SyscallAssetRelease* release)
{
    return SyscallInterface{SYSCALL_ASSET_RELEASE_ID, 0, (uintptr_t)release, 0, 0, 0, 0, 0};
}

static inline SyscallAssetRelease syscall_asset_release_decode(SyscallInterface interface)
{
    SyscallAssetRelease *release = (SyscallAssetRelease *)interface.arg1;
    return *release;
}


// ------- SYSCALL TASK LAUNCH ----

#define SYSCALL_TASK_LAUNCH_ID 0x00000006

typedef struct SyscallTaskLaunch
{
    uint64_t target_space_handle; // the handle of the space to launch the task in
    uint64_t task_handle; // the handle of the task to launch
    uint64_t args[4]; // the arguments for the task, can be used to pass data to the task
} SyscallTaskLaunch;

static inline SyscallInterface syscall_task_launch_encode(SyscallTaskLaunch* launch)
{
    return SyscallInterface{SYSCALL_TASK_LAUNCH_ID, 0, (uintptr_t)launch, 0, 0, 0, 0, 0};
}

static inline SyscallTaskLaunch syscall_task_launch_decode(SyscallInterface interface)
{
    SyscallTaskLaunch *launch = (SyscallTaskLaunch *)interface.arg1;
    return *launch;
}


// ------- SYSCALL ASSET MOVE ----

#define SYSCALL_ASSET_MOVE 0x00000007
typedef struct SyscallAssetMove
{
    uint64_t from_space_handle; // the handle of the space to move the asset from
    uint64_t to_space_handle; // the handle of the space to move the asset to
    uint64_t asset_handle; // the handle of the asset to move

    bool copy; // if true, the asset will be copied, otherwise it will be moved
    uint64_t returned_handle_in_space;
} SyscallAssetMove;

static inline SyscallInterface syscall_asset_move_encode(SyscallAssetMove* move)
{
    return SyscallInterface{SYSCALL_ASSET_MOVE, 0, (uintptr_t)move, 0, 0, 0, 0, 0};
}
static inline SyscallAssetMove syscall_asset_move_decode(SyscallInterface interface)
{
    SyscallAssetMove *move = (SyscallAssetMove *)interface.arg1;
    return *move;
}


// ------- SYSCALL IPC CREATE SERVER ----

#define SYSCALL_IPC_CREATE_SERVER_ID 0x00000008
typedef struct SyscallIpcCreateServer
{
    uint64_t space_handle;
   
    bool is_root; 
    IpcServerHandle returned_addr; // the handle of the created server
    uint64_t returned_handle; // the handle of the created server asset
} SyscallIpcCreateServer;

static inline SyscallInterface syscall_ipc_create_server_encode(SyscallIpcCreateServer* create)
{
    return SyscallInterface{SYSCALL_IPC_CREATE_SERVER_ID, 0, (uintptr_t)create, 0, 0, 0, 0, 0};
}

static inline SyscallIpcCreateServer syscall_ipc_create_server_decode(SyscallInterface interface)
{
    SyscallIpcCreateServer *create = (SyscallIpcCreateServer *)interface.arg1;
    return *create;
}

// ------- SYSCALL IPC CONNECT ----

#define SYSCALL_IPC_CONNECT_ID 0x00000009

#define IPC_CONNECTION_FLAG_PIPE 0x1
typedef struct SyscallIpcConnect
{
    bool block; // wait for the connection to be established
    uint64_t sender_space_handle;
    
    IpcServerHandle server_handle; // the handle of the server to connect to, if 0 create a pipe
    uint64_t flags; // flags for the connection
    uint64_t returned_handle_sender; // the handle of the connection (sending)

    // ONLY USED FOR PIPE CONNECTIONS
    uint64_t returned_handle_receiver; // the handle of the connection (receiving)
    uint64_t receiver_space_handle; 

} SyscallIpcConnect;

static inline SyscallInterface syscall_ipc_connect_encode(SyscallIpcConnect* connect)
{
    return SyscallInterface{SYSCALL_IPC_CONNECT_ID, 0, (uintptr_t)connect, 0, 0, 0, 0, 0};
}

static inline SyscallIpcConnect syscall_ipc_connect_decode(SyscallInterface interface)
{
    SyscallIpcConnect *connect = (SyscallIpcConnect *)interface.arg1;
    return *connect;
}

// ------- SYSCALL IPC SEND ----

#define SYSCALL_IPC_SEND_ID 0x0000000A

typedef struct SyscallIpcSend
{
     uint64_t space_handle;
    
     bool expect_reply;
    IpcConnectionHandle connection_handle; // the handle of the connection to send the message to
    IpcMessage* message; // the message to send
    MessageHandle returned_msg_handle; // the handle of the message sent, in the context of the connection
} SyscallIpcSend;

static inline SyscallInterface syscall_ipc_send_encode(SyscallIpcSend* send)
{
    return SyscallInterface{SYSCALL_IPC_SEND_ID, 0, (uintptr_t)send, 0, 0, 0, 0, 0};
}

static inline SyscallIpcSend syscall_ipc_send_decode(SyscallInterface interface)
{
    SyscallIpcSend *send = (SyscallIpcSend *)interface.arg1;
    return (*send);
}

// ------- SYSCALL IPC SERVER RECEIVE ----

#define SYSCALL_IPC_SERVER_RECEIVE_ID 0x0000000B

typedef struct SyscallIpcServerReceive
{
    bool block;
    bool contain_response;
 
    uint64_t space_handle;
    

    uint64_t server_handle; // the handle of the server that received the message (asset)
    bool is_disconnect; // if true, the connection will be disconnected after receiving the message
    
    IpcConnectionHandle connection_handle; // the handle of the connection to receive the message from
    MessageHandle returned_msg_handle; // the handle of the message received, in the context of the connection
    IpcMessage* returned_message; // the message received
} SyscallIpcServerReceive;

static inline SyscallInterface syscall_ipc_server_receive_encode(SyscallIpcServerReceive* receive)
{
    return SyscallInterface{SYSCALL_IPC_SERVER_RECEIVE_ID, 0, (uintptr_t)receive, 0, 0, 0, 0, 0};
}

static inline SyscallIpcServerReceive syscall_ipc_server_receive_decode(SyscallInterface interface)
{
    SyscallIpcServerReceive *receive = (SyscallIpcServerReceive *)interface.arg1;
    return (*receive);
}
// ------- SYSCALL IPC CLIENT RECEIVE REPLY ----

#define SYSCALL_IPC_CLIENT_RECEIVE_REPLY_ID 0x0000000C

typedef struct SyscallIpcClientReceiveReply
{
    bool block;
    bool contain_response; // if true, the client will wait for a response from the server
    uint64_t space_handle;
    
    
    bool is_disconnect; // if true, the connection will be disconnected after receiving the message
    MessageHandle message; // the message received
    
    IpcConnectionHandle connection_handle; // the handle of the connection to receive the message from
    IpcMessage* returned_message; // the message received
    
} SyscallIpcClientReceiveReply;

static inline SyscallInterface syscall_ipc_receive_client_reply_encode(SyscallIpcClientReceiveReply* receive)
{
    return SyscallInterface{SYSCALL_IPC_CLIENT_RECEIVE_REPLY_ID, 0, (uintptr_t)receive, 0, 0, 0, 0, 0};
}

static inline SyscallIpcClientReceiveReply syscall_ipc_receive_client_reply_decode(SyscallInterface interface)
{
    SyscallIpcClientReceiveReply *receive = (SyscallIpcClientReceiveReply *)interface.arg1;
    return (*receive);
}

// ------- SYSCALL IPC CALL ----

#define SYSCALL_IPC_CALL_ID 0x0000000D

typedef struct SyscallIpcCall
{
    uint64_t space_handle;
    bool has_reply;
    
    IpcConnectionHandle connection_handle; // the handle of the connection to call
    IpcMessage* message; // the message to send
    IpcMessage* returned_message; // the message returned from the call operation, if any
} SyscallIpcCall;

static inline SyscallInterface syscall_ipc_call_encode(SyscallIpcCall* call)
{
    return SyscallInterface{SYSCALL_IPC_CALL_ID, 0, (uintptr_t)call, 0, 0, 0, 0, 0};
}

static inline SyscallIpcCall syscall_ipc_call_decode(SyscallInterface interface)
{
    SyscallIpcCall *call = (SyscallIpcCall *)interface.arg1;
    return (*call);
}

// ------- SYSCALL IPC ACCEPT ----

#define SYSCALL_IPC_ACCEPT_ID 0x0000000E
typedef struct SyscallIpcAccept
{
    bool block;
    uint64_t space_handle;
    
    bool accepted_connection; // if true, has valid accepted connection, if false no attempt were made
    IpcServerHandle server_handle; // the handle of the server to accept the connection from
    IpcConnectionHandle connection_handle; // the handle of the connection to accept
} SyscallIpcAccept;

static inline SyscallInterface syscall_ipc_accept_encode(SyscallIpcAccept* accept)
{
    return SyscallInterface{SYSCALL_IPC_ACCEPT_ID, 0, (uintptr_t)accept, 0, 0, 0, 0, 0};
}

static inline SyscallIpcAccept syscall_ipc_accept_decode(SyscallInterface interface)
{
    SyscallIpcAccept *accept = (SyscallIpcAccept *)interface.arg1;
    return *accept;
}


// ------- SYSCALL IPC REPLY ----

#define SYSCALL_IPC_REPLY_ID 0x0000000F
typedef struct SyscallIpcReply
{
    uint64_t space_handle;

    IpcServerHandle server_handle; // the handle of the server to reply to
    IpcConnectionHandle connection_handle; // the handle of the connection to reply to
    MessageHandle message_handle; // the handle of the message to reply with
    IpcMessage* message; // the message to reply with
} SyscallIpcReply;

static inline SyscallInterface syscall_ipc_reply_encode(SyscallIpcReply* reply)
{
    return SyscallInterface{SYSCALL_IPC_REPLY_ID, 0, (uintptr_t)reply, 0, 0, 0, 0, 0};
}

static inline SyscallIpcReply syscall_ipc_reply_decode(SyscallInterface interface)
{
    SyscallIpcReply *reply = (SyscallIpcReply *)interface.arg1;
    return (*reply);
}

// ------- SYSCALL IPC STATUS ----

#define SYSCALL_IPC_STATUS_ID 0x00000010
typedef struct SyscallIpcStatus
{
    uint64_t space_handle;
    IpcConnectionHandle connection_handle; // the handle of the connection to check the status of
    bool returned_is_accepted; // if true, the connection is connected, otherwise it is not
} SyscallIpcStatus;

static inline SyscallInterface syscall_ipc_status_encode(SyscallIpcStatus* status)
{
    return SyscallInterface{SYSCALL_IPC_STATUS_ID, 0, (uintptr_t)status, 0, 0, 0, 0, 0};
}

static inline SyscallIpcStatus syscall_ipc_status_decode(SyscallInterface interface)
{
    SyscallIpcStatus *status = (SyscallIpcStatus *)interface.arg1;
    return *status;
}

// ------- SYSCALL OBJ INFO ----

#define SYSCALL_ASSET_INFO_ID 0x00000011
typedef struct SyscallAssetInfo
{
    uint64_t space_handle;
    uint64_t asset_handle;

    AssetKind returned_kind; // the kind of the asset
    union {
        struct {
            size_t size; 
            size_t addr; 
        } memory;

        struct {
            size_t start; 
            size_t end;   
            uint64_t physical_mem_handle; 
            bool writable;
            bool executable;
        } mapping;

        // other asset types can be added here
    } returned_info;
} SyscallAssetInfo;



static inline SyscallInterface syscall_ipc_asset_info_encode(SyscallAssetInfo* status)
{
    return SyscallInterface{SYSCALL_ASSET_INFO_ID, 0, (uintptr_t)status, 0, 0, 0, 0, 0};
}

static inline SyscallAssetInfo syscall_ipc_asset_info_decode(SyscallInterface interface)
{
    SyscallAssetInfo *status = (SyscallAssetInfo*)interface.arg1;
    return *status;
}


// ------- SYSCALL IPC X86 INB ----

#define SYSCALL_IPC_X86 0x10000 

#define SYSCALL_IPC_X86_PORT (SYSCALL_IPC_X86 + 0x1)
typedef struct SyscallIpcX86Port
{
    uint64_t space_handle;

    uint64_t port; // the port to read from
    size_t size; // 1 = byte, 2 = word, 4 = dword
    bool write; 
    uint64_t data;

    bool read;
    uint64_t returned_value; // the value read from the port

} SyscallIpcX86Port;

static inline SyscallInterface syscall_ipc_x86_port(SyscallIpcX86Port* port)
{
    return SyscallInterface{SYSCALL_IPC_X86_PORT, 0, (uintptr_t)port, 0, 0, 0, 0, 0};
}
static inline SyscallIpcX86Port syscall_ipc_x86_port_decode(SyscallInterface interface)
{
    SyscallIpcX86Port *port = (SyscallIpcX86Port *)interface.arg1;
    return *port;
}
#ifdef __cplusplus
}
#endif
