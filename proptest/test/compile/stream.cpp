#include "proptest/proptest.hpp"

#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, Stream)
{
    auto str1 = Stream<int>::one(1);
    auto str2 = Stream<string>::one("hello");
    auto str3 = Stream<vector<int>>::one(vector<int>());
    auto str4 = Stream<Function<int(int)>>::one([](int a) { return a + 76; });

    auto sht5 = Stream<map<string, string>>(map<string, string>());
}
