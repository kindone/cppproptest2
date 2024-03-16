#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, derive)
{
    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = derive<int, string>(intGen, [](const int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });
}
