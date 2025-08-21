#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, accumulate)
{
    auto gen1 = gen::interval<int>(0, 1);

    [[maybe_unused]] auto gen = gen::accumulate(
        gen1, [](int num) { return gen::interval(num, num + 2); }, 2, 4);
}
