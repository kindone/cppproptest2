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

template <typename FLOATTYPE>
PROPTEST_API Stream<Shrinkable<FLOATTYPE>> floatShrinks(FLOATTYPE value)
{
    using Stream = Stream<Shrinkable<FLOATTYPE>>;
    int exp = 0;
    if (value == 0.0f) {
        return Stream::empty();
    } else if (isnan(value)) {
        return Stream::one(make_shrinkable<FLOATTYPE>(0.0f));
    } else {
        FLOATTYPE fraction = 0.0f;
        if (isinf(value)) {
            if (value > 0) {
                auto max = numeric_limits<FLOATTYPE>::max();
                fraction = util::decomposeFloat(max, &exp);
            } else {
                auto min = numeric_limits<FLOATTYPE>::lowest();
                fraction = util::decomposeFloat(min, &exp);
            }
        } else {
            fraction = util::decomposeFloat(value, &exp);
        }

        auto expShrinkable = shrinkIntegral<int64_t>(exp);
        // shrink exponent
        auto floatShrinkable =
            expShrinkable.map<FLOATTYPE>([fraction](const int& exp) { return util::composeFloat(fraction, exp); });

        // prepend 0.0
        floatShrinkable = floatShrinkable.with(Stream::one(make_shrinkable<FLOATTYPE>(0.0f)).concat(floatShrinkable.getShrinks()));

        // shrink fraction within (0.0 and 0.5)
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.get();
            int exp = 0;
            /*FLOATTYPE fraction = */ util::decomposeFloat(value, &exp);
            if (value == 0.0f)
                return Stream::empty();
            else if (value > 0) {
                return Stream::one(make_shrinkable<FLOATTYPE>(util::composeFloat(0.5f, exp)));
            } else {
                return Stream::one(make_shrinkable<FLOATTYPE>(util::composeFloat(-0.5f, exp)));
            }
        });

        // integerfy
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.get();
            auto intValue = static_cast<int>(value);
            if (intValue != 0 && abs(intValue) < abs(value)) {
                return Stream::one(make_shrinkable<FLOATTYPE>(static_cast<FLOATTYPE>(intValue)));
            } else
                return Stream::empty();
        });

        return floatShrinkable.getShrinks();
    }
}

template <typename FLOATTYPE>
PROPTEST_API Shrinkable<FLOATTYPE> shrinkFloat(FLOATTYPE value) {
    return make_shrinkable<FLOATTYPE>(value).with(floatShrinks(value));
}

extern template Shrinkable<float> shrinkFloat<float>(float);
extern template Shrinkable<double> shrinkFloat<double>(double);


} // namespace proptest
