#pragma once

#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "iol/wingos/syscalls.h"
#include "wingos-headers/syscalls.h"
namespace prot
{
struct Duplex
{
    Wingos::IpcClient connection_sender;
    Wingos::IpcClient connection_receiver;

    static core::Result<Duplex> create(Wingos::Space  const &sender_space, Wingos::Space const  &receiver_space, uint64_t flags = 0)
    {

        Wingos::IpcClient client_sender = {};
        Wingos::IpcClient client_receiver = {};
        try$(Wingos::IpcClient::pipe_create(sender_space.handle, client_sender, receiver_space.handle, client_receiver, flags));

         Duplex result = {};
        result.connection_sender = core::move(client_sender);
        result.connection_receiver = core::move(client_receiver);
        return core::Result<Duplex>::success(result);
    }
};

class SenderPipe
{
public:
    core::Result<void> send(const void *buffer, size_t len)
    {
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            // FIXME: implement large buffer receiving
            return "length exceeds max ipc buffer size";
        }

        IpcMessage message = {};


        for(size_t i = 0; i < len; i++)
        {
            message.raw_buffer[i] = (( const uint8_t *)buffer)[i];
        }
        message.len = len;
        auto sended_message = _connection.send(message);

        return {};
    }

    static core::Result<SenderPipe> from(Wingos::IpcClient connection)
    {
        SenderPipe pipe;
        pipe._connection = core::move(connection);
        return core::Result<SenderPipe>::success(core::move(pipe));
    }
private:
    Wingos::IpcClient _connection;

};

class ReceiverPipe
{

    public:
    
    static core::Result<ReceiverPipe> from(Wingos::IpcClient connection)
    {
        ReceiverPipe pipe;
        pipe._connection = core::move(connection);
        return core::Result<ReceiverPipe>::success(core::move(pipe));
    }


    core::Result<IpcMessage> receive_message()
    {
        auto res = _connection.receive(false);

        if (res.is_error())
        {
            return "failed to receive message";
        }

        return core::Result<IpcMessage>::success(core::move(res.unwrap().received));
    }

    core::Result<size_t> receive(void *buffer, size_t len)
    {
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            return "length exceeds max ipc buffer size";
        }

        auto res = _connection.receive(false);

        if (res.is_error())
        {
            return "failed to receive message";
        }

        auto received_message = core::move(res.unwrap().received);

        size_t mlen = received_message.len;
        if (len < mlen)
        {
            mlen = len;
        }

        for(size_t i = 0; i < math::alignUp(mlen, sizeof(uint64_t)) / sizeof(uint64_t); i++)
        {
            (( uint64_t *)buffer)[i] = received_message.buffer[i];
        }

//        memcpy(buffer, (uint8_t *)received_message.raw_buffer, mlen);

        return {mlen};
    }
private:
    Wingos::IpcClient _connection;


};
} // namespace prot