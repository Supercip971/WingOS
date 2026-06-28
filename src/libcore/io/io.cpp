#include "io.hpp"
#include <libcore/fmt/log.hpp>

#include "reader.hpp"
#include "seekable.hpp"
#include "void.hpp"
#include "writer.hpp"

core::Reader::~Reader()
{
}

core::Seeker::~Seeker()
{
}

core::VoidRW::~VoidRW()
{
}

core::Writer::~Writer()
{
}
