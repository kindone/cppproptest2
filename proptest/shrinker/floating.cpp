#include "proptest/shrinker/floating.hpp"

namespace proptest {

namespace util {

template <>
float decomposeFloat<float>(float value, int* exp)
{
    return frexpf(value, exp);
}

template <>
double decomposeFloat<double>(double value, int* exp)
{
    return frexp(value, exp);
}

template <>
float composeFloat<float>(float value, int exp)
{
    return ldexpf(value, exp);
}

template <>
double composeFloat<double>(double value, int exp)
{
    return ldexp(value, exp);
}

} // namespace util


// function instantiation
template Shrinkable<double> shrinkFloat<double>(double value);
template Shrinkable<float> shrinkFloat<float>(float value);



} // namespace proptest
