#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, tuple)
{
    Random rand(1);
    auto gen = Arbi<tuple<int8_t, uint8_t, float>>();
    gen(rand);

    Arbi<bool>().tupleWith(+[](const bool& value) {
        if (value)
            return gen::interval(0, 10);
        else
            return gen::interval(10, 20);
    });
}
