#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, oneOf)
{
    oneOf<int>(Arbi<int>(), Arbi<int>(), Arbi<int>());
    // this should fail:
    // oneOf<int>(weightedGen<int>(Arbi<int>(), 0.1), Arbi<double>(), Arbi<int>());
    oneOf<int>(weightedGen<int>(Arbi<int>(), 0.1), weightedGen(Arbi<int>(), 0.1),
               weightedGen(Arbi<int>(), 0.8));
}
