#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/std/string.hpp"
#include "proptest/Random.hpp"
#include "proptest/gtest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(Filter, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 100);
    auto filtered = filter(gen, [](int i) { return i % 2 == 0; });

    for(int i = 0; i < 100; i++)
    {
        auto result = filtered(rand);
        EXPECT_TRUE(result.get() % 2 == 0);
    }
}

TEST(Transform, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 100);
    auto transformed = transform<int, int>(gen, [](const int& i) { return i * 2; });

    for(int i = 0; i < 100; i++)
    {
        auto result = transformed(rand);
        EXPECT_TRUE(result.get() % 2 == 0);
    }
}

TEST(Transform, basic2)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 10);
    auto transformed = transform<int, string>(gen, [](const int& i) { return to_string(i); });

    for(int i = 0; i < 100; i++)
    {
        auto result = transformed(rand);
        if(result.get() == "8") {
            EXPECT_EQ(serializeShrinkable(result), "{value: \"8\" (38), shrinks: [{value: \"0\" (30)}, {value: \"4\" (34), shrinks: [{value: \"2\" (32), shrinks: [{value: \"1\" (31)}]}, {value: \"3\" (33)}]}, {value: \"6\" (36), shrinks: [{value: \"5\" (35)}]}, {value: \"7\" (37)}]}");
        }
    }
}

TEST(FilterTransform, basic)
{
    Random rand(getCurrentTime());
    auto gen = filter(interval<int>(0, 8), [](int i) { return i == 8; });
    auto transformed = transform<int, string>(gen, [](const int& i) { return to_string(i); });

    for(int i = 0; i < 10; i++)
    {
        auto result = transformed(rand);
        EXPECT_EQ(result.get(), "8");
        EXPECT_EQ(serializeShrinkable(result), "{value: \"8\" (38)}"); // shrinks are also filtered
    }
}
