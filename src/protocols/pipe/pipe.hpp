#pragma once

#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/result.hpp"
#include "wingos-headers/ipc.h"

namespace prot
{

// EMPTY DUPLEX USED FOR BUILD TIME FOR NOW, WILLE BE REWORKED LATER

struct Duplex
{
    Wingos::UAsset connection_sender;
    Wingos::UAsset connection_receiver;

    static fc::Result<Duplex> create(Wingos::Space, Wingos::Space, uint64_t)
    {
        Duplex d = {};
        d.connection_sender.handle = 0;
        d.connection_receiver.handle = 0;
        return d;
    }
};

class SenderPipe
{
    Wingos::IpcClient connection;

public:
    SenderPipe() = default;

    SenderPipe(Wingos::IpcClient &&c) : connection(std::move(c)) {}

    static fc::Result<SenderPipe> from(Wingos::IpcClient &&c)
    {
        SenderPipe p(std::move(c));
        return p;
    }

    fc::Result<void> send(const void *data, size_t len)
    {
        IpcMessage msg = {};
        if (len > MAX_IPC_BUFFER_SIZE)
        {
            len = MAX_IPC_BUFFER_SIZE;
        }
        memcpy(msg.raw_buffer, data, len);
        msg.len = (uint16_t)len;
        connection.send(msg);
        return {};
    }

    Wingos::IpcClient &raw_connection() { return connection; }
};

class ReceiverPipe
{
    Wingos::IpcClient connection;

public:
    ReceiverPipe() = default;

    ReceiverPipe(Wingos::IpcClient &&c) : connection(std::move(c)) {}

    static fc::Result<ReceiverPipe> from(Wingos::IpcClient &&c)
    {
        ReceiverPipe p(std::move(c));
        return p;
    }

    fc::Result<size_t> receive(void *buffer, size_t len)
    {
        (void)buffer;
        (void)len;
        return fc::Result<size_t>::error("no data");
    }

    fc::Result<IpcMessage> receive_message()
    {
        return fc::Result<IpcMessage>::error("no data");
    }

    Wingos::IpcClient &raw_connection() { return connection; }
};

} // namespace prot
