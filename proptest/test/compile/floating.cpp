#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, floating)
{
    Random rand(1);
    auto gen = Arbi<float>();
    gen(rand);
}
