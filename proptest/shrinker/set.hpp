#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/set.hpp"


namespace std {
template <typename T>
class less<proptest::Shrinkable<T>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs) const
    {
        return lhs.getRef() < rhs.getRef();
    }
};
}  // namespace std

namespace proptest {

template <typename T>
Shrinkable<set<T>> shrinkSet(const Shrinkable<set<Shrinkable<T>>>& shrinkableSet, size_t minSize) {
    // elementwise shrink may result in duplicates removed automatically, which would be inefficient (TODO: unless there is only one element?)
    return shrinkContainer<set, T>(shrinkableSet, minSize, /*elementwise*/false, /*membershipwise*/true);
}

}
