#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/bool.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/shrinker/pair.hpp"
#include "proptest/shrinker/tuple.hpp"
#include "proptest/shrinker/string.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

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

TYPED_TEST(SignedIntegralTest, shrinkIntegral_signed)
{
    Shrinkable<TypeParam> shr = shrinkIntegral<TypeParam>(-8);
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

TYPED_TEST(SignedIntegralTest, shrinkIntegral_signed2)
{
    // concat with positive signed number
    using StreamType = Shrinkable<TypeParam>::StreamType;
    using Elem = Shrinkable<TypeParam>::StreamElementType;
    Shrinkable<TypeParam> shr = shrinkIntegral<TypeParam>(-8).concat([](const Elem& shr) {
        Shrinkable<TypeParam> inShr = shr;
        if(inShr.get() == 0)
            return StreamType::empty();
        return StreamType::template one<Elem>(Shrinkable<TypeParam>(-inShr.get()));
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

TEST(TupleShrinker, basic)
{
    auto shr1 = shrinkIntegral(2);
    auto shr2 = shrinkString("ab", 0);
    auto shr3 = shrinkBool(true);
    auto shr = shrinkTuple(Shrinkable<tuple<Shrinkable<int>,Shrinkable<string>,Shrinkable<bool>>>(util::make_tuple(shr1, shr2, shr3)));
    EXPECT_EQ(serializeShrinkable(shr),
        "{value: { 2, \"ab\" (61 62), true }, shrinks: "
            "[{value: { 0, \"ab\" (61 62), true }, shrinks: "
                "[{value: { 0, \"\" (), true }, shrinks: "
                    "[{value: { 0, \"\" (), false }}]}, "
                "{value: { 0, \"a\" (61), true }, shrinks: "
                    "[{value: { 0, \"a\" (61), false }}]}, "
                "{value: { 0, \"b\" (62), true }, shrinks: "
                    "[{value: { 0, \"b\" (62), false }}]}, "
                "{value: { 0, \"ab\" (61 62), false }}]}, "
            "{value: { 1, \"ab\" (61 62), true }, shrinks: "
                "[{value: { 1, \"\" (), true }, shrinks: "
                    "[{value: { 1, \"\" (), false }}]}, "
                "{value: { 1, \"a\" (61), true }, shrinks: "
                    "[{value: { 1, \"a\" (61), false }}]}, "
                "{value: { 1, \"b\" (62), true }, shrinks: "
                    "[{value: { 1, \"b\" (62), false }}]}, "
                "{value: { 1, \"ab\" (61 62), false }}]}, "
            "{value: { 2, \"\" (), true }, shrinks: "
                "[{value: { 2, \"\" (), false }}]}, "
            "{value: { 2, \"a\" (61), true }, shrinks: "
                "[{value: { 2, \"a\" (61), false }}]}, "
            "{value: { 2, \"b\" (62), true }, shrinks: "
                "[{value: { 2, \"b\" (62), false }}]}, "
            "{value: { 2, \"ab\" (61 62), false }}]}");
}
