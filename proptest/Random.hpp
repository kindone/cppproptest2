#pragma once

#include "proptest/api.hpp"
#include "proptest/std/random.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/std/exception.hpp"

/**
 * @file Random.hpp
 * @brief Random number generator class
 */

namespace proptest {

PROPTEST_API int64_t getCurrentTime();

class PROPTEST_API Random {
public:
    Random(uint64_t seed);
    Random(const Random& other);
    bool getRandomBool(double threshold = 0.5);
    int8_t getRandomInt8(int8_t min = INT8_MIN, int8_t max = INT8_MAX);
    uint8_t getRandomUInt8(uint8_t min = 0, uint8_t max = UINT8_MAX);
    int16_t getRandomInt16(int16_t min = INT16_MIN, int16_t max = INT16_MAX);
    uint16_t getRandomUInt16(uint16_t min = 0, uint16_t max = UINT16_MAX);
    int32_t getRandomInt32(int32_t min = INT32_MIN, int32_t max = INT32_MAX);
    uint32_t getRandomUInt32(uint32_t min = 0, uint32_t max = UINT32_MAX);
    int64_t getRandomInt64(int64_t min = INT64_MIN, int64_t max = INT64_MAX);
    uint64_t getRandomUInt64(uint64_t min = 0, uint64_t max = UINT64_MAX);
    float getRandomFloat();
    double getRandomDouble();
    float getRandomFloat(float min, float max);
    double getRandomDouble(double min, double max);
    uint32_t getRandomSize(size_t fromIncluded, size_t toExcluded);

    Random& operator=(const Random& other);

    template <typename T>
    T getRandom(int64_t min, int64_t max)
    {
        if constexpr(!is_signed_v<T>) {
            throw runtime_error(__FILE__, __LINE__, "getRandom<T> unsigned type not defined. Use getRandomU<T> instead");}
        return static_cast<T>(getRandomInt64(min, max));
    }

    template <typename T>
    T getRandomU(uint64_t min, uint64_t max)
    {
        if constexpr(is_signed_v<T>) {
            throw runtime_error(__FILE__, __LINE__, "getRandomU<T> for signed type not defined. Use getRandom<T> instead");}
        return static_cast<T>(getRandomUInt64(min, max));
    }

private:
    uint64_t next8U();
    // default_random_engine engine;
    mt19937_64 engine;
    uniform_int_distribution<uint64_t> dist;
};

template <>
PROPTEST_API char Random::getRandom<char>(int64_t min, int64_t max);

template <>
PROPTEST_API int8_t Random::getRandom<int8_t>(int64_t min, int64_t max);

template <>
PROPTEST_API int16_t Random::getRandom<int16_t>(int64_t min, int64_t max);

template <>
PROPTEST_API int32_t Random::getRandom<int32_t>(int64_t min, int64_t max);

template <>
PROPTEST_API int64_t Random::getRandom<int64_t>(int64_t min, int64_t max);

template <>
PROPTEST_API char Random::getRandomU<char>(uint64_t min, uint64_t max);

template <>
PROPTEST_API uint8_t Random::getRandomU<uint8_t>(uint64_t min, uint64_t max);

template <>
PROPTEST_API uint16_t Random::getRandomU<uint16_t>(uint64_t min, uint64_t max);

template <>
PROPTEST_API uint32_t Random::getRandomU<uint32_t>(uint64_t min, uint64_t max);

template <>
PROPTEST_API uint64_t Random::getRandomU<uint64_t>(uint64_t min, uint64_t max);


}  // namespace proptest
