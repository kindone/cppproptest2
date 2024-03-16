#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, filter)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    filter(gen, [](const map<string, string>&) { return true; });
    suchThat<map<string, string>>(gen, [](const map<string, string>&) { return true; });
}
