#pragma once

#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/type.hpp"

namespace proptest {
namespace util {

PROPTEST_API Shrinkable<int64_t> binarySearchShrinkable(int64_t value);
PROPTEST_API Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value);

}  // namespace util

template <typename T> requires is_integral_v<T>
PROPTEST_API Shrinkable<T> shrinkIntegral(T value) {
    if constexpr(is_signed<T>::value) {
        if constexpr(sizeof(T) == 1)
            return shrinkIntegral<int8_t>(value).template map<T>([](const int8_t& val) { return static_cast<T>(val); });
        else if constexpr(sizeof(T) == 2)
            return shrinkIntegral<int16_t>(value).template map<T>([](const int16_t& val) { return static_cast<T>(val); });
        else if constexpr(sizeof(T) == 4)
            return shrinkIntegral<int32_t>(value).template map<T>([](const int32_t& val) { return static_cast<T>(val); });
        else if constexpr(sizeof(T) == 8)
            return shrinkIntegral<int64_t>(value).template map<T>([](const int64_t& val) { return static_cast<T>(val); });
    }
    else {
        if constexpr(sizeof(T) == 1)
            return shrinkIntegral<uint8_t>(value).template map<T>([](const uint8_t& val) { return static_cast<T>(val); });
        else if constexpr(sizeof(T) == 2)
            return shrinkIntegral<uint16_t>(value).template map<T>([](const uint16_t& val) { return static_cast<T>(val); });
        else if constexpr(sizeof(T) == 4)
            return shrinkIntegral<uint32_t>(value).template map<T>([](const uint32_t& val) { return static_cast<T>(val); });
        else if constexpr(sizeof(T) == 8)
            return shrinkIntegral<uint64_t>(value).template map<T>([](const uint64_t& val) { return static_cast<T>(val); });
    }
}

template<> PROPTEST_API Shrinkable<int8_t> shrinkIntegral<int8_t>(int8_t);
template<> PROPTEST_API Shrinkable<uint8_t> shrinkIntegral<uint8_t>(uint8_t);
template<> PROPTEST_API Shrinkable<int16_t> shrinkIntegral<int16_t>(int16_t);
template<> PROPTEST_API Shrinkable<uint16_t> shrinkIntegral<uint16_t>(uint16_t);
template<> PROPTEST_API Shrinkable<int32_t> shrinkIntegral<int32_t>(int32_t);
template<> PROPTEST_API Shrinkable<uint32_t> shrinkIntegral<uint32_t>(uint32_t);
template<> PROPTEST_API Shrinkable<int64_t> shrinkIntegral<int64_t>(int64_t);
template<> PROPTEST_API Shrinkable<uint64_t> shrinkIntegral<uint64_t>(uint64_t);
// template<> PROPTEST_API Shrinkable<long> shrinkIntegral<long>(long);
// template<> PROPTEST_API Shrinkable<unsigned long> shrinkIntegral<unsigned long>(unsigned long);

}  // namespace proptest
