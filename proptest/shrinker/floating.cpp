#include "proptest/shrinker/floating.hpp"
#include "proptest/std/math.hpp"

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


template <typename FLOATTYPE>
Shrinkable<FLOATTYPE>::StreamType floatShrinksImpl(FLOATTYPE value)
{
    using Stream = Shrinkable<FLOATTYPE>::StreamType;
    using Elem = Shrinkable<FLOATTYPE>::StreamElementType;
    int exp = 0;
    if (value == 0.0f) {
        return Stream::empty();
    } else if (isnan(value)) {
        return Stream::template one<Elem>(make_shrinkable<FLOATTYPE>(0.0f));
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
            expShrinkable.map<FLOATTYPE>([fraction](const int64_t& exp) { return util::composeFloat(fraction, static_cast<int>(exp)); });

        // prepend 0.0
        floatShrinkable = floatShrinkable.with(Stream::template one<Elem>(make_shrinkable<FLOATTYPE>(0.0f)).concat(floatShrinkable.getShrinks()));

        // shrink fraction within (0.0 and 0.5)
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.getRef();
            int exp = 0;
            /*FLOATTYPE fraction = */ util::decomposeFloat(value, &exp);
            if (value == 0.0f)
                return Stream::empty();
            else if (value > 0) {
                return Stream::template one<Elem>(make_shrinkable<FLOATTYPE>(util::composeFloat(0.5f, exp)));
            } else {
                return Stream::template one<Elem>(make_shrinkable<FLOATTYPE>(util::composeFloat(-0.5f, exp)));
            }
        });

        // integerfy
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.getRef();
            auto intValue = static_cast<int>(value);
            if (intValue != 0 && static_cast<FLOATTYPE>(abs(intValue)) < abs(value)) {
                return Stream::template one<Elem>(make_shrinkable<FLOATTYPE>(static_cast<FLOATTYPE>(intValue)));
            } else
                return Stream::empty();
        });

        return floatShrinkable.getShrinks();
    }
}

template <>
Shrinkable<float>::StreamType floatShrinks(float value)
{
    return floatShrinksImpl<float>(value);
}

template <>
Shrinkable<double>::StreamType floatShrinks(double value)
{
    return floatShrinksImpl<double>(value);
}



// function instantiation
template Shrinkable<double> shrinkFloat<double>(double value);
template Shrinkable<float> shrinkFloat<float>(float value);



} // namespace proptest
