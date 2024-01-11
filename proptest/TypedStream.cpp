#include "proptest/TypedStream.hpp"

namespace proptest {

UntypedIterator::UntypedIterator(const TypedStream<Any>& stream) : stream(stream) {}

UntypedIterator::UntypedIterator(const UntypedStream& stream) : stream(stream) {}

bool UntypedIterator::hasNext() {
    return !stream.isEmpty();
}

Any UntypedIterator::nextAny() {
    if(!hasNext())
        throw runtime_error("no more elements in stream");

    Any value = stream.getHeadRef();
    stream = stream.getTail();
    return value;
}

} // namespace proptest