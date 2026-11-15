#pragma once

#include <stddef.h>
#include <stdint.h>

#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct SyscallInterface
    {
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
        return SyscallInterface{.id = SYSCALL_DEBUG_LOG_ID, ._zero = 0, .arg1 = (uintptr_t)debug.message, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallDebug syscall_debug_decode(SyscallInterface interface)
    {
        return SyscallDebug{(char *)interface.arg1};
    }

    // ------- SYSCALL PHYSICAL MEM OWN ----

#define SYSCALL_PHYSICAL_MEM_OWN_ID 0x00000001

    typedef struct SyscallMemOwn
    {
        AssetHandle target_space_handle;
        size_t size;
        size_t addr; // the address of the memory, if 0, it will be allocated by the kernel, will be set to the allocated address
        AssetHandle returned_handle;
    } SyscallMemOwn;

    static inline SyscallInterface syscall_physical_mem_own_encode(SyscallMemOwn *create)
    {
        return SyscallInterface{.id = SYSCALL_PHYSICAL_MEM_OWN_ID, ._zero = 0, .arg1 = (uintptr_t)create, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallMemOwn syscall_physical_mem_own_decode(SyscallInterface interface)
    {
        auto *create = (SyscallMemOwn *)interface.arg1;
        return *create;
    }

    // ------- SYSCALL MAPPING CREATE ----

#define SYSCALL_MAPPING_CREATE_ID 0x00000002

    typedef struct SyscallMap
    {
        AssetHandle target_space_handle;
        uint64_t start;
        uint64_t end;
        AssetHandle physical_mem_handle; // the handle of the physical memory asset
        uint64_t flags;
        AssetHandle returned_handle; // the handle of the created mapping
    } SyscallMap;

    static inline SyscallInterface syscall_map_encode(SyscallMap *create)
    {
        return SyscallInterface{.id = SYSCALL_MAPPING_CREATE_ID, ._zero = 0, .arg1 = (uintptr_t)create, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallMap syscall_map_decode(SyscallInterface interface)
    {
        auto *create = (SyscallMap *)interface.arg1;
        return *create;
    }

    // ------- SYSCALL TASK CREATE ----

#define SYSCALL_TASK_CREATE_ID 0x00000003

    typedef struct SyscallTaskCreate
    {
        AssetHandle target_space_handle;
        uint64_t launch; // the launch parameters for the task

        uint64_t args[4]; // the arguments for the task, can be used to pass data to the task
        AssetHandle returned_handle;
    } SyscallTaskCreate;

    static inline SyscallInterface syscall_task_create_encode(SyscallTaskCreate *create)
    {
        return SyscallInterface{.id = SYSCALL_TASK_CREATE_ID, ._zero = 0, .arg1 = (uintptr_t)create, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallTaskCreate syscall_task_create_decode(SyscallInterface interface)
    {
        auto *create = (SyscallTaskCreate *)interface.arg1;
        return *create;
    }

    // ------- SYSCALL SPACE CREATE ----

#define SYSCALL_SPACE_CREATE_ID 0x00000004

    typedef struct SyscallSpaceCreate
    {
        AssetHandle parent_space_handle; // the handle of the parent space
        uint64_t flags;                  // flags for the space creation
        uint64_t rights;                 // rights for the space creation
        AssetHandle returned_handle;     // the handle of the created space
    } SyscallSpaceCreate;

    static inline SyscallInterface syscall_space_create_encode(SyscallSpaceCreate *create)
    {
        return SyscallInterface{.id = SYSCALL_SPACE_CREATE_ID, ._zero = 0, .arg1 = (uintptr_t)create, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallSpaceCreate syscall_space_create_decode(SyscallInterface interface)
    {
        auto *create = (SyscallSpaceCreate *)interface.arg1;
        return *create;
    }

    // ------- SYSCALL ASSET RELEASE ----

#define SYSCALL_ASSET_RELEASE_ID 0x00000005

    typedef struct SyscallAssetRelease
    {
        AssetHandle space_handle; // the handle of the space, if null, the asset will be released from the kernel
        AssetHandle asset_handle; // the handle of the asset to release
        void *addr;               // the address of the asset if it is memory
        void *end;                // the end address of the asset if it is memory
    } SyscallAssetRelease;

    static inline SyscallInterface syscall_asset_release_encode(SyscallAssetRelease *release)
    {
        return SyscallInterface{.id = SYSCALL_ASSET_RELEASE_ID, ._zero = 0, .arg1 = (uintptr_t)release, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallAssetRelease syscall_asset_release_decode(SyscallInterface interface)
    {
        auto *release = (SyscallAssetRelease *)interface.arg1;
        return *release;
    }

    // ------- SYSCALL TASK LAUNCH ----

#define SYSCALL_TASK_LAUNCH_ID 0x00000006

    typedef struct SyscallTaskLaunch
    {
        AssetHandle target_space_handle; // the handle of the space to launch the task in
        AssetHandle task_handle;         // the handle of the task to launch
        uint64_t args[4];                // the arguments for the task, can be used to pass data to the task
    } SyscallTaskLaunch;

    static inline SyscallInterface syscall_task_launch_encode(SyscallTaskLaunch *launch)
    {
        return SyscallInterface{.id = SYSCALL_TASK_LAUNCH_ID, ._zero = 0, .arg1 = (uintptr_t)launch, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallTaskLaunch syscall_task_launch_decode(SyscallInterface interface)
    {
        auto *launch = (SyscallTaskLaunch *)interface.arg1;
        return *launch;
    }

    // ------- SYSCALL ASSET MOVE ----

#define SYSCALL_ASSET_MOVE 0x00000007

    typedef struct SyscallAssetMove
    {
        AssetHandle from_space_handle; // the handle of the space to move the asset from
        AssetHandle to_space_handle;   // the handle of the space to move the asset to
        AssetHandle asset_handle;      // the handle of the asset to move

        bool copy; // if true, the asset will be copied, otherwise it will be moved
        AssetHandle returned_handle_in_space;
    } SyscallAssetMove;

    static inline SyscallInterface syscall_asset_move_encode(SyscallAssetMove *move)
    {
        return SyscallInterface{.id = SYSCALL_ASSET_MOVE, ._zero = 0, .arg1 = (uintptr_t)move, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallAssetMove syscall_asset_move_decode(SyscallInterface interface)
    {
        auto *move = (SyscallAssetMove *)interface.arg1;
        return *move;
    }

    // ------- SYSCALL IPC CREATE SERVER ----

#define SYSCALL_IPC_CREATE_ENDPOINT_ID 0x00000008

    typedef struct SyscallIpcCreateEndpoint
    {
        AssetHandle space_handle;
        bool is_root;
        bool publish;
        IpcServerHandle returned_addr; // the published address of the endpoint
        AssetHandle returned_handle;   // the handle of the created endpoint asset
    } SyscallIpcCreateEndpoint;

    static inline SyscallInterface syscall_ipc_create_endpoint_encode(SyscallIpcCreateEndpoint *create)
    {
        return SyscallInterface{.id = SYSCALL_IPC_CREATE_ENDPOINT_ID, ._zero = 0, .arg1 = (uintptr_t)create, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcCreateEndpoint syscall_ipc_create_endpoint_decode(SyscallInterface interface)
    {
        auto *create = (SyscallIpcCreateEndpoint *)interface.arg1;
        return *create;
    }

    // ------- SYSCALL IPC CONNECT ----

#define SYSCALL_IPC_CONNECT_ID 0x00000009

    typedef struct SyscallIpcConnect
    {
        AssetHandle sender_space_handle;

        bool connect_by_address;

        union
        {
            IpcServerHandle server_address; // if connecting to registered server, the server's address
            AssetHandle endpoint_handle;    // if not 0, create a connection to a server in local space
        };

        AssetHandle returned_handle_sender; // the handle of the connection (sending)
        uint64_t port_used;
    } SyscallIpcConnect;

    static inline SyscallInterface syscall_ipc_connect_encode(SyscallIpcConnect *connect)
    {
        return SyscallInterface{.id = SYSCALL_IPC_CONNECT_ID, ._zero = 0, .arg1 = (uintptr_t)connect, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcConnect syscall_ipc_connect_decode(SyscallInterface interface)
    {
        auto *connect = (SyscallIpcConnect *)interface.arg1;
        return *connect;
    }

    // ------- SYSCALL IPC SEND ----

#define SYSCALL_IPC_SEND_ID 0x0000000A

    typedef struct SyscallIpcSend
    {
        AssetHandle space_handle;

        bool async;
        IpcConnectionHandle connection_handle; // the handle of the connection to send the message to
        IpcMessage *message;                   // the message to send, will be reused for the reply
    } SyscallIpcSend;

    static inline SyscallInterface syscall_ipc_send_encode(SyscallIpcSend *send)
    {
        return SyscallInterface{.id = SYSCALL_IPC_SEND_ID, ._zero = 0, .arg1 = (uintptr_t)send, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcSend syscall_ipc_send_decode(SyscallInterface interface)
    {
        auto *send = (SyscallIpcSend *)interface.arg1;
        return (*send);
    }

    // ------- SYSCALL IPC SERVER RECEIVE ----

#define SYSCALL_IPC_RECEIVE_ID 0x0000000B
#define SYSCALL_IPC_RECEIVE_INTERNAL_SPACE ((uint64_t)-1)

    typedef struct SyscallIpcReceive
    {

        AssetHandle space_handle; // if -1 uses the kernel internal notification/interrupt

        union
        {
            AssetHandle endpoint_handle; //
            uint64_t internal_signal_id; //
        };

        bool async;

        // RETURN:

        IpcMessage *returned_message; // the message received

        union
        {
            AssetHandle return_context_handle; // if is a server, the port used by the connection
            AssetHandle return_asset_id;
        };

    } SyscallIpcReceive;

    static inline SyscallInterface syscall_ipc_receive_encode(SyscallIpcReceive *receive)
    {
        return SyscallInterface{.id = SYSCALL_IPC_RECEIVE_ID, ._zero = 0, .arg1 = (uintptr_t)receive, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcReceive syscall_ipc_receive_decode(SyscallInterface interface)
    {
        auto *receive = (SyscallIpcReceive *)interface.arg1;
        return (*receive);
    }

    // ------- SYSCALL IPC CALL ----

#define SYSCALL_IPC_CALL_ID 0x0000000D

    typedef struct SyscallIpcCall
    {
        AssetHandle space_handle;

        IpcConnectionHandle connection_handle; // the handle of the connection to send the message to
        IpcMessage *message;                   // the message to send, will be reused for the reply
    } SyscallIpcCall;

    static inline SyscallInterface syscall_ipc_call_encode(SyscallIpcCall *call)
    {
        return SyscallInterface{.id = SYSCALL_IPC_CALL_ID, ._zero = 0, .arg1 = (uintptr_t)call, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcCall syscall_ipc_call_decode(SyscallInterface interface)
    {
        auto *call = (SyscallIpcCall *)interface.arg1;
        return (*call);
    }

    // ------- SYSCALL IPC REPLY ----

#define SYSCALL_IPC_REPLY_ID 0x0000000F

    typedef struct SyscallIpcReply
    {
        AssetHandle space_handle;

        AssetHandle return_task_handle; // the handle of the server to reply to
        IpcMessage *message;            // the message to reply with
    } SyscallIpcReply;

    static inline SyscallInterface syscall_ipc_reply_encode(SyscallIpcReply *reply)
    {
        return SyscallInterface{.id = SYSCALL_IPC_REPLY_ID, ._zero = 0, .arg1 = (uintptr_t)reply, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcReply syscall_ipc_reply_decode(SyscallInterface interface)
    {
        auto *reply = (SyscallIpcReply *)interface.arg1;
        return (*reply);
    }

    // ------- SYSCALL OBJ INFO ----

#define SYSCALL_ASSET_INFO_ID 0x00000011

    typedef struct SyscallAssetInfo
    {
        AssetHandle space_handle;
        bool use_index;

        union
        {
            AssetHandle asset_handle;
            uint64_t asset_index;
        };

        AssetKind returned_kind;           // the kind of the asset
        AssetHandle returned_asset_handle; // if using asset_index

        union
        {
            struct
            {
                size_t size;
                size_t addr;
            } memory;

            struct
            {
                size_t start;
                size_t end;
                AssetHandle physical_mem_handle;
                bool writable;
                bool executable;
            } mapping;

            struct
            {

                uint64_t port;
            } connection;

            struct
            {
                uint64_t element_count;
            } space;

            // other asset types can be added here
        } returned_info;
    } SyscallAssetInfo;

    static inline SyscallInterface syscall_ipc_asset_info_encode(SyscallAssetInfo *status)
    {
        return SyscallInterface{.id = SYSCALL_ASSET_INFO_ID, ._zero = 0, .arg1 = (uintptr_t)status, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallAssetInfo syscall_ipc_asset_info_decode(SyscallInterface interface)
    {
        auto *status = (SyscallAssetInfo *)interface.arg1;
        return *status;
    }

    // ------- SYSCALL CREATE VM ----
#define SYSCALL_VM_CREATE 0x00000012

    typedef struct SyscallCreateVm
    {
        AssetHandle space_handle;
        AssetHandle result_vm;
    } SyscallCreateVm;

    static inline SyscallInterface syscall_create_vm_encode(SyscallCreateVm *vm)
    {
        return SyscallInterface{.id = SYSCALL_VM_CREATE, ._zero = 0, .arg1 = (uintptr_t)vm, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallCreateVm syscall_create_vm_decode(SyscallInterface interface)
    {
        auto *vm = (SyscallCreateVm *)interface.arg1;
        return *vm;
    }

    // ------- SYSCALL CREATE VM MAPPING ----
#define SYSCALL_VM_CREATE_MAPPING 0x00000013

    typedef struct SyscallCreateVmMapping
    {
        AssetHandle space_handle;
        AssetHandle vm_handle; // vm in the space
        uint64_t start;
        uint64_t end;
        AssetHandle physical_mem_handle; // the handle of the physical memory asset
        uint64_t flags;
        AssetHandle returned_handle; // the handle of the created mapping
    } SyscallCreateVmMapping;

    static inline SyscallInterface syscall_create_vm_mapping_encode(SyscallCreateVmMapping *mapping)
    {
        return SyscallInterface{.id = SYSCALL_VM_CREATE_MAPPING, ._zero = 0, .arg1 = (uintptr_t)mapping, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallCreateVmMapping syscall_create_vm_mapping_decode(SyscallInterface interface)
    {
        return *(SyscallCreateVmMapping *)interface.arg1;
    }

    // ------- SYSCALL VM START ----
#define SYSCALL_VM_START 0x00000014

    typedef struct SyscallVmStart
    {
        AssetHandle space_handle;
        AssetHandle vm_handle;
        uint64_t flags;
    } SyscallVmStart;

    static inline SyscallInterface syscall_vm_start_encode(SyscallVmStart *vm)
    {
        return SyscallInterface{.id = SYSCALL_VM_START, ._zero = 0, .arg1 = (uintptr_t)vm, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallVmStart syscall_vm_start_decode(SyscallInterface interface)
    {
        return *(SyscallVmStart *)interface.arg1;
    }

    // ------- SYSCALL VM REG WRITE ----
#define SYSCALL_VM_REG_WRITE 0x00000015

    typedef struct SyscallVmRegWrite
    {
        AssetHandle space_handle;
        AssetHandle vm_handle;
        uint64_t mode;
        uint64_t address;
        uint64_t value;
    } SyscallVmRegWrite;

    static inline SyscallInterface syscall_vm_reg_write(SyscallVmRegWrite *reg)
    {
        return SyscallInterface{.id = SYSCALL_VM_REG_WRITE, ._zero = 0, .arg1 = (uintptr_t)reg, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallVmRegWrite syscall_vm_reg_write_decode(SyscallInterface interface)
    {
        return *(SyscallVmRegWrite *)interface.arg1;
    }

    // ------- SYSCALL VM REG READ ----
#define SYSCALL_VM_REG_READ 0x00000016

    typedef struct SyscallVmRegRead
    {
        AssetHandle space_handle;
        AssetHandle vm_handle;
        uint64_t mode;
        uint64_t address;
        uint64_t value;
    } SyscallVmRegRead;

    static inline SyscallInterface syscall_vm_reg_read(SyscallVmRegRead *reg)
    {
        return SyscallInterface{.id = SYSCALL_VM_REG_READ, ._zero = 0, .arg1 = (uintptr_t)reg, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallVmRegRead syscall_vm_reg_read_decode(SyscallInterface interface)
    {
        return *(SyscallVmRegRead *)interface.arg1;
    }

// ----- SYSCALL SIGNAL CREATE ----
#define SYSCALL_SIGNAL_CREATE 0x00000017

    typedef struct SyscallSignalCreate
    {
        AssetHandle target_space_handle;
        AssetHandle returned_handle;
    } SyscallSignalCreate;

    static inline SyscallInterface syscall_signal_create_encode(SyscallSignalCreate *create)
    {
        return SyscallInterface{.id = SYSCALL_SIGNAL_CREATE, ._zero = 0, .arg1 = (uintptr_t)create, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallSignalCreate syscall_signal_create_decode(SyscallInterface interface)
    {
        auto *create = (SyscallSignalCreate *)interface.arg1;
        return *create;
    }

// ----- SYSCALL SIGNAL PING  ----
#define SYSCALL_SIGNAL_PING 0x00000018

    typedef struct SyscallSignalPing
    {
        AssetHandle target_space_handle;
        AssetHandle handle;
        uint64_t data; // a single uint64_t of data can be sended
        // unused for now
    } SyscallSignalPing;

    static inline SyscallInterface syscall_signal_ping(SyscallSignalPing *ping)
    {
        return SyscallInterface{.id = SYSCALL_SIGNAL_PING, ._zero = 0, .arg1 = (uintptr_t)ping, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallSignalPing syscall_signal_ping_decode(SyscallInterface interface)
    {
        auto *ping = (SyscallSignalPing *)interface.arg1;
        return *ping;
    }

    // ------- SYSCALL IPC X86 INB ----

#define SYSCALL_IPC_X86 0x10000

#define SYSCALL_IPC_X86_PORT (SYSCALL_IPC_X86 + 0x1)

    typedef struct SyscallIpcX86Port
    {
        AssetHandle space_handle;
        uint64_t size; // 1 = byte, 2 = word, 4 = dword
        uint16_t port; // the port to read from
        uint8_t write;
        uint8_t read;
        uint32_t data;
        uint32_t returned_value; // the value read from the port

    } SyscallIpcX86Port;

    static inline SyscallInterface syscall_ipc_x86_port(SyscallIpcX86Port *port)
    {
        return SyscallInterface{.id = SYSCALL_IPC_X86_PORT, ._zero = 0, .arg1 = (uintptr_t)port, .arg2 = 0, .arg3 = 0, .arg4 = 0, .arg5 = 0, .arg6 = 0};
    }

    static inline SyscallIpcX86Port syscall_ipc_x86_port_decode(SyscallInterface interface)
    {
        auto *port = (SyscallIpcX86Port *)interface.arg1;
        return *port;
    }
#ifdef __cplusplus
}
#endif
