#pragma once

#include "proptest/util/any.hpp"

namespace proptest {

template <typename T> struct Shrinkable;
template <typename T> struct Iterator;
template <typename T> struct Stream;

struct ShrinkableLike {
    template <typename T>
    ShrinkableLike(const Shrinkable<T>& shr) : shrinkable(shr) {}

    bool isEmpty() const;

    const T& getHeadRef() const;

    Stream getTail();

    Iterator<T> iterator();

    template <typename U>
    Stream<U> transform<U>(function<U(const T&)> transformer) const;

    Stream<T> filter(function<bool(const T&)> criteria) const;

    Stream<T> concat(const Stream<T>& other) const;

    Stream<T> take(int n) const;

private:

    Any shrinkable;
};

template <typename T>
struct ShrinkableLikeImpl {

};

} // namespace proptest