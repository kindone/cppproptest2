#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/bool.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/shrinker/pair.hpp"
#include "gtest/gtest.h"
#include "proptest/test/testutil.hpp"

using namespace proptest;

template <typename T>
struct NumericTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct SignedNumericTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct IntegralTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct SignedIntegralTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct UnsignedIntegralTest : public testing::Test
{
    using NumericType = T;
};

using NumericTypes =
    testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double>;
using SignedNumericTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, float, double>;
using IntegralTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>;
using SignedIntegralTypes = testing::Types<int8_t, int16_t, int32_t, int64_t>;
using UnsignedIntegralTypes = testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
using FloatingTypes = testing::Types<float, double>;

TYPED_TEST_SUITE(NumericTest, NumericTypes);
TYPED_TEST_SUITE(SignedNumericTest, SignedNumericTypes);
TYPED_TEST_SUITE(IntegralTest, IntegralTypes);
TYPED_TEST_SUITE(SignedIntegralTest, SignedIntegralTypes);
TYPED_TEST_SUITE(UnsignedIntegralTest, UnsignedIntegralTypes);
TYPED_TEST_SUITE(FloatingTest, FloatingTypes);

TYPED_TEST(IntegralTest, shrinkIntegral_signed)
{
    auto shr = shrinkIntegral<TypeParam>(8);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: "
        "[{value: 0}, "
        "{value: 4, shrinks: "
            "[{value: 2, shrinks: "
                "[{value: 1}]}, "
            "{value: 3}]}, "
        "{value: 6, shrinks: "
            "[{value: 5}]}, "
        "{value: 7}]}");
}

TYPED_TEST(SignedIntegralTest, shrinkIntegral_unsigned)
{
    auto shr = shrinkIntegral<TypeParam>(-8);
    EXPECT_EQ(serializeShrinkable(shr), "{value: -8, shrinks: "
        "[{value: 0}, "
        "{value: -4, shrinks: "
            "[{value: -2, shrinks: "
                "[{value: -1}]}, "
            "{value: -3}]}, "
        "{value: -6, shrinks: "
            "[{value: -5}]}, "
        "{value: -7}]}");
}

TYPED_TEST(SignedIntegralTest, shrinkIntegral_unsigned2)
{
    // concat with positive signed number
    auto shr = shrinkIntegral<TypeParam>(-8).concat([](const Shrinkable<TypeParam>& shr) {
        if(shr.get() == 0)
            return TypedStream<Shrinkable<TypeParam>>::empty();
        return TypedStream<Shrinkable<TypeParam>>::one(Shrinkable<TypeParam>(-shr.get()));
    });
    EXPECT_EQ(serializeShrinkable(shr), "{value: -8, shrinks: [{value: 0}, {value: -4, shrinks: [{value: -2, shrinks: [{value: -1, shrinks: [{value: 1}]}, {value: 2}]}, {value: -3, shrinks: [{value: 3}]}, {value: 4}]}, {value: -6, shrinks: [{value: -5, shrinks: [{value: 5}]}, {value: 6}]}, {value: -7, shrinks: [{value: 7}]}, {value: 8}]}");
}

TEST(BoolShrinker, basic)
{
    auto shr = shrinkBool(true);
    EXPECT_EQ(serializeShrinkable(shr), "{value: true, shrinks: [{value: false}]}");
}

TEST(FloatingShrinker, basic)
{
    // FIXME: shrinker  not good enough
    auto shr = shrinkFloat(1.0f);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 1, shrinks: [{value: 0}, {value: 0.5, shrinks: [{value: 0.5}]}]}");
}

TEST(PairShrinker, basic)
{
    auto shr = shrinkPair(shrinkIntegral<int>(1), shrinkIntegral<int>(2));
    EXPECT_EQ(serializeShrinkable(shr), "{value: (1, 2), shrinks: [{value: (0, 2), shrinks: [{value: (0, 0)}, {value: (0, 1)}]}, {value: (1, 0)}, {value: (1, 1)}]}");
}