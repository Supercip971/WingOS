#include "io.hpp"

#include "reader.hpp"
#include "seekable.hpp"
#include "void.hpp"
#include "writer.hpp"

fc::Reader::~Reader() = default;

fc::Seeker::~Seeker() = default;

fc::VoidRW::~VoidRW() = default;

fc::Writer::~Writer() = default;
