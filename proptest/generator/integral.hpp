#pragma once
#include "proptest/Stream.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/concepts.hpp"

/**
 * @file integral.hpp
 * @brief Arbitrary for integers
 */

#define DEFINE_FOR_ALL_INTTYPES(DEF) \
    DEF(int8_t);\
    DEF(int16_t);\
    DEF(int32_t);\
    DEF(int64_t);\
    DEF(uint8_t);\
    DEF(uint16_t);\
    DEF(uint32_t);\
    DEF(long);\
    DEF(unsigned long);

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);

// template declaration
template <typename T>
PROPTEST_API Shrinkable<T> generateInteger(Random& rand, T min = numeric_limits<T>::min(), T max = numeric_limits<T>::max());

#define SPECIALIZE_GENERATEINTEGER(TYPE) \
    template <> PROPTEST_API Shrinkable<TYPE> generateInteger(Random& rand, TYPE min, TYPE max)

DEFINE_FOR_ALL_INTTYPES(SPECIALIZE_GENERATEINTEGER);

template <integral T>
class PROPTEST_API Arbi<T> final : public ArbiBase<T> {
public:
    virtual Shrinkable<T> operator()(Random&) const override {
        throw runtime_error(__FILE__, __LINE__, "Arbi for integral type not defined for this integral type");
    }
    static constexpr char boundaryValues[] = {0, -1 ,1, -2, 2,
                                                numeric_limits<char>::min(),
                                                numeric_limits<char>::max(),
                                                numeric_limits<char>::min() + 1,
                                                numeric_limits<char>::max() - 1,
                                                ' ', '"', '\'', '\t', '\n', '\r'};

};

/**
 * @ingroup Generators
 * @brief Arbitrary for char
 */
template <>
class PROPTEST_API Arbi<char> final : public ArbiBase<char> {
public:
    virtual Shrinkable<char> operator()(Random& rand) const override;
    static constexpr char boundaryValues[] = {0, -1 ,1, -2, 2,
                                                numeric_limits<char>::min(),
                                                numeric_limits<char>::max(),
                                                numeric_limits<char>::min() + 1,
                                                numeric_limits<char>::max() - 1,
                                                ' ', '"', '\'', '\t', '\n', '\r'};

};

/**
 * @ingroup Generators
 * @brief Arbitrary for int8_t
 */
template <>
class PROPTEST_API Arbi<int8_t> final : public ArbiBase<int8_t> {
public:
    virtual Shrinkable<int8_t> operator()(Random& rand) const override;
    static constexpr int8_t boundaryValues[] = {0, -1 ,1, -2, 2,
                                                numeric_limits<int8_t>::min(),
                                                numeric_limits<int8_t>::max(),
                                                numeric_limits<int8_t>::min() + 1,
                                                numeric_limits<int8_t>::max() - 1,
                                                ' ', '"', '\'', '\t', '\n', '\r'};

};

/**
 * @ingroup Generators
 * @brief Arbitrary for int16_t
 */
template <>
class PROPTEST_API Arbi<int16_t> final : public ArbiBase<int16_t> {
public:
    Shrinkable<int16_t> operator()(Random& rand) const override;
    static constexpr int16_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 numeric_limits<int16_t>::min(),
                                                 numeric_limits<int16_t>::max(),
                                                 numeric_limits<int16_t>::min() + 1,
                                                 numeric_limits<int16_t>::max() - 1,
                                                 numeric_limits<int8_t>::min(),
                                                 numeric_limits<int8_t>::max(),
                                                 numeric_limits<uint8_t>::max(),
                                                 numeric_limits<int8_t>::min() - 1,
                                                 numeric_limits<int8_t>::min() + 1,
                                                 numeric_limits<int8_t>::max() - 1,
                                                 numeric_limits<int8_t>::max() + 1,
                                                 numeric_limits<uint8_t>::max() - 1,
                                                 numeric_limits<uint8_t>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for int32_t
 */
template <>
struct PROPTEST_API Arbi<int32_t> final : public ArbiBase<int32_t>
{
public:
    Shrinkable<int32_t> operator()(Random& rand) const override;
    static constexpr int32_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 numeric_limits<int32_t>::min(),
                                                 numeric_limits<int32_t>::max(),
                                                 numeric_limits<int32_t>::min() + 1,
                                                 numeric_limits<int32_t>::max() - 1,
                                                 numeric_limits<int16_t>::min(),
                                                 numeric_limits<int16_t>::max(),
                                                 numeric_limits<uint16_t>::max(),
                                                 numeric_limits<int16_t>::min() - 1,
                                                 numeric_limits<int16_t>::min() + 1,
                                                 numeric_limits<int16_t>::max() - 1,
                                                 numeric_limits<int16_t>::max() + 1,
                                                 numeric_limits<uint16_t>::max() - 1,
                                                 numeric_limits<uint16_t>::max() + 1,
                                                 numeric_limits<int8_t>::min(),
                                                 numeric_limits<int8_t>::max(),
                                                 numeric_limits<uint8_t>::max(),
                                                 numeric_limits<int8_t>::min() - 1,
                                                 numeric_limits<int8_t>::min() + 1,
                                                 numeric_limits<int8_t>::max() - 1,
                                                 numeric_limits<int8_t>::max() + 1,
                                                 numeric_limits<uint8_t>::max() - 1,
                                                 numeric_limits<uint8_t>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for int64_t
 */
template <>
struct PROPTEST_API Arbi<int64_t> final : public ArbiBase<int64_t>
{
public:
    Shrinkable<int64_t> operator()(Random& rand) const override;

    static constexpr int64_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 numeric_limits<int64_t>::min(),
                                                 numeric_limits<int64_t>::max(),
                                                 numeric_limits<int64_t>::min() + 1,
                                                 numeric_limits<int64_t>::max() - 1,
                                                 numeric_limits<int32_t>::min(),
                                                 numeric_limits<int32_t>::max(),
                                                 numeric_limits<uint32_t>::max(),
                                                 static_cast<int64_t>(numeric_limits<int32_t>::min()) - 1,
                                                 numeric_limits<int32_t>::min() + 1,
                                                 numeric_limits<int32_t>::max() - 1,
                                                 static_cast<int64_t>(numeric_limits<int32_t>::max()) + 1,
                                                 numeric_limits<uint32_t>::max() - 1,
                                                 static_cast<int64_t>(numeric_limits<uint32_t>::max()) + 1,
                                                 numeric_limits<int16_t>::min(),
                                                 numeric_limits<int16_t>::max(),
                                                 numeric_limits<uint16_t>::max(),
                                                 numeric_limits<int16_t>::min() - 1,
                                                 numeric_limits<int16_t>::min() + 1,
                                                 numeric_limits<int16_t>::max() - 1,
                                                 numeric_limits<int16_t>::max() + 1,
                                                 numeric_limits<uint16_t>::max() - 1,
                                                 numeric_limits<uint16_t>::max() + 1,
                                                 numeric_limits<int8_t>::min(),
                                                 numeric_limits<int8_t>::max(),
                                                 numeric_limits<uint8_t>::max(),
                                                 numeric_limits<int8_t>::min() - 1,
                                                 numeric_limits<int8_t>::min() + 1,
                                                 numeric_limits<int8_t>::max() - 1,
                                                 numeric_limits<int8_t>::max() + 1,
                                                 numeric_limits<uint8_t>::max() - 1,
                                                 numeric_limits<uint8_t>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint8_t
 */
template <>
class PROPTEST_API Arbi<uint8_t> final : public ArbiBase<uint8_t> {
public:
    Shrinkable<uint8_t> operator()(Random& rand) const override;

    static constexpr uint8_t boundaryValues[] = {0, 1, 2,
                                                numeric_limits<uint8_t>::max(),
                                                numeric_limits<uint8_t>::max() - 1,
                                                numeric_limits<int8_t>::max(),
                                                numeric_limits<int8_t>::max() - 1,
                                                numeric_limits<int8_t>::max() + 1,
                                                ' ', '"', '\'', '\t', '\n', '\r'};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint16_t
 */
template <>
class PROPTEST_API Arbi<uint16_t> final : public ArbiBase<uint16_t> {
public:
    Shrinkable<uint16_t> operator()(Random& rand) const override;

    static constexpr uint16_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  numeric_limits<uint16_t>::max(),
                                                  numeric_limits<uint16_t>::max() - 1,
                                                  numeric_limits<int16_t>::max(),
                                                  numeric_limits<int16_t>::max() - 1,
                                                  numeric_limits<int16_t>::max() + 1,
                                                  numeric_limits<int8_t>::max(),
                                                  numeric_limits<uint8_t>::max(),
                                                  numeric_limits<int8_t>::max() + 1,
                                                  numeric_limits<int8_t>::max() - 1,
                                                  numeric_limits<uint8_t>::max() - 1,
                                                  numeric_limits<uint8_t>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint32_t
 */
template <>
struct PROPTEST_API Arbi<uint32_t> final : public ArbiBase<uint32_t>
{
public:
    Shrinkable<uint32_t> operator()(Random& rand) const override;

    static constexpr uint32_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  numeric_limits<uint32_t>::max(),
                                                  numeric_limits<uint32_t>::max() - 1,
                                                  numeric_limits<int32_t>::max(),
                                                  numeric_limits<int32_t>::max() - 1,
                                                  static_cast<uint32_t>(numeric_limits<int32_t>::max()) + 1,
                                                  numeric_limits<int16_t>::max(),
                                                  numeric_limits<uint16_t>::max(),
                                                  numeric_limits<int16_t>::max() - 1,
                                                  numeric_limits<int16_t>::max() + 1,
                                                  numeric_limits<uint16_t>::max() - 1,
                                                  numeric_limits<uint16_t>::max() + 1,
                                                  numeric_limits<int8_t>::max(),
                                                  numeric_limits<uint8_t>::max(),
                                                  numeric_limits<int8_t>::max() + 1,
                                                  numeric_limits<int8_t>::max() - 1,
                                                  numeric_limits<uint8_t>::max() - 1,
                                                  numeric_limits<uint8_t>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint64_t
 */
template <>
struct PROPTEST_API Arbi<uint64_t> : public ArbiBase<uint64_t>
{
public:
    Shrinkable<uint64_t> operator()(Random& rand) const override;

    static constexpr uint64_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  numeric_limits<uint64_t>::max(),
                                                  numeric_limits<uint64_t>::max() - 1,
                                                  numeric_limits<int32_t>::max(),
                                                  numeric_limits<uint32_t>::max(),
                                                  numeric_limits<int32_t>::max() - 1,
                                                  static_cast<int64_t>(numeric_limits<int32_t>::max()) + 1,
                                                  numeric_limits<uint32_t>::max() - 1,
                                                  static_cast<int64_t>(numeric_limits<uint32_t>::max()) + 1,
                                                  numeric_limits<int16_t>::max(),
                                                  numeric_limits<uint16_t>::max(),
                                                  numeric_limits<int16_t>::max() - 1,
                                                  numeric_limits<int16_t>::max() + 1,
                                                  numeric_limits<uint16_t>::max() - 1,
                                                  numeric_limits<uint16_t>::max() + 1,
                                                  numeric_limits<int8_t>::max(),
                                                  numeric_limits<uint8_t>::max(),
                                                  numeric_limits<int8_t>::max() + 1,
                                                  numeric_limits<int8_t>::max() - 1,
                                                  numeric_limits<uint8_t>::max() - 1,
                                                  numeric_limits<uint8_t>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for long
 */
template <>
struct PROPTEST_API Arbi<long> final : public ArbiBase<long>
{
public:
    Shrinkable<long> operator()(Random& rand) const override;

    static constexpr long boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 numeric_limits<long>::min(),
                                                 numeric_limits<long>::min() + 1,
                                                 numeric_limits<long>::max(),
                                                 numeric_limits<long>::max() - 1,
                                                 numeric_limits<int>::min(),
                                                 numeric_limits<int>::min() + 1,
                                                 static_cast<long>(numeric_limits<int>::min()) - 1,
                                                 numeric_limits<int>::max(),
                                                 numeric_limits<int>::max() - 1,
                                                 static_cast<long>(numeric_limits<int>::max()) + 1,
                                                 numeric_limits<unsigned int>::max(),
                                                 numeric_limits<unsigned int>::max() - 1,
                                                 static_cast<long>(numeric_limits<unsigned int>::max()) + 1,
                                                 numeric_limits<short>::min(),
                                                 numeric_limits<short>::min() - 1,
                                                 numeric_limits<short>::min() + 1,
                                                 numeric_limits<short>::max(),
                                                 numeric_limits<short>::max() - 1,
                                                 numeric_limits<short>::max() + 1,
                                                 numeric_limits<unsigned short>::max(),
                                                 numeric_limits<unsigned short>::max() - 1,
                                                 numeric_limits<unsigned short>::max() + 1,
                                                 numeric_limits<signed char>::min(),
                                                 numeric_limits<signed char>::min() - 1,
                                                 numeric_limits<signed char>::min() + 1,
                                                 numeric_limits<signed char>::max(),
                                                 numeric_limits<signed char>::max() - 1,
                                                 numeric_limits<signed char>::max() + 1,
                                                 numeric_limits<unsigned char>::max(),
                                                 numeric_limits<unsigned char>::max() - 1,
                                                 numeric_limits<unsigned char>::max() + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for unsigned long
 */
template <>
struct PROPTEST_API Arbi<unsigned long> : public ArbiBase<unsigned long>
{
public:
    Shrinkable<unsigned long> operator()(Random& rand) const override;

    static constexpr unsigned long boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  numeric_limits<unsigned long>::max(),
                                                  numeric_limits<unsigned long>::max() - 1,
                                                  numeric_limits<int>::max(),
                                                  numeric_limits<unsigned int>::max(),
                                                  numeric_limits<int>::max() - 1,
                                                  static_cast<long>(numeric_limits<int>::max()) + 1,
                                                  numeric_limits<unsigned int>::max() - 1,
                                                  static_cast<long>(numeric_limits<unsigned int>::max()) + 1,
                                                  numeric_limits<short>::max(),
                                                  numeric_limits<unsigned short>::max(),
                                                  numeric_limits<short>::max() - 1,
                                                  numeric_limits<short>::max() + 1,
                                                  numeric_limits<unsigned short>::max() - 1,
                                                  numeric_limits<unsigned short>::max() + 1,
                                                  numeric_limits<signed char>::max(),
                                                  numeric_limits<unsigned char>::max(),
                                                  numeric_limits<signed char>::max() + 1,
                                                  numeric_limits<signed char>::max() - 1,
                                                  numeric_limits<unsigned char>::max() - 1,
                                                  numeric_limits<unsigned char>::max() + 1};
};


namespace util {

// would've been simpler with lambdas, but optimizing for compilation performance

template <typename T>
struct NaturalFunctor {
    NaturalFunctor(T _max) : max(_max) {}
    Shrinkable<T> operator()(Random& rand) { return generateInteger<T>(rand, 1, max); }
    T max;
};

template <typename T>
struct NonNegativeFunctor {
    NonNegativeFunctor(T _max) : max(_max) {}
    Shrinkable<T> operator()(Random& rand) { return generateInteger<T>(rand, 0, max); }
    T max;
};

template <typename T>
struct IntervalFunctor {
    IntervalFunctor(T _min, T _max) : min(_min), max(_max) {}
    Shrinkable<T> operator()(Random& rand) { return generateInteger<T>(rand, min, max); }
    T min;
    T max;
};

template <typename T>
struct InRangeFunctor {
    InRangeFunctor(T _from, T _to) : from(_from), to(_to) {}
    Shrinkable<T> operator()(Random& rand) { return generateInteger<T>(rand, from, to - 1); }
    T from;
    T to;
};

template <typename T>
struct IntegersFunctor {
    IntegersFunctor(T _start, T _count) : start(_start), count(_count) {}
    Shrinkable<T> operator()(Random& rand) { return generateInteger<T>(rand, start, static_cast<T>(start + count - 1)); }
    T start;
    T count;
};

} // namespace util

/**
 * @ingroup Generators
 * @brief Generates a positive integer, excluding 0
 */
template <proptest::integral T>
PROPTEST_API Generator<T> natural(T max = numeric_limits<T>::max())
{
    return Generator<T>(util::NaturalFunctor<T>(max));
}

/**
 * @ingroup Generators
 * @brief Generates 0 or a positive integer
 */
template <proptest::integral T>
PROPTEST_API Generator<T> nonNegative(T max = numeric_limits<T>::max())
{
    return Generator<T>(util::NonNegativeFunctor<T>(max));
}

/**
 * @ingroup Generators
 * @brief Generates numeric values in [min, max]. e.g. interval(0,100) generates a value in {0, ..., 100}
 */
template <proptest::integral T>
PROPTEST_API Generator<T> interval(T min, T max)
{
    return Generator<T>([min, max](Random& rand) { return generateInteger<T>(rand, min, max); });
}

/**
 * @ingroup Generators
 * @brief Generates numeric values in [from, to). e.g. inRange(0,100) generates a value in {0, ..., 99}
 */
template <proptest::integral T>
PROPTEST_API Generator<T> inRange(T from, T to)
{
    return Generator<T>(util::InRangeFunctor(from, to));
}

/**
 * @ingroup Generators
 * @brief Generates numeric values in [a, a+count). e.g. integers(0,100) generates a value in {0, ..., 99}
 */
template <proptest::integral T>
PROPTEST_API Generator<T> integers(T start, T count)
{
    return Generator<T>(util::IntegersFunctor<T>(start, count));
}

}  // namespace proptest

// for template instantiations
#define EXTERN_GENERATEINTEGER(TYPE) \
    extern template proptest::Shrinkable<TYPE> generateInteger(proptest::Random& rand, TYPE min, TYPE max)

#define EXTERN_NATURAL(TYPE) \
    extern template proptest::Generator<TYPE> natural(TYPE max)

#define EXTERN_NONNEGATIVE(TYPE) \
    extern template proptest::Generator<TYPE> nonNegative(TYPE max)

#define EXTERN_INTERVAL(TYPE) \
    extern template proptest::Generator<TYPE> interval(TYPE min, TYPE max)

#define EXTERN_INRANGE(TYPE) \
    extern template proptest::Generator<TYPE> inRange(TYPE from, TYPE to)

#define EXTERN_INTEGERS(TYPE) \
    extern template proptest::Generator<TYPE> integers(TYPE start, TYPE count)




#define DEFINE_GENERATEINTEGER(TYPE) \
    template proptest::Shrinkable<TYPE> generateInteger(proptest::Random& rand, TYPE min, TYPE max)

#define DEFINE_NATURAL(TYPE) \
    template proptest::Generator<TYPE> natural(TYPE max)

#define DEFINE_NONNEGATIVE(TYPE) \
    template proptest::Generator<TYPE> nonNegative(TYPE max)

#define DEFINE_INTERVAL(TYPE) \
    template proptest::Generator<TYPE> interval(TYPE min, TYPE max)

#define DEFINE_INRANGE(TYPE) \
    template proptest::Generator<TYPE> inRange(TYPE from, TYPE to)

#define DEFINE_INTEGERS(TYPE) \
    template proptest::Generator<TYPE> integers(TYPE start, TYPE count)

namespace proptest {
// template instantiation
DEFINE_FOR_ALL_INTTYPES(EXTERN_INTEGERS);
// DEFINE_FOR_ALL_INTTYPES(EXTERN_GENERATEINTEGER);
DEFINE_FOR_ALL_INTTYPES(EXTERN_NATURAL);
DEFINE_FOR_ALL_INTTYPES(EXTERN_NONNEGATIVE);
DEFINE_FOR_ALL_INTTYPES(EXTERN_INTERVAL);
DEFINE_FOR_ALL_INTTYPES(EXTERN_INRANGE);

}// namespace proptest
