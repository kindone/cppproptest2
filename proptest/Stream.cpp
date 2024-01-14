#include "proptest/Stream.hpp"

namespace proptest {

AnyStreamIterator::AnyStreamIterator(const Stream<Any>& stream) : stream(stream) {}

AnyStreamIterator::AnyStreamIterator(const AnyStream& stream) : stream(stream) {}

bool AnyStreamIterator::hasNext() {
    return !stream.isEmpty();
}

Any AnyStreamIterator::nextAny() {
    if(!hasNext())
        throw runtime_error("no more elements in stream");

    Any value = stream.getHeadRef();
    stream = stream.getTail();
    return value;
}

} // namespace proptest