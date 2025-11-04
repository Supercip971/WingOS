#include "kernel/generic/blocker.hpp"

#include <generic/ipc.hpp>
static size_t block_event_id = 0;

namespace kernel
{

BlockEvent create_block(BlockEvent::Type type, uintptr_t data)
{
    (void)data;
    BlockEvent event;
    event.type = type;
    event.id = ++block_event_id;
    event.resolved = false;
    return event;
}

BlockEvent create_mutex_block(BlockMutex*msg)
{
    BlockEvent ev = create_block(BlockEvent::Type::MUTEX);

    ev.id = msg->acquire_uid;
    ev.mtx = msg;
    return ev;
}

} // namespace kernel
