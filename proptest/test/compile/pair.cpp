#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, pair)
{
    Random rand(1);
    auto gen = Arbi<pair<int8_t, uint8_t>>();
    gen(rand);

    Arbi<bool>().pairWith(+[](const bool& value) {
        if (value)
            return gen::interval(0, 10);
        else
            return gen::interval(10, 20);
    });
}
