#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"

namespace proptest {

namespace util {

template <typename FLOATTYPE>
FLOATTYPE decomposeFloat(FLOATTYPE value, int* exp);

template <typename FLOATTYPE>
FLOATTYPE composeFloat(FLOATTYPE value, int exp);

template <>
float decomposeFloat<float>(float value, int* exp);

template <>
double decomposeFloat<double>(double value, int* exp);

template <>
float composeFloat<float>(float value, int exp);

template <>
double composeFloat<double>(double value, int exp);

}  // namespace util


// template declaration
template <typename FLOATTYPE>
PROPTEST_API Shrinkable<FLOATTYPE>::StreamType floatShrinks(FLOATTYPE value);

// specialization
template <> PROPTEST_API Shrinkable<float>::StreamType floatShrinks(float value);
template <> PROPTEST_API Shrinkable<double>::StreamType floatShrinks(double value);

template <typename FLOATTYPE>
PROPTEST_API Shrinkable<FLOATTYPE> shrinkFloat(FLOATTYPE value) {
    return make_shrinkable<FLOATTYPE>(value).with(floatShrinks(value));
}

extern template Shrinkable<float> shrinkFloat<float>(float);
extern template Shrinkable<double> shrinkFloat<double>(double);


} // namespace proptest
