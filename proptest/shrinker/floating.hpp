#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"

namespace proptest {

// template declaration
template <typename FLOATTYPE>
PROPTEST_API typename Shrinkable<FLOATTYPE>::StreamType floatShrinks(FLOATTYPE value);

// specialization
template <> PROPTEST_API typename Shrinkable<float>::StreamType floatShrinks(float value);
template <> PROPTEST_API typename Shrinkable<double>::StreamType floatShrinks(double value);

template <typename FLOATTYPE>
PROPTEST_API Shrinkable<FLOATTYPE> shrinkFloat(FLOATTYPE value) {
    return make_shrinkable<FLOATTYPE>(value).with(floatShrinks(value));
}

extern template Shrinkable<float> shrinkFloat<float>(float);
extern template Shrinkable<double> shrinkFloat<double>(double);


} // namespace proptest
