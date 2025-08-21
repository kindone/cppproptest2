#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, flatmap)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    gen.flatMap([](const map<string, string>&) { return gen::interval(0, 10); });
    gen.flatMap<int>([](const map<string, string>&) { return gen::interval(0, 10); });
}
