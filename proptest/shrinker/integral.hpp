#pragma once

#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/type.hpp"

namespace proptest {
namespace util {

PROPTEST_API Shrinkable<int64_t> binarySearchShrinkable(int64_t value);
PROPTEST_API Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value);

}  // namespace util

template <typename T>
    requires is_integral_v<T>
PROPTEST_API Shrinkable<T> shrinkIntegral(T value)
{
    if constexpr(is_same_v<T, int64_t>)
        return util::binarySearchShrinkable(value);
    else if constexpr(is_signed<T>::value)
        return util::binarySearchShrinkable(static_cast<int64_t>(value)).map<T>([](const int64_t& val) { return static_cast<T>(val); });
    else
        return util::binarySearchShrinkableU(static_cast<uint64_t>(value)).map<T>([](const uint64_t& val) { return static_cast<T>(val); });
}

extern template Shrinkable<int8_t> shrinkIntegral<int8_t>(int8_t);
extern template Shrinkable<uint8_t> shrinkIntegral<uint8_t>(uint8_t);
extern template Shrinkable<int16_t> shrinkIntegral<int16_t>(int16_t);
extern template Shrinkable<uint16_t> shrinkIntegral<uint16_t>(uint16_t);
extern template Shrinkable<int32_t> shrinkIntegral<int32_t>(int32_t);
extern template Shrinkable<uint32_t> shrinkIntegral<uint32_t>(uint32_t);
extern template Shrinkable<int64_t> shrinkIntegral<int64_t>(int64_t);
extern template Shrinkable<uint64_t> shrinkIntegral<uint64_t>(uint64_t);

}  // namespace proptest
