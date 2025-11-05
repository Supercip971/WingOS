#pragma once

#ifdef __cplusplus

#    include "libcore/type-utils.hpp"
#    include "math/align.hpp"
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

    typedef uint64_t MessageHandle;
    typedef uint64_t IpcServerHandle;
    typedef uint64_t IpcConnectionHandle;

// first IPC server manager,
// each server is a string and a handle and
// either need to register or unregister
// and the handle is used to connect to the server
#define IPC_SERVER_HANDLE_INIT 0

#define MAX_IPC_DATA_SIZE 8

#define MAX_IPC_BUFFER_SIZE 112

    struct IpcData
    {
        bool is_asset;
        union
        {
            uint64_t data;         // the data for the IPC message
            uint64_t asset_handle; // the handle of the asset
        };
    };

#ifdef __cplusplus
    struct IpcMessage : public core::NoCopy
    {
        IpcMessage() : message_id(0), flags(0), data{}, len(0)
        {
        }

        constexpr IpcMessage(IpcMessage && other) noexcept
            : message_id(other.message_id), flags(other.flags), len(other.len)
        {

            for (size_t i = 0; i < 8; i++)
            {
                data[i] = other.data[i];
                data[i].is_asset = other.data[i].is_asset;
                data[i].data = other.data[i].data;
            }
            for (size_t i = 0; i < math::alignUp((size_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
            {
                buffer[i] = other.buffer[i];
            }
        }


        constexpr IpcMessage &operator=(IpcMessage &&other) noexcept
        {
            if (this != &other)
            {
                message_id = other.message_id;
                flags = other.flags;
                len = other.len;

                for (size_t i = 0; i < 8; i++)
                {
                    data[i] = other.data[i];
                    data[i].is_asset = other.data[i].is_asset;
                    data[i].data = other.data[i].data;
                }
                for (size_t i = 0; i < math::alignUp((size_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
                {
                    buffer[i] = other.buffer[i];
                }
            }
            return *this;
        }
        constexpr static IpcMessage copy(const IpcMessage &other)
        {
            IpcMessage msg;
            msg.message_id = other.message_id;
            msg.flags = other.flags;
            msg.len = other.len;
            for (size_t i = 0; i < 8; i++)
            {
                msg.data[i] = other.data[i];
                msg.data[i].is_asset = other.data[i].is_asset;
                msg.data[i].data = other.data[i].data;
            }
            for (size_t i = 0; i < math::alignUp((size_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
            {
                msg.buffer[i] = other.buffer[i];
            }
            return msg;
        }
#else
struct IpcMessage
{
#endif
        uint64_t message_id; // unique id of the message
        uint64_t flags;      // flags for the message
        IpcData data[8];     // data for the message, can be used for IPC payload

        uint16_t len;

        union [[gnu::packed]]
        {

            uint64_t buffer[MAX_IPC_BUFFER_SIZE / sizeof(uint64_t)]; // buffer for the message, used for IPC payload
            uint8_t raw_buffer[MAX_IPC_BUFFER_SIZE];
        };
    };

#ifdef __cplusplus
};

#endif