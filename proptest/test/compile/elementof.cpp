#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

struct Cat
{
    int age;
};

TEST(Compile, elementOf)
{
    gen::elementOf<int>(0, 1, 2);
    gen::elementOf<int>(gen::weightedVal(0, 0.1), 1, 2);
    gen::elementOf<int>(gen::weightedVal(0, 0.1), gen::weightedVal<int>(1, 0.1), gen::weightedVal(2, 0.8));
    gen::elementOf<int>(0, gen::weighted<int>(1, 0.1), gen::weightedVal<int>(2, 0.8));
    // weighted(value, prob) without explicit T
    gen::elementOf<int>(0, gen::weighted(1, 0.1), gen::weighted(2, 0.8));
}
