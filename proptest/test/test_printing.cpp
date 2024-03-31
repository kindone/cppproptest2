#include "proptest/util/printing.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Printing, int)
{
    stringstream os;
    os << Show<int>(5);
    EXPECT_EQ(os.str(), "5");
}

TEST(Printing, string)
{
    stringstream os;
    os << Show<string>("hello");
    EXPECT_EQ(os.str(), "\"hello\" (68 65 6c 6c 6f)");
}

TEST(Printing, utf8)
{
    stringstream os;
    os << Show<UTF8String>(UTF8String("hello"));
    EXPECT_EQ(os.str(), "\"hello\" (68 65 6c 6c 6f)");
}

TEST(Printing, vector)
{
    stringstream os;
    os << Show<vector<int>>(vector<int>{1,2,3});
    EXPECT_EQ(os.str(), "[ 1, 2, 3 ]");
}

TEST(Printing, Shrinkable)
{
    stringstream os;
    os << Show(Shrinkable<int>(5));
    EXPECT_EQ(os.str(), "5");
}

TEST(Printing, ShrinkableAny)
{
    stringstream os;
    os << Show<ShrinkableAny, int>(ShrinkableAny(Shrinkable<int>(5)));
    EXPECT_EQ(os.str(), "5");
}
