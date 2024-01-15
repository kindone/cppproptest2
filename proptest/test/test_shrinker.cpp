#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/bool.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/shrinker/pair.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/shrinker/set.hpp"
#include "proptest/shrinker/string.hpp"
#include "proptest/shrinker/stringlike.hpp"
#include "proptest/shrinker/tuple.hpp"
#include "proptest/util/utf8string.hpp"
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
            return Stream<Shrinkable<TypeParam>>::empty();
        return Stream<Shrinkable<TypeParam>>::one(Shrinkable<TypeParam>(-shr.get()));
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

TEST(PairShrinker, ints)
{
    auto shr = shrinkPair(shrinkIntegral<int>(1), shrinkIntegral<int>(2));
    EXPECT_EQ(serializeShrinkable(shr), "{value: (1, 2), shrinks: [{value: (0, 2), shrinks: [{value: (0, 0)}, {value: (0, 1)}]}, {value: (1, 0)}, {value: (1, 1)}]}");
}

TEST(PairShrinker, double)
{
    auto toDouble = +[](const int& i) { return (double)i; };
    auto shr = shrinkPair<double, double>(shrinkIntegral<int>(1).map<double>(toDouble), shrinkIntegral<int>(2).map<double>(toDouble));
    EXPECT_EQ(serializeShrinkable(shr), "{value: (1, 2), shrinks: [{value: (0, 2), shrinks: [{value: (0, 0)}, {value: (0, 1)}]}, {value: (1, 0)}, {value: (1, 1)}]}");
    printExhaustive(shr);
}

TEST(PairShrinker, strings)
{
    auto toString = +[](const int& i) { return to_string(i); };
    auto shr = shrinkPair(shrinkIntegral<int>(1).map<string>(toString), shrinkIntegral<int>(2).map<string>(toString));
    EXPECT_EQ(serializeShrinkable(shr), "{value: (\"1\" (31), \"2\" (32)), shrinks: [{value: (\"0\" (30), \"2\" (32)), shrinks: [{value: (\"0\" (30), \"0\" (30))}, {value: (\"0\" (30), \"1\" (31))}]}, {value: (\"1\" (31), \"0\" (30))}, {value: (\"1\" (31), \"1\" (31))}]}");
}

TEST(PairShrinker, different_types)
{
    auto shr = shrinkPair(shrinkIntegral<int>(1), shrinkIntegral<int>(2).map<string>([](int i) { return to_string(i); }));
    EXPECT_EQ(serializeShrinkable(shr), "{value: (1, \"2\" (32)), shrinks: [{value: (0, \"2\" (32)), shrinks: [{value: (0, \"0\" (30))}, {value: (0, \"1\" (31))}]}, {value: (1, \"0\" (30))}, {value: (1, \"1\" (31))}]}");
}

TEST(ListLikeShrinker, ints)
{
    Shrinkable<vector<ShrinkableAny>> baseShr = make_shrinkable<vector<ShrinkableAny>, initializer_list<ShrinkableAny>>({
        ShrinkableAny(shrinkIntegral<int>(1)),
        ShrinkableAny(shrinkIntegral<int>(2)),
        ShrinkableAny(shrinkIntegral<int>(3)),
        ShrinkableAny(shrinkIntegral<int>(4)),
        ShrinkableAny(shrinkIntegral<int>(5)),
        ShrinkableAny(shrinkIntegral<int>(6)),
        ShrinkableAny(shrinkIntegral<int>(7)),
        ShrinkableAny(shrinkIntegral<int>(8))});

    Shrinkable<vector<int>> shr0 = shrinkListLike<vector, int>(baseShr, 0, false, false);
    Shrinkable<vector<int>> shr1 = shrinkListLike<vector, int>(baseShr, 0, true, false);
    Shrinkable<vector<int>> shr2 = shrinkListLike<vector, int>(baseShr, 0, false, true);
    Shrinkable<vector<int>> shr3 = shrinkListLike<vector, int>(baseShr, 0, true, true);

    EXPECT_EQ(shr0.getRef(), vector<int>({1,2,3,4,5,6,7,8}));
    EXPECT_EQ(shr1.getRef(), vector<int>({1,2,3,4,5,6,7,8}));
    EXPECT_EQ(shr2.getRef(), vector<int>({1,2,3,4,5,6,7,8}));
    EXPECT_EQ(shr3.getRef(), vector<int>({1,2,3,4,5,6,7,8}));

    EXPECT_EQ(serializeShrinkable(shr0), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ]}");
    EXPECT_EQ(serializeShrinkable(shr1), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ], shrinks: [{value: [ 0, 0, 0, 0, 0, 0, 0, 0 ]}, {value: [ 1, 1, 1, 2, 2, 3, 3, 4 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 2 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 1 ]}]}, {value: [ 1, 1, 1, 2, 2, 2, 2, 3 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 2, 2, 3 ]}]}]}, {value: [ 1, 2, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 0, 2, 3, 3, 4, 4, 5 ]}, {value: [ 1, 1, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 1, 2, 3, 3, 4, 4, 5 ]}]}]}, {value: [ 1, 2, 3, 4, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 0, 0, 4, 5, 6, 7 ]}, {value: [ 1, 1, 1, 2, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 1, 1, 4, 5, 6, 7 ]}]}, {value: [ 1, 2, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 2, 3, 4, 5, 6, 7 ]}, {value: [ 1, 1, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 2, 3, 4, 5, 6, 7 ]}]}]}]}]}");
    // EXPECT_EQ(serializeShrinkable(shr2), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ], shrinks: [{value: [ 0, 0, 0, 0, 0, 0, 0, 0 ]}, {value: [ 1, 1, 1, 2, 2, 3, 3, 4 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 2 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 1 ]}]}, {value: [ 1, 1, 1, 2, 2, 2, 2, 3 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 2, 2, 3 ]}]}]}, {value: [ 1, 2, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 0, 2, 3, 3, 4, 4, 5 ]}, {value: [ 1, 1, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 1, 2, 3, 3, 4, 4, 5 ]}]}]}, {value: [ 1, 2, 3, 4, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 0, 0, 4, 5, 6, 7 ]}, {value: [ 1, 1, 1, 2, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 1, 1, 4, 5, 6, 7 ]}]}, {value: [ 1, 2, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 2, 3, 4, 5, 6, 7 ]}, {value: [ 1, 1, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 2, 3, 4, 5, 6, 7 ]}]}]}]}]}");
    // EXPECT_EQ(serializeShrinkable(shr3), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ], shrinks: [{value: [ 0, 0, 0, 0, 0, 0, 0, 0 ]}, {value: [ 1, 1, 1, 2, 2, 3, 3, 4 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 2 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 1 ]}]}, {value: [ 1, 1, 1, 2, 2, 2, 2, 3 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 2, 2, 3 ]}]}]}, {value: [ 1, 2, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 0, 2, 3, 3, 4, 4, 5 ]}, {value: [ 1, 1, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 1, 2, 3, 3, 4, 4, 5 ]}]}]}, {value: [ 1, 2, 3, 4, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 0, 0, 4, 5, 6, 7 ]}, {value: [ 1, 1, 1, 2, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 1, 1, 4, 5, 6, 7 ]}]}, {value: [ 1, 2, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 2, 3, 4, 5, 6, 7 ]}, {value: [ 1, 1, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 2, 3, 4, 5, 6, 7 ]}]}]}]}]}");
}

TEST(SetShrinker, ints)
{
    Shrinkable<set<Shrinkable<int>>> baseShr = make_shrinkable<set<Shrinkable<int>>,initializer_list<Shrinkable<int>>>({
        shrinkIntegral<int>(1),
        shrinkIntegral<int>(2)});
    Shrinkable<set<int>> shr = shrinkSet<int>(baseShr, 0);
    EXPECT_EQ(shr.getRef(), set<int>({ 1, 2 }));
    EXPECT_EQ(serializeShrinkable(shr), "{value: { 1, 2 }, shrinks: [{value: {  }}, {value: { 1 }, shrinks: [{value: { 0 }}]}, {value: { 2 }, shrinks: [{value: { 0 }}, {value: { 1 }}]}]}");
}

TEST(StringShrinker, basic)
{
    auto shr = shrinkString("abc", 0);
    EXPECT_EQ(serializeShrinkable(shr), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"bc\" (62 63)}, {value: \"c\" (63)}]}");
}

// membershipwise + elementwise
TEST(shrinkContainer, string)
{
    auto fwd_converter = [](const string& str) -> vector<Shrinkable<uint32_t>> {
        vector<Shrinkable<uint32_t>> shrinks;
        shrinks.reserve(str.size());
        for(auto& c : str)
            shrinks.push_back(make_shrinkable<uint32_t>(static_cast<uint32_t>(c)));
        return shrinks;
    };

    auto back_converter = [](const vector<uint32_t>& vec) {
        string str;
        str.reserve(vec.size());
        for(auto& c : vec)
            str.push_back(static_cast<char>(c));
        return str;
    };

    auto shr1 = shrinkContainer<vector, uint32_t>(Shrinkable(fwd_converter("abc")), 0, false, true).map<string>(back_converter);
    EXPECT_EQ(serializeShrinkable(shr1), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"c\" (63)}, {value: \"ac\" (61 63)}, {value: \"bc\" (62 63)}]}");
    auto shr2 = shrinkContainer<vector, uint32_t>(Shrinkable(fwd_converter("abc")), 0, true, false).map<string>(back_converter);
    EXPECT_EQ(serializeShrinkable(shr2), "{value: \"abc\" (61 62 63)}");
    auto shr3 = shrinkContainer<vector, uint32_t>(Shrinkable(fwd_converter("abc")), 0, true, true).map<string>(back_converter);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"c\" (63)}, {value: \"ac\" (61 63)}, {value: \"bc\" (62 63)}]}");
}

// same as StringShrinker, but with UTF8String/UTF16String
TEST(StringLikeShrinker, basic)
{
    vector<int> bytePositions({0,1,2,3}); // need 4 for 3 chars
    auto shr = shrinkStringLike<UTF8String>("abc", 0, 3, bytePositions);
    EXPECT_EQ(serializeShrinkable(shr), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"bc\" (62 63)}, {value: \"c\" (63)}]}");
}

TEST(TupleShrinker, basic)
{
    // auto shr1 = shrinkIntegral(2);
    // auto shr2 = shrinkString("ab", 0);
    // auto shr3 = shrinkBool(true);
    // auto shr = shrinkTuple(Shrinkable(util::make_tuple(shr1, shr2, shr3)));
    // EXPECT_EQ(serializeShrinkable(shr), "{value: (2, \"ab\" (61 62), true), shrinks: [{value: (0, \"ab\" (61 62), true)}, {value: (1, \"ab\" (61 62), true)}, {value: (2, \"\" (), true)}, {value: (2, \"a\" (61), true), shrinks: [{value: (2, \"\" (), true)}]}, {value: (2, \"b\" (62), true), shrinks: [{value: (2, \"\" (), true)}, {value: (2, \"a\" (61), true), shrinks: [{value: (2, \"\" (), true)}]}]}, {value: (2, \"ab\" (61 62), false)}]}");
}