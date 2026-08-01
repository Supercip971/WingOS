#pragma once

#include "libcore/type-utils.hpp"
#ifdef __cplusplus

#    include "math/align.hpp"
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

    typedef uint64_t MessageHandle;

    typedef uint32_t IpcServerAddress;
    typedef uint32_t IpcServerPort;

    typedef uint64_t IpcServerHandle;
    typedef uint64_t IpcConnectionHandle;

    // first IPC server manager,
    // each server is a string and a handle and
    // either need to register or unregister
    // and the handle is used to connect to the server

#define IPC_MESSAGE_FLAG_NONE 0x0
#define IPC_MESSAGE_FLAG_DISCONNECT 0x1
#define IPC_SERVER_HANDLE_INIT 0

#define MAX_IPC_DATA_SIZE 8

#define MAX_IPC_BUFFER_SIZE 112

    struct IpcData
    {

        struct [[gnu::packed]]
        {

            bool is_asset : 1;
            bool copy_asset : 1;
            bool _reserved_is_mapping : 1; // only used by the kernel
        };

        union
        {
            uint64_t data;         // the data for the IPC message
            uint64_t asset_handle; // the handle of the asset
        };

#ifdef __cplusplus
        constexpr IpcData() : is_asset(false), copy_asset(false), _reserved_is_mapping(false), data(0) {}
#endif
    };

#define IPC_MESSAGE_ARGUMENTS_COUNT 6

    struct IpcMessageArguments
    {
#ifdef __cplusplus
        static constexpr size_t DataCount = IPC_MESSAGE_ARGUMENTS_COUNT;
#endif
        IpcData data[IPC_MESSAGE_ARGUMENTS_COUNT]; // data for the message, can be used for IPC payload
    };
#ifdef __cplusplus

    struct IpcMessage : public fc::NoCopy
    {
        constexpr IpcMessage() : arguments(), port(0), len(0), is_null(false)
        {
        }

        constexpr IpcMessage(IpcMessage &&other) noexcept
            : arguments(other.arguments), port(other.port), len(other.len), is_null(other.is_null)
        {
            for (size_t i = 0; i < math::alignUp((size_t)other.len, sizeof(uint64_t)) / sizeof(uint64_t); i++)
            {
                buffer[i] = other.buffer[i];
            }
        }

        constexpr IpcMessage &operator=(IpcMessage &&other) noexcept
        {
            if (this != &other)
            {
                this->arguments = other.arguments;
                this->port = other.port;
                this->len = other.len;
                this->is_null = other.is_null;

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
            msg.arguments = other.arguments;
            msg.port = other.port;
            msg.len = other.len;
            msg.is_null = other.is_null;

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

        IpcMessageArguments arguments;
        uint64_t port;
        uint16_t len;
        bool is_null;

        union
        {

            uint64_t buffer[MAX_IPC_BUFFER_SIZE / sizeof(uint64_t)]; // buffer for the message, used for IPC payload
            uint8_t raw_buffer[MAX_IPC_BUFFER_SIZE];
        };
    };

#ifdef __cplusplus
};

#endif
