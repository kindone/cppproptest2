#include "proptest/proptest.hpp"

#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, Stream)
{
    auto str1 = Stream::one(1);
    auto str2 = Stream::one<string>("hello");
    auto str3 = Stream::one(vector<int>());
    auto str4 = Stream::one([](int a) { return a + 76; });

    auto sht5 = Stream(map<string, string>());
}
