#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, accumulate)
{
    auto gen1 = interval<int>(0, 1);

    [[maybe_unused]] auto gen = accumulate(
        gen1, [](const int& num) { return interval(num, num + 2); }, 2, 4);
}
