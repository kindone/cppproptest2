#include "proptest/shrinker/integral.hpp"
#include "proptest/Stream.hpp"
#include "proptest/util/function.hpp"


namespace proptest {
namespace util {

template <typename T>
Shrinkable<T> binarySearchShrinkableImpl(T value);
template <typename T>
Shrinkable<T> binarySearchShrinkableImpl(T value);

template <typename T>
Shrinkable<T> binarySearchShrinkableImpl(T value)
{
    using stream_t = Shrinkable<T>::StreamType;
    using elem_t = Shrinkable<T>::StreamElementType;
    using genfunc_t = Function<stream_t(T, T)>;

    // given min, max, generate stream
    static genfunc_t genpos = +[](T min, T max) -> stream_t {
        T mid = static_cast<T>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::template one<elem_t>(make_shrinkable<T>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<T>(mid).with([=]() -> stream_t { return genpos(min, mid); })),
                            [=]() -> stream_t { return genpos(mid, max); });
    };

    static genfunc_t genneg = +[](T min, T max) -> stream_t {
        int64_t mid = static_cast<T>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? -1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::template one<elem_t>(make_shrinkable<T>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<T>(mid).with([=]() -> stream_t { return genneg(mid, max); })),
                            [=]() -> stream_t { return genneg(min, mid); });
    };

    return make_shrinkable<T>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t::template one<elem_t>(make_shrinkable<T>(0)).concat(genpos(0, value));
        else
            return stream_t::template one<elem_t>(make_shrinkable<T>(0)).concat(genneg(value, 0));
    });
}

template <typename T>
Shrinkable<T> binarySearchShrinkableUImpl(T value)
{
    using stream_t = Shrinkable<T>::StreamType;
    using elem_t = Shrinkable<T>::StreamElementType;
    using genfunc_t = Function<stream_t(T, T)>;

    // given min, max, generate stream
    static genfunc_t genpos = +[](T min, T max) {
        T mid = static_cast<T>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::template one<elem_t>(make_shrinkable<T>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<T>(mid).with([=]() { return genpos(min, mid); })),
                            [=]() { return genpos(mid, max); });
    };

    static genfunc_t genneg = +[](T min, T max) {
        T mid = (min % 2 != 0 && max % 2 != 0) ? (min / 2 + max / 2 - 1) : (min / 2 + max / 2);

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::template one<elem_t>(make_shrinkable<T>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<T>(mid).with([=]() { return genneg(mid, max); })),
                            [=]() { return genneg(min, mid); });
    };

    return make_shrinkable<T>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t::template one<elem_t>(make_shrinkable<T>(0U)).concat(genpos(0U, value));
        else
            return stream_t::template one<elem_t>(make_shrinkable<T>(0U)).concat(genneg(value, 0U));
    });
}

Shrinkable<int64_t> binarySearchShrinkable(int64_t value) {
    return binarySearchShrinkableImpl<int64_t>(value);
}

Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value) {
    return binarySearchShrinkableUImpl<uint64_t>(value);
}

}  // namespace util


template <typename T>
    requires is_integral_v<T>
Shrinkable<T> shrinkIntegralImpl(T value)
{
    if constexpr(is_signed<T>::value)
        return util::binarySearchShrinkableImpl<T>(value);
    else
        return util::binarySearchShrinkableUImpl<T>(value);
}

template<> Shrinkable<int8_t> shrinkIntegral<int8_t>(int8_t value) {
    return shrinkIntegralImpl<int8_t>(value);
}

template<> Shrinkable<uint8_t> shrinkIntegral<uint8_t>(uint8_t value) {
    return shrinkIntegralImpl<uint8_t>(value);
}

template<> Shrinkable<int16_t> shrinkIntegral<int16_t>(int16_t value) {
    return shrinkIntegralImpl<int16_t>(value);
}

template<> Shrinkable<uint16_t> shrinkIntegral<uint16_t>(uint16_t value) {
    return shrinkIntegralImpl<uint16_t>(value);
}

template<> Shrinkable<int32_t> shrinkIntegral<int32_t>(int32_t value) {
    return shrinkIntegralImpl<int32_t>(value);
}

template<> Shrinkable<uint32_t> shrinkIntegral<uint32_t>(uint32_t value) {
    return shrinkIntegralImpl<uint32_t>(value);
}

template<> Shrinkable<int64_t> shrinkIntegral<int64_t>(int64_t value) {
    return shrinkIntegralImpl<int64_t>(value);
}

template<> Shrinkable<uint64_t> shrinkIntegral<uint64_t>(uint64_t value) {
    return shrinkIntegralImpl<uint64_t>(value);
}

// template<> Shrinkable<size_t> shrinkIntegral<size_t>(size_t value) {
//     return shrinkIntegralImpl<size_t>(value);
// }

}  // namespace proptest
