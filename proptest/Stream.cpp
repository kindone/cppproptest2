#include "proptest/Stream.hpp"

namespace proptest {

Stream::Stream(const Any& _head) : head(_head), tailGen(emptyTailGen) {}
Stream::Stream(const Any& _head, const Function<Stream()>& _tailGen) : head(_head), tailGen(_tailGen) {}

bool Stream::isEmpty() const {
    return head.isEmpty();
};

Stream Stream::getTail() const {
    return tailGen();
}


Stream Stream::concat(const Stream& other) const {
    if(isEmpty())
        return other;
    else {
        return Stream(head, [tailGen = this->tailGen, other]() -> Stream { return tailGen().concat(other); });
    }
}

Stream Stream::concat(Function<Stream()> otherFunc) const {
    if(isEmpty())
        return otherFunc();
    else {
        return Stream(head, [tailGen = this->tailGen, otherFunc]() -> Stream { return tailGen().concat(otherFunc()); });
    }
}

Stream Stream::take(int n) const {
    if(isEmpty() || n <= 0)
        return Stream::empty();
    else {
        return Stream(head, [tailGen = this->tailGen, n]() -> Stream { return tailGen().take(n-1); });
    }
}


Stream Stream::empty() {
    static const Stream emptyStream = Stream(Any::empty);
    return emptyStream;
}


Function<Stream()> Stream::emptyTailGen = +[]() -> Stream { return Stream::empty(); };

} // namespace proptest
