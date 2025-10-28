#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, tuple)
{
    Random rand(1);
    auto gen = gen::tuple(gen::int8(), gen::uint8(), gen::float32());
    gen(rand);

    gen::boolean().tupleWith(+[](const bool& value) {
        if (value)
            return gen::interval(0, 10);
        else
            return gen::interval(10, 20);
    });
}
