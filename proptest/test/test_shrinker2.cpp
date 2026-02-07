#include "proptest/shrinker/shrinkers.hpp"
#include "proptest/proptest.hpp"
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
    // too long to compare results, just check the count
    size_t count = 0;
    serializeShrinkable(shr2, count);
    EXPECT_EQ(count, pow(2, 8));
    // EXPECT_EQ(serializeShrinkable(shr3), "{value: [ 1, 2, 3, 4, 5, 6, 7, 8 ], shrinks: [{value: [ 0, 0, 0, 0, 0, 0, 0, 0 ]}, {value: [ 1, 1, 1, 2, 2, 3, 3, 4 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 2 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 1, 1, 1 ]}]}, {value: [ 1, 1, 1, 2, 2, 2, 2, 3 ], shrinks: [{value: [ 0, 1, 1, 1, 1, 2, 2, 3 ]}]}]}, {value: [ 1, 2, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 0, 2, 3, 3, 4, 4, 5 ]}, {value: [ 1, 1, 2, 3, 3, 4, 5, 6 ], shrinks: [{value: [ 0, 1, 2, 3, 3, 4, 4, 5 ]}]}]}, {value: [ 1, 2, 3, 4, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 0, 0, 4, 5, 6, 7 ]}, {value: [ 1, 1, 1, 2, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 1, 1, 4, 5, 6, 7 ]}]}, {value: [ 1, 2, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 0, 2, 3, 4, 5, 6, 7 ]}, {value: [ 1, 1, 2, 3, 4, 5, 6, 7 ], shrinks: [{value: [ 0, 1, 2, 3, 4, 5, 6, 7 ]}]}]}]}]}");
}


template <typename T>
void checkShrinkableHelper(const proptest::Shrinkable<T>& shrinkable, set<string>& resultSet) {
    stringstream ss;
    ss << proptest::Show<T>(shrinkable.getRef());
    auto insertResult = resultSet.insert(ss.str());
    // element must be unique
    if(!insertResult.second)
        throw runtime_error(__FILE__, __LINE__, "Duplicate shrinkable found: " + ss.str());
    auto shrinks = shrinkable.getShrinks();
    if(!shrinks.isEmpty()) {
        for (auto itr = shrinks.template iterator<typename proptest::Shrinkable<T>::StreamElementType>(); itr.hasNext();) {
            proptest::Shrinkable<T> shrinkable2 = itr.next();
            checkShrinkableHelper<T>(shrinkable2, resultSet);
        }
    }
}

// checks whether shrinks are unique and returns number of shrinks by full traversal
template <typename T>
size_t checkShrinkable(const proptest::Shrinkable<T>& shr)
{
    set<string> resultSet;
    checkShrinkableHelper<T>(shr, resultSet);
    return resultSet.size();
}

TEST(ListLikeShrinker, intsN)
{
    matrix([](int N) {
        Shrinkable<vector<ShrinkableBase>> baseShr = make_shrinkable<vector<ShrinkableBase>>();
        vector<ShrinkableBase>& vec = baseShr.getMutableRef();
        for(int i = 0; i < N; i++)
            vec.push_back(ShrinkableBase(shrinkIntegral<int>(i+1)));

        Shrinkable<vector<int>> shr = shrinkListLike<vector, int>(baseShr, 0, false, true);
        EXPECT_EQ(checkShrinkable(shr), pow(2, N));
    }, {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18});
}

// Test for duplicates when minSize > 0
// This test checks if cppproptest also has the duplicate issue with minSize > 0
TEST(ListLikeShrinker, intsN_with_minSize)
{
    matrix([](int N, int minSize) {
        if(minSize > N) return; // Skip invalid cases
        Shrinkable<vector<ShrinkableBase>> baseShr = make_shrinkable<vector<ShrinkableBase>>();
        vector<ShrinkableBase>& vec = baseShr.getMutableRef();
        for(int i = 0; i < N; i++)
            vec.push_back(ShrinkableBase(shrinkIntegral<int>(i+1)));

        Shrinkable<vector<int>> shr = shrinkListLike<vector, int>(baseShr, minSize, false, true);
        // Check for duplicates - this will throw if duplicates are found
        size_t uniqueCount = checkShrinkable(shr);
        // With minSize > 0, we expect fewer than 2^N unique values
        // (since we can't create all subsets due to minSize constraint)
        EXPECT_GT(uniqueCount, 0);
        EXPECT_LE(uniqueCount, pow(2, N));
    }, {4,5,6}, {2,3});
}

// Detailed test to see the structure for a specific case with unique values: [4,1,2,3] with minSize=2
// This tests whether the shrinking algorithm itself creates duplicates (not due to duplicates in root)
TEST(ListLikeShrinker, specific_case_minSize2_unique_values)
{
    Shrinkable<vector<ShrinkableBase>> baseShr = make_shrinkable<vector<ShrinkableBase>>();
    vector<ShrinkableBase>& vec = baseShr.getMutableRef();
    vec.push_back(ShrinkableBase(shrinkIntegral<int>(4)));
    vec.push_back(ShrinkableBase(shrinkIntegral<int>(1)));
    vec.push_back(ShrinkableBase(shrinkIntegral<int>(2)));
    vec.push_back(ShrinkableBase(shrinkIntegral<int>(3)));

    Shrinkable<vector<int>> shr = shrinkListLike<vector, int>(baseShr, 2, false, true);
    // This should not throw (no duplicates created by shrinking algorithm)
    size_t uniqueCount = checkShrinkable(shr);
    EXPECT_GT(uniqueCount, 0);
    // Serialize to see the structure
    string serialized = serializeShrinkable(shr);
    // Just verify it serializes without error
    EXPECT_FALSE(serialized.empty());
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
