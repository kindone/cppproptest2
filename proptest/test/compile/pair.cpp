#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, pair)
{
    Random rand(1);
    // auto gen = gen::pair<int8_t, uint8_t>(); FIXME (fall back to arbitrary)
    auto gen = gen::pair(gen::int8(), gen::uint8());
    gen(rand);

    gen::boolean().pairWith(+[](const bool& value) {
        if (value)
            return gen::interval(0, 10);
        else
            return gen::interval(10, 20);
    });
}
