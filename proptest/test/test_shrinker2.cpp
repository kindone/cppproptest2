#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/bool.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/shrinker/set.hpp"
#include "proptest/shrinker/map.hpp"
#include "proptest/shrinker/string.hpp"
#include "proptest/shrinker/stringlike.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(ListLikeShrinker, ints3)
{
    Shrinkable<vector<ShrinkableBase>> baseShr = make_shrinkable<vector<ShrinkableBase>, initializer_list<ShrinkableBase>>({
        ShrinkableBase(shrinkIntegral<int>(1)),
        ShrinkableBase(shrinkIntegral<int>(2)),
        ShrinkableBase(shrinkIntegral<int>(3))});

    // EXPECT_EQ(serializeShrinkable(baseShr), "");

    Shrinkable<vector<int>> shr0 = shrinkListLike<vector, int>(baseShr, 0, false, false);
    Shrinkable<vector<int>> shr1 = shrinkListLike<vector, int>(baseShr, 0, true, false);
    Shrinkable<vector<int>> shr2 = shrinkListLike<vector, int>(baseShr, 0, false, true);
    Shrinkable<vector<int>> shr3 = shrinkListLike<vector, int>(baseShr, 0, true, true);

    EXPECT_EQ(shr0.getRef(), vector<int>({1,2,3}));
    EXPECT_EQ(shr1.getRef(), vector<int>({1,2,3}));
    EXPECT_EQ(shr2.getRef(), vector<int>({1,2,3}));
    EXPECT_EQ(shr3.getRef(), vector<int>({1,2,3}));

    EXPECT_EQ(serializeShrinkable(shr0), "{value: [ 1, 2, 3 ]}");
    EXPECT_EQ(serializeShrinkable(shr1), "{value: [ 1, 2, 3 ], shrinks: [{value: [ 0, 0, 0 ]}, {value: [ 1, 1, 1 ], shrinks: [{value: [ 0, 1, 1 ]}]}, {value: [ 1, 2, 2 ], shrinks: [{value: [ 0, 0, 2 ]}, {value: [ 1, 1, 2 ], shrinks: [{value: [ 0, 1, 2 ]}]}]}]}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: [ 1, 2, 3 ], shrinks: [{value: [  ]}, {value: [ 1 ]}, {value: [ 1, 2 ], shrinks: [{value: [ 2 ]}]}, {value: [ 3 ]}, {value: [ 1, 3 ]}, {value: [ 2, 3 ]}]}");
    EXPECT_EQ(serializeShrinkable(shr3), "{value: [ 1, 2, 3 ], shrinks: [{value: [  ]}, {value: [ 1 ], shrinks: [{value: [ 0 ]}]}, {value: [ 1, 2 ], shrinks: [{value: [ 2 ], shrinks: [{value: [ 0 ]}, {value: [ 1 ]}]}]}, {value: [ 3 ], shrinks: [{value: [ 0 ]}, {value: [ 1 ]}, {value: [ 2 ]}]}, {value: [ 1, 3 ], shrinks: [{value: [ 0, 0 ]}, {value: [ 1, 1 ], shrinks: [{value: [ 0, 1 ]}]}, {value: [ 1, 2 ], shrinks: [{value: [ 0, 2 ]}]}]}, {value: [ 2, 3 ], shrinks: [{value: [ 0, 0 ]}, {value: [ 1, 1 ]}, {value: [ 2, 2 ], shrinks: [{value: [ 0, 2 ]}, {value: [ 1, 2 ]}]}]}]}");
}

TEST(ListLikeShrinker, ints8)
{
    Shrinkable<vector<ShrinkableBase>> baseShr = make_shrinkable<vector<ShrinkableBase>, initializer_list<ShrinkableBase>>({
        ShrinkableBase(shrinkIntegral<int>(1)),
        ShrinkableBase(shrinkIntegral<int>(2)),
        ShrinkableBase(shrinkIntegral<int>(3)),
        ShrinkableBase(shrinkIntegral<int>(4)),
        ShrinkableBase(shrinkIntegral<int>(5)),
        ShrinkableBase(shrinkIntegral<int>(6)),
        ShrinkableBase(shrinkIntegral<int>(7)),
        ShrinkableBase(shrinkIntegral<int>(8))});

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
    // too long!
    // EXPECT_EQ(serializeShrinkable(shr2), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ], shrinks: [{value: [ 0, 0, 0, 0, 0, 0, 0, 0 ]}, {value: [ 1, 1, 1, 2, 2, 3, 3, 4 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 2 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 1 ]}]}, {value: [ 1, 1, 1, 2, 2, 2, 2, 3 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 2, 2, 3 ]}]}]}, {value: [ 1, 2, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 0, 2, 3, 3, 4, 4, 5 ]}, {value: [ 1, 1, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 1, 2, 3, 3, 4, 4, 5 ]}]}]}, {value: [ 1, 2, 3, 4, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 0, 0, 4, 5, 6, 7 ]}, {value: [ 1, 1, 1, 2, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 1, 1, 4, 5, 6, 7 ]}]}, {value: [ 1, 2, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 2, 3, 4, 5, 6, 7 ]}, {value: [ 1, 1, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 2, 3, 4, 5, 6, 7 ]}]}]}]}]}");
    // EXPECT_EQ(serializeShrinkable(shr3), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ], shrinks: [{value: [ 0, 0, 0, 0, 0, 0, 0, 0 ]}, {value: [ 1, 1, 1, 2, 2, 3, 3, 4 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 2 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 1 ]}]}, {value: [ 1, 1, 1, 2, 2, 2, 2, 3 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 2, 2, 3 ]}]}]}, {value: [ 1, 2, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 0, 2, 3, 3, 4, 4, 5 ]}, {value: [ 1, 1, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 1, 2, 3, 3, 4, 4, 5 ]}]}]}, {value: [ 1, 2, 3, 4, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 0, 0, 4, 5, 6, 7 ]}, {value: [ 1, 1, 1, 2, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 1, 1, 4, 5, 6, 7 ]}]}, {value: [ 1, 2, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 2, 3, 4, 5, 6, 7 ]}, {value: [ 1, 1, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 2, 3, 4, 5, 6, 7 ]}]}]}]}]}");
}

TEST(SetShrinker, ints)
{
    auto baseShr = util::make_shared<set<Shrinkable<int>>,initializer_list<Shrinkable<int>>>({
        shrinkIntegral<int>(1),
        shrinkIntegral<int>(2),
        shrinkIntegral<int>(3)});
    Shrinkable<set<int>> shr = shrinkSet<int>(baseShr, 0);
    EXPECT_EQ(shr.getRef(), set<int>({ 1, 2, 3 }));
    EXPECT_EQ(serializeShrinkable(shr), "{value: { 1, 2, 3 }, shrinks: [{value: {  }}, {value: { 1 }}, {value: { 1, 2 }, shrinks: [{value: { 2 }}]}, {value: { 3 }}, {value: { 1, 3 }}, {value: { 2, 3 }}]}");
}

TEST(MapShrinker, int_ints)
{
    auto keyShrVec = util::make_shared<vector<Shrinkable<int>>,initializer_list<Shrinkable<int>>>({Shrinkable<int>(100), Shrinkable<int>(300), Shrinkable<int>(500)});
    auto valShrVec = util::make_shared<vector<Shrinkable<int>>,initializer_list<Shrinkable<int>>>({shrinkIntegral<int>(1), shrinkIntegral<int>(3), shrinkIntegral<int>(5)});
    Shrinkable<map<int, int>> shr = shrinkMap<int, int>(keyShrVec, valShrVec, 0);
    // EXPECT_EQ(shr.getRef(), map<int, int>({ { 1, 2 }, { 3, 4 }, { 5, 6 } }));
    EXPECT_EQ(serializeShrinkable(shr),
        "{value: { (100, 1), (300, 3), (500, 5) }, shrinks: "
            "[{value: {  }}, "
            "{value: { (100, 1) }, shrinks: "
                "[{value: { (100, 0) }}]}, "
            "{value: { (100, 1), (300, 3) }, shrinks: "
                "[{value: { (300, 3) }, shrinks: "
                    "[{value: { (300, 0) }}, "
                    "{value: { (300, 1) }}, "
                    "{value: { (300, 2) }}]}]}, "
            "{value: { (500, 5) }, shrinks: "
                "[{value: { (500, 0) }}, "
                "{value: { (500, 2) }, shrinks: "
                    "[{value: { (500, 1) }}]}, "
                "{value: { (500, 3) }}, "
                "{value: { (500, 4) }}]}, "
            "{value: { (100, 1), (500, 5) }, shrinks: "
                "[{value: { (100, 0), (500, 0) }}, "
                "{value: { (100, 1), (500, 2) }, shrinks: "
                    "[{value: { (100, 0), (500, 1) }}]}, "
                "{value: { (100, 1), (500, 3) }, shrinks: "
                    "[{value: { (100, 0), (500, 3) }}]}, "
                "{value: { (100, 1), (500, 4) }, shrinks: "
                    "[{value: { (100, 0), (500, 4) }}]}]}, "
            "{value: { (300, 3), (500, 5) }, shrinks: "
                "[{value: { (300, 0), (500, 0) }}, "
                "{value: { (300, 1), (500, 2) }, shrinks: "
                    "[{value: { (300, 1), (500, 1) }}]}, "
                "{value: { (300, 2), (500, 3) }}, "
                "{value: { (300, 3), (500, 4) }, shrinks: "
                    "[{value: { (300, 0), (500, 4) }}, "
                "{value: { (300, 1), (500, 4) }}, "
                "{value: { (300, 2), (500, 4) }}]}]}]}");
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

    auto shr1 = shrinkContainer<vector, uint32_t>(util::make_shared<vector<Shrinkable<uint32_t>>>(fwd_converter("abc")), 0, false, true).map<string>(back_converter);
    EXPECT_EQ(serializeShrinkable(shr1), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"c\" (63)}, {value: \"ac\" (61 63)}, {value: \"bc\" (62 63)}]}");
    auto shr2 = shrinkContainer<vector, uint32_t>(util::make_shared<vector<Shrinkable<uint32_t>>>(fwd_converter("abc")), 0, true, false).map<string>(back_converter);
    EXPECT_EQ(serializeShrinkable(shr2), "{value: \"abc\" (61 62 63)}");
    auto shr3 = shrinkContainer<vector, uint32_t>(util::make_shared<vector<Shrinkable<uint32_t>>>(fwd_converter("abc")), 0, true, true).map<string>(back_converter);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"c\" (63)}, {value: \"ac\" (61 63)}, {value: \"bc\" (62 63)}]}");
}

// same as StringShrinker, but with UTF8String/UTF16String
TEST(StringLikeShrinker, basic)
{
    vector<int> bytePositions({0,1,2,3}); // need 4 for 3 chars
    auto shr = shrinkStringLike<UTF8String>("abc", 0, 3, bytePositions);
    EXPECT_EQ(serializeShrinkable(shr), "{value: \"abc\" (61 62 63), shrinks: [{value: \"\" ()}, {value: \"a\" (61)}, {value: \"ab\" (61 62), shrinks: [{value: \"b\" (62)}]}, {value: \"bc\" (62 63)}, {value: \"c\" (63)}]}");
}
