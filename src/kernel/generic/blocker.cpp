#include "kernel/generic/blocker.hpp"
static size_t block_event_id = 0;

namespace kernel
{

BlockEvent create_block()
{
    BlockEvent event;
    event.id = ++block_event_id;
    event.resolved = false;
    return event;
}
} // namespace kernel
