#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, oneOf)
{
    gen::oneOf<int>(gen::int32(), gen::int32(), gen::int32());
    // Raw values (implicit just)
    gen::oneOf<int>(1339, 42);
    gen::oneOf<int>(1339, gen::interval(0, 10), 42);
    // this should fail to compile:
    // oneOf<int>(weightedGen<int>(gen::int32(), 0.1), gen::float64(), gen::int());
    gen::oneOf<int>(gen::weightedGen<int>(gen::int32(), 0.1), gen::weightedGen(gen::int32(), 0.1),
               gen::weightedGen(gen::int32(), 0.8));
    // Raw values with weight (implicit just)
    gen::oneOf<int>(gen::weightedGen<int>(1339, 0.9), gen::weightedGen<int>(42, 0.1));
    int val = 1339;
    gen::oneOf<int>(gen::weightedGen<int>(val, 0.9), gen::weightedGen<int>(42, 0.1));
    // gen::weighted(value, prob) and gen::weighted(gen, prob) both work in oneOf
    gen::oneOf<int>(gen::weighted(gen::interval(0, 10), 0.5), gen::weightedGen<int>(42, 0.5));
    gen::oneOf<int>(1, gen::weightedGen<int>(2, 0.2), gen::weighted<int>(1339, 0.3), gen::weighted(gen::interval(0, 10), 0.1));
    // weighted with or without explicit T
    gen::oneOf<int>(gen::weighted(1339, 0.9), gen::weighted(42, 0.1));  // no T
    gen::oneOf<int>(gen::weighted<int>(gen::interval(0, 10), 0.5), gen::weighted<int>(42, 0.5));  // explicit T for both
    // all of possible combinations in one oneOf
    gen::oneOf<int>(1, gen::weightedGen<int>(2, 0.2), gen::weighted<int>(1339, 0.3),
        gen::weighted(gen::interval(0, 10), 0.1), 42, gen::weighted<int>(gen::interval(10, 20), 0.5));
}
