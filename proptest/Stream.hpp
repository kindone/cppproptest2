#pragma once

#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/limits.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"

#include "proptest/typefwd.hpp"

namespace proptest {

template<typename T, typename... ARGS>
    concept AllT = (is_same_v<ARGS, T> && ...);


template <typename T> struct Stream;
template <typename T> struct StreamIterator;
//struct AnyStream;
//struct AnyStreamIterator;

template <typename T>
struct StreamIterator
{
    StreamIterator(const Stream<T>& stream) : stream(stream) {}

    ~StreamIterator() {}
    bool hasNext() {
        return !stream.isEmpty();
    }
    T next() {
        if(!hasNext())
            throw runtime_error(__FILE__, __LINE__, "no more elements in stream");

        T value = stream.getHeadRef();
        stream = stream.getTail();
        return value;
    }

    Stream<T> stream;
};

template <typename T>
struct PROPTEST_API Stream
{
    Stream(const Any& _head) : head(_head), tailGen([]() -> Stream { return Stream::empty(); }) {}
    Stream(const Any& _head, const Function<Stream()>& _tailGen) : head(_head), tailGen(_tailGen) {}

    Stream(const Stream<T>& other) : head(other.head), tailGen(other.tailGen) {}

    bool isEmpty() const {
        return head.isEmpty();
    };

    const T& getHeadRef() const {
        return head.getRef<T>();
    }

    Stream<T> getTail() const {
        return Stream<T>{tailGen()};
    }

    StreamIterator<T> iterator() const {
        return StreamIterator<T>{*this};
    }

    template <typename U>
    Stream<U> transform(Function<U(const T&)> transformer) const {
        if(isEmpty())
            return Stream<U>::empty();
        return Stream<U>(transformer(head.getRef<T>()), [thisStream = *this, transformer]() -> Stream<U> {
            return thisStream.tailGen().template transform<U>(transformer);
        });
    }

    Stream<T> filter(Function<bool(const T&)> criteria) const {
        if(isEmpty())
            return Stream::empty();
        else {
            for(auto itr = iterator(); itr.hasNext(); ) {
                const T& value = itr.next();
                if(criteria(value)) {
                    auto stream = itr.stream;
                    return Stream(value, [stream,criteria]() -> Stream { return stream.filter(criteria); });
                }
            }
        }
        return Stream::empty();
    }

    Stream concat(const Stream& other) const {
        if(isEmpty())
            return other;
        else {
            return Stream(head.getRef<T>(), [stream = *this, other]() -> Stream { return stream.getTail().concat(other); });
        }
    }

    Stream<T> take(int n) const {
        if(isEmpty() || n <= 0)
            return Stream::empty();
        else {
            return Stream(head.getRef<T>(), [stream = *this, n]() -> Stream { return stream.getTail().take(n-1); });
        }
    }

private:
    Any head;
    Function<Stream()> tailGen;

public:

    static Stream empty() {
        return Stream(Any::empty);
    }

    static Stream one(const T& value) {
        return Stream<T>(Any(value));
    }

    static Stream<T> two(const T& value1, const T& value2) {
        Any value2Any(value2);
        return Stream(Any(value1), [value2Any]() -> Stream { return Stream::one(value2Any.getRef<T>()); });
    }

    template <typename...ARGS>
        requires (AllT<T, ARGS...>)
    static Stream<T> of(ARGS&&...args) {
        return values({args...});
    }

    static Stream<T> values(initializer_list<T> list) {
        if(list.size() == 0)
            return empty();
        else if(list.size() == 1)
            return one(*list.begin());

        auto vec = util::make_shared<vector<T>>(list);
        return Stream(Any(vec->front()), [vec]() -> Stream { return values(vec, 1);});
    }

private:
    static Stream<T> values(shared_ptr<vector<T>> vec, size_t beginIdx) {
        if(vec->size() == beginIdx)
            return empty();
        else if(vec->size() == beginIdx+1)
            return one((*vec)[beginIdx]);
        return Stream(Any((*vec)[beginIdx]), [vec, beginIdx]() -> Stream { return values(vec, beginIdx+1);});
    }
};

/*
struct PROPTEST_API AnyStream {
    template <typename T>
    AnyStream(const Stream<T>& otherStream) : stream(otherStream.template transform<Any>([](const T& t) { return Any(t); })) {}
    AnyStream(const Stream<Any>& otherStream) : stream(otherStream) {}
    AnyStream(const Any& value, const AnyFunction& callable) : stream(value, callable) {}

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

    AnyStream getTail() const {
        return stream.getTail();
    }

    AnyStream transform(Function<Any(const Any&)> transformer) const {
        return stream.transform(transformer);
    }

    AnyStream filter(Function<bool(const Any&)> criteria) const {
        return stream.filter(criteria);
    }

    AnyStream concat(const AnyStream& other) const {
        return stream.concat(other.stream);
    }

    AnyStream take(int n) const {
        return stream.take(n);
    }

private:
    Stream<Any> stream;
};

struct PROPTEST_API AnyStreamIterator
{
    AnyStreamIterator(const Stream<Any>& stream);
    AnyStreamIterator(const AnyStream& stream);

    ~AnyStreamIterator() {}
    bool hasNext();

    template <typename T>
    T next() {
        if(!hasNext())
            throw runtime_error(__FILE__, __LINE__, "no more elements in stream");

        T value = stream.getHeadRef<T>();
        stream = stream.getTail();
        return value;
    }

    Any nextAny();

    AnyStream stream;
};
*/

} // namespace proptest
