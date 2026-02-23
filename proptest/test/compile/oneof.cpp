#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, oneOf)
{
    gen::oneOf<int>(gen::int32(), gen::int32(), gen::int32());
    // Raw values (implicit just)
    gen::oneOf<int>(1339, 42);
    gen::oneOf<int>(1339, gen::interval(0, 10), 42);
    // this should fail:
    // oneOf<int>(weightedGen<int>(gen::int32(), 0.1), gen::float64(), genn::int());
    gen::oneOf<int>(gen::weightedGen<int>(gen::int32(), 0.1), gen::weightedGen(gen::int32(), 0.1),
               gen::weightedGen(gen::int32(), 0.8));
    // Raw values with weight (implicit just)
    gen::oneOf<int>(gen::weightedGen<int>(1339, 0.9), gen::weightedGen<int>(42, 0.1));
    int val = 1339;
    gen::oneOf<int>(gen::weightedGen<int>(val, 0.9), gen::weightedGen<int>(42, 0.1));
}
