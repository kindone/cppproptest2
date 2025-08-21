#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, lazy)
{
    Random rand(3);
    auto gen = gen::lazy([]() { return 5; });

    gen(rand);
}
