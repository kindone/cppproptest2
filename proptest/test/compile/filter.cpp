#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, filter)
{
    Random rand(3);
    auto gen = Arbi<map<string, string>>();
    gen(rand);
    gen::filter<map<string,string>>(gen, [](const map<string, string>&) { return true; });
    gen::suchThat<map<string, string>>(gen, [](const map<string, string>&) { return true; });
}
