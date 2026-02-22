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
}
