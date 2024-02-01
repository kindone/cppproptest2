#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/set.hpp"

namespace proptest {

template <typename T>
Shrinkable<set<T>> shrinkSet(const Shrinkable<set<Shrinkable<T>>>& shrinkableSet, size_t minSize) {
    // elementwise shrink may result in duplicates removed automatically, which would be inefficient (TODO: unless there is only one element?)
    return shrinkContainer<set, T>(shrinkableSet, minSize, /*elementwise*/false, /*membershipwise*/true);
}

}
