#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, oneOf)
{
    gen::oneOf<int>(Arbi<int>(), Arbi<int>(), Arbi<int>());
    // this should fail:
    // oneOf<int>(weightedGen<int>(Arbi<int>(), 0.1), Arbi<double>(), Arbi<int>());
    gen::oneOf<int>(gen::weightedGen<int>(Arbi<int>(), 0.1), gen::weightedGen(Arbi<int>(), 0.1),
               gen::weightedGen(Arbi<int>(), 0.8));
}
