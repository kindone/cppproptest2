#pragma once

#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/limits.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/function.hpp"

#include "proptest/typefwd.hpp"

/**
 * @file Stream.hpp
 * @brief Stream class that represents a lazy list composed of value (head) and a generator (tail)
 */

namespace proptest {

struct Stream;
template <typename T> struct StreamIterator;

struct PROPTEST_API Stream
{
    Stream(const Any& _head);
    Stream(const Any& _head, const Function<Stream()>& _tailGen);

    Stream& operator=(const Stream& other) {
        head = other.head;
        tailGen = other.tailGen;
        return *this;
    }

    bool isEmpty() const;

    template <typename T>
    decltype(auto) getHeadRef() const {
        return head.getRef<T>();
    }

    Stream getTail() const;

    template <typename T>
    StreamIterator<T> iterator() const;

    template <typename U, typename T>
    Stream transform(Function<U(const T&)> transformer) const {
        if(isEmpty())
            return Stream::empty();
        return Stream(transformer(head.getRef<T>()), [tailGen = this->tailGen, transformer]() {
            return tailGen().template transform<U, T>(transformer);
        });
    }

    template <typename T>
    Stream filter(Function<bool(const T&)> criteria) const {
        if(isEmpty())
            return Stream::empty();
        else {
            for(auto itr = iterator<T>(); itr.hasNext(); ) {
                const T& value = itr.next();
                if(criteria(value)) {
                    auto stream = itr.stream;
                    return Stream(value, [stream, criteria]() -> Stream { return stream.template filter<T>(criteria); });
                }
            }
        }
        return Stream::empty();
    }

    Stream concat(const Stream& other) const;

    Stream concat(Function<Stream()> otherFunc) const;

    Stream take(int n) const;

private:
    Any head;
    Function<Stream()> tailGen;

public:

    static Stream empty();

    template <typename T>
    static Stream one(const T& value) {
        return Stream(Any(value));
    }

    template <typename T>
    static Stream two(const T& value1, const T& value2) {
        Any value2Any(value2);
        return Stream(Any(value1), [value2Any]() -> Stream { return Stream::one<T>(value2Any.getRef<T>()); });
    }

    template <typename T, constructible_from<T>... ARGS>
    static Stream of(ARGS&&...args) {
        return values<T>({args...});
    }

    template <typename T>
    static Stream values(initializer_list<T> list) {
        if(list.size() == 0)
            return empty();
        else if(list.size() == 1)
            return one<T>(*list.begin());

        auto vec = util::make_shared<vector<T>>(list);
        return Stream(Any(vec->front()), [vec]() -> Stream { return values<T>(vec, 1);});
    }

private:
    template <typename T>
    static Stream values(shared_ptr<vector<T>> vec, size_t beginIdx) {
        if(vec->size() == beginIdx)
            return empty();
        else if(vec->size() == beginIdx+1)
            return one<T>((*vec)[beginIdx]);
        return Stream(Any((*vec)[beginIdx]), [vec, beginIdx]() -> Stream { return values<T>(vec, beginIdx+1);});
    }

    static Function<Stream()> emptyTailGen;
};

template <typename T>
struct StreamIterator
{
    StreamIterator(const Stream& stream) : stream(stream) {}

    ~StreamIterator() {}
    bool hasNext() {
        return !stream.isEmpty();
    }

    T next() {
        if(!hasNext())
            throw runtime_error(__FILE__, __LINE__, "no more elements in stream");

        T value = stream.template getHeadRef<T>();
        stream = stream.getTail();
        return value;
    }

    Stream stream;
};

template <typename T>
StreamIterator<T> Stream::iterator() const {
    return StreamIterator<T>{*this};
}

} // namespace proptest
