#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

struct Cat
{
    int age;
};

TEST(Compile, elementOf)
{
    elementOf<int>(0, 1, 2);
    elementOf<int>(gen::weightedVal(0, 0.1), 1, 2);
    elementOf<int>(gen::weightedVal(0, 0.1), gen::weightedVal<int>(1, 0.1), gen::weightedVal(2, 0.8));
}
