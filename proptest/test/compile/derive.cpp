#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, derive)
{
    auto intGen = gen::elementOf<int>(2, 4, 6);
    auto stringGen = gen::derive<int,int>(intGen, [](const int& value) {
        auto gen = gen::string();
        gen.setMaxSize(value);
        return gen;
    });
}
