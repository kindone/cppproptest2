#include "proptest/shrinker/integral.hpp"
#include "proptest/Stream.hpp"
#include "proptest/util/function.hpp"


namespace proptest {
namespace util {

Shrinkable<int64_t> binarySearchShrinkable(int64_t value)
{
    using stream_t = Shrinkable<int64_t>::StreamType;
    using elem_t = Shrinkable<int64_t>::StreamElementType;
    using genfunc_t = Function<stream_t(int64_t, int64_t)>;

    // given min, max, generate stream
    static genfunc_t genpos = +[](int64_t min, int64_t max) -> stream_t {
        int64_t mid = static_cast<int64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::one<elem_t>(make_shrinkable<int64_t>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<int64_t>(mid).with([=]() -> stream_t { return genpos(min, mid); })),
                            [=]() -> stream_t { return genpos(mid, max); });
    };

    static genfunc_t genneg = +[](int64_t min, int64_t max) -> stream_t {
        int64_t mid = static_cast<int64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? -1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::one<elem_t>(make_shrinkable<int64_t>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<int64_t>(mid).with([=]() -> stream_t { return genneg(mid, max); })),
                            [=]() -> stream_t { return genneg(min, mid); });
    };

    return make_shrinkable<int64_t>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t::one<elem_t>(make_shrinkable<int64_t>(0)).concat(genpos(0, value));
        else
            return stream_t::one<elem_t>(make_shrinkable<int64_t>(0)).concat(genneg(value, 0));
    });
}

Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value)
{
    using stream_t = Shrinkable<uint64_t>::StreamType;
    using elem_t = Shrinkable<uint64_t>::StreamElementType;
    using genfunc_t = Function<stream_t(uint64_t, uint64_t)>;

    // given min, max, generate stream
    static genfunc_t genpos = +[](uint64_t min, uint64_t max) {
        uint64_t mid = static_cast<uint64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::one<elem_t>(make_shrinkable<uint64_t>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<uint64_t>(mid).with([=]() { return genpos(min, mid); })),
                            [=]() { return genpos(mid, max); });
    };

    static genfunc_t genneg = +[](uint64_t min, uint64_t max) {
        uint64_t mid = (min % 2 != 0 && max % 2 != 0) ? (min / 2 + max / 2 - 1) : (min / 2 + max / 2);

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t::one<elem_t>(make_shrinkable<uint64_t>(mid));
        } else
            return stream_t(elem_t(make_shrinkable<uint64_t>(mid).with([=]() { return genneg(mid, max); })),
                            [=]() { return genneg(min, mid); });
    };

    return make_shrinkable<uint64_t>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t::one<elem_t>(make_shrinkable<uint64_t>(0U)).concat(genpos(0U, value));
        else
            return stream_t::one<elem_t>(make_shrinkable<uint64_t>(0U)).concat(genneg(value, 0U));
    });
}

}  // namespace util


template <typename T>
    requires is_integral_v<T>
Shrinkable<T> shrinkIntegralImpl(T value)
{
    if constexpr(is_same_v<T, int64_t>)
        return util::binarySearchShrinkable(value);
    else if constexpr(is_signed<T>::value)
        return util::binarySearchShrinkable(static_cast<int64_t>(value)).map<T>([](const int64_t& val) { return static_cast<T>(val); });
    else
        return util::binarySearchShrinkableU(static_cast<uint64_t>(value)).map<T>([](const uint64_t& val) { return static_cast<T>(val); });
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

template<> Shrinkable<long> shrinkIntegral<long>(long value) {
    return shrinkIntegralImpl<long>(value);
}

template<> Shrinkable<unsigned long> shrinkIntegral<unsigned long>(unsigned long value) {
    return shrinkIntegralImpl<unsigned long>(value);
}

}  // namespace proptest
