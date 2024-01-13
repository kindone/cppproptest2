#pragma once

#include "proptest/Shrinkable.hpp"

namespace proptest {
namespace util {

Shrinkable<int64_t> binarySearchShrinkable(int64_t value);
Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value);

}  // namespace util

template <typename T>
    requires std::is_integral<T>::value
PROPTEST_API Shrinkable<T> shrinkIntegral(T value)
{
    if constexpr(is_same_v<T, int64_t>)
        return util::binarySearchShrinkable(value);
    else if constexpr(is_signed<T>::value)
        return util::binarySearchShrinkable(static_cast<int64_t>(value)).map<T>([](const int64_t& val) { return static_cast<T>(val); });
    else
        return util::binarySearchShrinkableU(static_cast<uint64_t>(value)).map<T>([](const uint64_t& val) { return static_cast<T>(val); });
}

}  // namespace proptest
