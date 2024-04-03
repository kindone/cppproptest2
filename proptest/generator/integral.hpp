#pragma once
#include "proptest/Stream.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/concepts.hpp"

/**
 * @file integral.hpp
 * @brief Arbitrary for integers
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);


// template declaration
template <typename T>
PROPTEST_API Shrinkable<T> generateInteger(Random& rand, T min = numeric_limits<T>::min(), T max = numeric_limits<T>::max());

// template specialization
template <> PROPTEST_API Shrinkable<char> generateInteger(Random& rand, char min, char max);

template <> PROPTEST_API Shrinkable<int8_t> generateInteger(Random& rand, int8_t min, int8_t max);

template <> PROPTEST_API Shrinkable<int16_t> generateInteger(Random& rand, int16_t min, int16_t max);

template <> PROPTEST_API Shrinkable<int32_t> generateInteger(Random& rand, int32_t min, int32_t max);

template <> PROPTEST_API Shrinkable<int64_t> generateInteger(Random& rand, int64_t min, int64_t max);

template <> PROPTEST_API Shrinkable<uint8_t> generateInteger(Random& rand, uint8_t min, uint8_t max);

template <> PROPTEST_API Shrinkable<uint16_t> generateInteger(Random& rand, uint16_t min, uint16_t max);

template <> PROPTEST_API Shrinkable<uint32_t> generateInteger(Random& rand, uint32_t min, uint32_t max);

template <> PROPTEST_API Shrinkable<uint64_t> generateInteger(Random& rand, uint64_t min, uint64_t max);

/**
 * @ingroup Generators
 * @brief Arbitrary for int8_t
 */
template <>
class PROPTEST_API Arbi<int8_t> final : public ArbiBase<int8_t> {
public:
    virtual Shrinkable<int8_t> operator()(Random& rand) const override;
    static constexpr int8_t boundaryValues[] = {INT8_MIN,     0,   INT8_MAX, -1,   1,    -2,   2,   INT8_MIN + 1,
                                                INT8_MAX - 1, ' ', '"',      '\'', '\t', '\n', '\r'};

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
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
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
                                                 INT32_MIN,
                                                 INT32_MAX,
                                                 INT32_MIN + 1,
                                                 INT32_MAX - 1,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 UINT16_MAX,
                                                 INT16_MIN - 1,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT16_MAX + 1,
                                                 UINT16_MAX - 1,
                                                 UINT16_MAX + 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
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
                                                 INT64_MIN,
                                                 INT64_MAX,
                                                 INT64_MIN + 1,
                                                 INT64_MAX - 1,
                                                 INT32_MIN,
                                                 INT32_MAX,
                                                 UINT32_MAX,
                                                 static_cast<int64_t>(INT32_MIN) - 1,
                                                 INT32_MIN + 1,
                                                 INT32_MAX - 1,
                                                 static_cast<int64_t>(INT32_MAX) + 1,
                                                 UINT32_MAX - 1,
                                                 static_cast<int64_t>(UINT32_MAX) + 1,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 UINT16_MAX,
                                                 INT16_MIN - 1,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT16_MAX + 1,
                                                 UINT16_MAX - 1,
                                                 UINT16_MAX + 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for char
 */
template <>
class PROPTEST_API Arbi<char> final : public ArbiBase<char> {
public:
    Shrinkable<char> operator()(Random& rand) const override;

    static constexpr char boundaryValues[] = {0};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint8_t
 */
template <>
class PROPTEST_API Arbi<uint8_t> final : public ArbiBase<uint8_t> {
public:
    Shrinkable<uint8_t> operator()(Random& rand) const override;

    static constexpr uint8_t boundaryValues[] = {
        0, 1, 2, UINT8_MAX, UINT8_MAX - 1, INT8_MAX, INT8_MAX - 1, INT8_MAX + 1, ' ', '"', '\'', '\t', '\n', '\r'};
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
                                                  UINT16_MAX,
                                                  UINT16_MAX - 1,
                                                  INT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
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
                                                  UINT32_MAX,
                                                  UINT32_MAX - 1,
                                                  INT32_MAX,
                                                  INT32_MAX - 1,
                                                  static_cast<uint32_t>(INT32_MAX) + 1,
                                                  INT16_MAX,
                                                  UINT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  UINT16_MAX - 1,
                                                  UINT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
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
                                                  UINT64_MAX,
                                                  UINT64_MAX - 1,
                                                  INT32_MAX,
                                                  UINT32_MAX,
                                                  INT32_MAX - 1,
                                                  static_cast<int64_t>(INT32_MAX) + 1,
                                                  UINT32_MAX - 1,
                                                  static_cast<int64_t>(UINT32_MAX) + 1,
                                                  INT16_MAX,
                                                  UINT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  UINT16_MAX - 1,
                                                  UINT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
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

#define DEFINE_FOR_ALL_INTTYPES(DEF) \
    DEF(int8_t);\
    DEF(int16_t);\
    DEF(int32_t);\
    DEF(int64_t);\
    DEF(uint8_t);\
    DEF(uint16_t);\
    DEF(uint32_t);\
    DEF(uint64_t)


namespace proptest {
// template instantiation
DEFINE_FOR_ALL_INTTYPES(EXTERN_INTEGERS);
// DEFINE_FOR_ALL_INTTYPES(EXTERN_GENERATEINTEGER);
DEFINE_FOR_ALL_INTTYPES(EXTERN_NATURAL);
DEFINE_FOR_ALL_INTTYPES(EXTERN_NONNEGATIVE);
DEFINE_FOR_ALL_INTTYPES(EXTERN_INTERVAL);
DEFINE_FOR_ALL_INTTYPES(EXTERN_INRANGE);

}// namespace proptest
