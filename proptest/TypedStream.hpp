#pragma once

#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/util/std.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"

#include "proptest/typefwd.hpp"

namespace proptest {

template <typename T> struct TypedStream;
template <typename T> struct TypedIterator;
struct UntypedStream;
struct UntypedIterator;

template <typename T>
struct TypedIterator
{
    TypedIterator(const TypedStream<T>& stream) : stream(stream) {}

    ~TypedIterator() {}
    bool hasNext() {
        return !stream.isEmpty();
    }
    T next() {
        if(!hasNext())
            throw runtime_error("no more elements in stream");

        T value = stream.getHeadRef();
        stream = stream.getTail();
        return value;
    }

    TypedStream<T> stream;
};

template <typename T>
struct TypedStream
{
    TypedStream(const Any& _head) : head(_head), tailGen([]() -> TypedStream { return TypedStream::empty(); }) {}
    TypedStream(const Any& _head, const Function<TypedStream()>& _tailGen) : head(_head), tailGen(_tailGen) {}

    TypedStream(const TypedStream<T>& other) : head(other.head), tailGen(other.tailGen) {}

    bool isEmpty() const {
        return head.isEmpty();
    };

    const T& getHeadRef() const {
        return head.getRef<T>();
    }

    TypedStream<T> getTail() const {
        return TypedStream<T>{tailGen()};
    }

    TypedIterator<T> iterator() const {
        return TypedIterator<T>{*this};
    }

    template <typename U>
    TypedStream<U> transform(Function<U(const T&)> transformer) const {
        if(isEmpty())
            return TypedStream<U>::empty();
        return TypedStream<U>(transformer(head.getRef<T>()), [thisStream = *this, transformer]() -> TypedStream<U> {
            return thisStream.tailGen().template transform<U>(transformer);
        });
    }

    TypedStream<T> filter(Function<bool(const T&)> criteria) const {
        if(isEmpty())
            return TypedStream::empty();
        else {
            for(auto itr = iterator(); itr.hasNext(); ) {
                const T& value = itr.next();
                if(criteria(value)) {
                    auto stream = itr.stream;
                    return TypedStream(value, [stream,criteria]() -> TypedStream { return stream.filter(criteria); });
                }
            }
        }
        return TypedStream::empty();
    }

    TypedStream concat(const TypedStream& other) const {
        if(isEmpty())
            return other;
        else {
            return TypedStream(head.getRef<T>(), [stream = *this, other]() -> TypedStream { return stream.getTail().concat(other); });
        }
    }

    TypedStream<T> take(int n) const {
        if(isEmpty() || n <= 0)
            return TypedStream::empty();
        else {
            return TypedStream(head.getRef<T>(), [stream = *this, n]() -> TypedStream { return stream.getTail().take(n-1); });
        }
    }

private:
    Any head;
    Function<TypedStream()> tailGen;

public:

    static TypedStream empty() {
        return TypedStream(Any::empty, []() -> TypedStream { return TypedStream::empty(); });
    }

    static TypedStream one(const T& value) {
        return TypedStream<T>(Any(value), []() -> TypedStream { return TypedStream::empty(); });
    }

    static TypedStream<T> two(const T& value1, const T& value2) {
        Any value2Any(value2);
        return TypedStream(Any(value1), [value2Any]() -> TypedStream { return TypedStream::one(value2Any.getRef<T>()); });
    }
};


struct UntypedStream {
    template <typename T>
    UntypedStream(const TypedStream<T>& otherStream) : stream(otherStream.template transform<Any>([](const T& t) { return Any(t); })) {}
    UntypedStream(const TypedStream<Any>& otherStream) : stream(otherStream) {}
    UntypedStream(const Any& value, const AnyFunction& callable) : stream(value, callable) {}

    bool isEmpty() const {
        return stream.isEmpty();
    }

    template <typename T>
    const T& getHeadRef() const {
        return stream.getHeadRef().getRef<T>();
    }

    const Any& getHeadRef() const {
        return stream.getHeadRef();
    }

    UntypedStream getTail() const {
        return stream.getTail();
    }

    UntypedStream transform(Function<Any(const Any&)> transformer) const {
        return stream.transform(transformer);
    }

    UntypedStream filter(Function<bool(const Any&)> criteria) const {
        return stream.filter(criteria);
    }

    UntypedStream concat(const UntypedStream& other) const {
        return stream.concat(other.stream);
    }

    UntypedStream take(int n) const {
        return stream.take(n);
    }

private:
    TypedStream<Any> stream;
};

struct UntypedIterator
{
    UntypedIterator(const TypedStream<Any>& stream);
    UntypedIterator(const UntypedStream& stream);

    ~UntypedIterator() {}
    bool hasNext();

    template <typename T>
    T next() {
        if(!hasNext())
            throw runtime_error("no more elements in stream");

        T value = stream.getHeadRef<T>();
        stream = stream.getTail();
        return value;
    }

    Any nextAny();

    UntypedStream stream;
};

} // namespace proptest