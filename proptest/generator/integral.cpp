#include "proptest/generator/integral.hpp"
#include "proptest/shrinker/integral.hpp"


namespace proptest {

template <typename T>
Shrinkable<T> generateIntegerImpl(Random& rand, T min = numeric_limits<T>::min(), T max = numeric_limits<T>::max())
{
    T value = 0;
    // TODO: trueProb arg for boundary values
    if (min == numeric_limits<T>::min() && max == numeric_limits<T>::max() && rand.getRandomBool(0.3)) {
        uint32_t i = rand.getRandomSize(0, sizeof(Arbi<T>::boundaryValues) / sizeof(Arbi<T>::boundaryValues[0]));
        value = Arbi<T>::boundaryValues[i];
    } else if (numeric_limits<T>::min() < 0)
        value = rand.getRandom<T>(min, max);
    else
        value = rand.getRandomU<T>(min, max);

    if (value < min || max < value)
        throw runtime_error(__FILE__, __LINE__, "invalid range");

    if (min >= 0)  // [3,5] -> [0,2] -> [3,5]
    {
        return util::binarySearchShrinkableU(static_cast<T>(value - min))
            .template map<T>([min](const uint64_t& _value) { return static_cast<T>(_value + min); });
    } else if (max <= 0)  // [-5,-3] -> [-2,0] -> [-5,-3]
    {
        return util::binarySearchShrinkable(static_cast<T>(value - max)).template map<T>([max](const int64_t& _value) { return static_cast<T>(_value + max); });
    } else  // [-2, 2]
    {
        auto transformer = +[](const int64_t& _value) { return static_cast<T>(_value); };
        return util::binarySearchShrinkable(value).template map<T>(transformer);
    }
}


template <>
Shrinkable<char> generateInteger(Random& rand, char min, char max) {
    return generateIntegerImpl<char>(rand, min, max);
}

template <>
Shrinkable<int8_t> generateInteger(Random& rand, int8_t min, int8_t max) {
    return generateIntegerImpl<int8_t>(rand, min, max);
}

template <>
Shrinkable<int16_t> generateInteger(Random& rand, int16_t min, int16_t max) {
    return generateIntegerImpl<int16_t>(rand, min, max);
}

template <>
Shrinkable<int32_t> generateInteger(Random& rand, int32_t min, int32_t max) {
    return generateIntegerImpl<int32_t>(rand, min, max);
}

template <>
Shrinkable<int64_t> generateInteger(Random& rand, int64_t min, int64_t max) {
    return generateIntegerImpl<int64_t>(rand, min, max);
}

template <>
Shrinkable<uint8_t> generateInteger(Random& rand, uint8_t min, uint8_t max) {
    return generateIntegerImpl<uint8_t>(rand, min, max);
}

template <>
Shrinkable<uint16_t> generateInteger(Random& rand, uint16_t min, uint16_t max) {
    return generateIntegerImpl<uint16_t>(rand, min, max);
}

template <>
Shrinkable<uint32_t> generateInteger(Random& rand, uint32_t min, uint32_t max) {
    return generateIntegerImpl<uint32_t>(rand, min, max);
}

template <>
Shrinkable<uint64_t> generateInteger(Random& rand, uint64_t min, uint64_t max) {
    return generateIntegerImpl<uint64_t>(rand, min, max);
}

// template <>
// Shrinkable<long> generateInteger(Random& rand, long min, long max) {
//     return generateIntegerImpl<long>(rand, min, max);
// }

// template <>
// Shrinkable<unsigned long> generateInteger(Random& rand, unsigned long min, unsigned long max) {
//     return generateIntegerImpl<unsigned long>(rand, min, max);
// }



Shrinkable<char> Arbi<char>::operator()(Random& rand) const
{
    return generateInteger<char>(rand);
}

Shrinkable<int8_t> Arbi<int8_t>::operator()(Random& rand) const
{
    return generateInteger<int8_t>(rand);
}

Shrinkable<int16_t> Arbi<int16_t>::operator()(Random& rand) const
{
    return generateInteger<int16_t>(rand);
}

Shrinkable<int32_t> Arbi<int32_t>::operator()(Random& rand) const
{
    return generateInteger<int32_t>(rand);
}

Shrinkable<int64_t> Arbi<int64_t>::operator()(Random& rand) const
{
    return generateInteger<int64_t>(rand);
}

Shrinkable<uint8_t> Arbi<uint8_t>::operator()(Random& rand) const
{
    return generateInteger<uint8_t>(rand);
}

Shrinkable<uint16_t> Arbi<uint16_t>::operator()(Random& rand) const
{
    return generateInteger<uint16_t>(rand);
}

Shrinkable<uint32_t> Arbi<uint32_t>::operator()(Random& rand) const
{
    return generateInteger<uint32_t>(rand);
}

Shrinkable<uint64_t> Arbi<uint64_t>::operator()(Random& rand) const
{
    return generateInteger<uint64_t>(rand);
}

// Shrinkable<long> Arbi<long>::operator()(Random& rand) const
// {
//     return generateInteger<long>(rand);
// }

// Shrinkable<unsigned long> Arbi<unsigned long>::operator()(Random& rand) const
// {
//     return generateInteger<unsigned long>(rand);
// }

// template instantiation for gen namespace
namespace gen {
DEFINE_FOR_ALL_INTTYPES(DEFINE_GEN_NATURAL);
DEFINE_FOR_ALL_INTTYPES(DEFINE_GEN_NONNEGATIVE);
DEFINE_FOR_ALL_INTTYPES(DEFINE_GEN_INTERVAL);
DEFINE_FOR_ALL_INTTYPES(DEFINE_GEN_INRANGE);
DEFINE_FOR_ALL_INTTYPES(DEFINE_GEN_INTEGERS);
} // namespace gen

}  // namespace proptest
