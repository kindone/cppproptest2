#include "proptest/combinator/just.hpp"
#include "proptest/combinator/reference.hpp"
#include "proptest/combinator/filter.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/derive.hpp"
#include "proptest/combinator/dependency.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/elementof.hpp"
#include "proptest/combinator/chain.hpp"
#include "proptest/combinator/intervals.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/std/string.hpp"
#include "proptest/Random.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;


TEST(Just, lvalue)
{
    int a = 1339;
    Random rand(getCurrentTime());
    auto gen = just<int>(a);
    auto result = gen(rand);
    EXPECT_EQ(result.get(), 1339);
}

TEST(Just, rvalue)
{
    Random rand(getCurrentTime());
    auto gen = just<int>(1339);
    auto result = gen(rand);
    EXPECT_EQ(result.get(), 1339);
}

TEST(Just, Any)
{
    Random rand(getCurrentTime());
    auto gen = just<Any>(Any(1339));
    auto result = gen(rand);
    EXPECT_EQ(result.get().getRef<int>(), 1339);
}

TEST(OneOf, basic)
{
    Random rand(getCurrentTime());
    auto gen = oneOf<int>(just<int>(1339), just<int>(42));
    auto result = gen(rand);
    EXPECT_TRUE(result.get() == 1339 || result.get() == 42);
}

TEST(OneOf, weighted)
{
    Random rand(getCurrentTime());
    auto gen = unionOf<int>(weightedGen(just<int>(1339), 0.9), weightedGen(just<int>(42), 0.1));
    int num1339 = 0, num42 = 0;
    for(int i = 0; i < 1000; i++) {
        auto result = gen(rand);
        if(result.get() == 1339)
            num1339++;
        else if(result.get() == 42)
            num42++;
        EXPECT_TRUE(result.get() == 1339 || result.get() == 42);
    }
    EXPECT_GT(num1339, num42);
}

TEST(ElementOf, basic)
{
    Random rand(getCurrentTime());
    auto gen = elementOf<int>(1339, 42);
    auto result = gen(rand);
    EXPECT_TRUE(result.get() == 1339 || result.get() == 42);
}

TEST(ElementOf, weighted)
{
    Random rand(getCurrentTime());
    auto gen = elementOf<int>(weightedVal(1339, 0.9), weightedVal(42, 0.1));
    int num1339 = 0, num42 = 0;
    for(int i = 0; i < 1000; i++) {
        auto result = gen(rand);
        if(result.get() == 1339)
            num1339++;
        else if(result.get() == 42)
            num42++;
        EXPECT_TRUE(result.get() == 1339 || result.get() == 42);
    }
    EXPECT_GT(num1339, num42);
}

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

TEST(Derive, basic)
{
    Random rand(getCurrentTime());
    auto gen = just<int>(8);
    auto derived = derive<int, int>(gen, [](const int& i) { return interval(0, i); });

    for(int i = 0; i < 5; i++)
    {
        // TODO validation
        auto result = derived(rand);
        cout << serializeShrinkable(result) << endl;
        if(result.get() == 8) {
            EXPECT_EQ(serializeShrinkable(result), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
        }
    }
}

TEST(Dependency, basic)
{
    Random rand(getCurrentTime());
    auto gen = just<int>(8);
    auto derived = dependency<int, int>(gen, [](const int& i) { return interval(0, i); });

    for(int i = 0; i < 5; i++)
    {
        // TODO validation
        [[maybe_unused]] auto result = derived(rand);
        cout << serializeShrinkable(result) << endl;
        // if(result.get() == 8)
        //     EXPECT_EQ(serializeShrinkable(result), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    }
}

TEST(Chain, basic)
{
    Random rand(getCurrentTime());
    auto gen = chain(interval<int>(0, 8), [](const int& i) -> Generator<int> { return interval(0, i); });

    for(int i = 0; i < 20; i++)
    {
        // TODO validation
        [[maybe_unused]] auto result = gen(rand);
        // if(get<0>(result.get()) == 8) {
        //     EXPECT_EQ(serializeShrinkable(result), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
        // }
    }
}


TEST(PropTest, TestChain)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto nullableIntegers = Arbi<bool>().tupleWith<int>(+[](const bool& isNull) -> GenFunction<int> {
        if (isNull)
            return just(0);
        else
            return interval<int>(10, 20);
    });

    auto tupleGen = nullableIntegers.tupleWith<int>(+[](const Chain<bool, int>& chain) {
        bool isNull = get<0>(chain);
        int value = get<1>(chain);
        if (isNull)
            return interval(0, value);
        else
            return interval(-10, value);
    });

    for (int i = 0; i < 3; i++) {
        auto tupleShr = tupleGen(rand);
        cout << serializeShrinkable(tupleShr) << endl;
    }
}

TEST(Chain, chainTwice)
{
    Random rand(getCurrentTime());
    auto gen = chain(interval<int>(0, 4), [](const int&) -> Generator<int> { return interval(0, 4); });

    for(int i = 0; i < 5; i++)
    {
        [[maybe_unused]] auto result = gen(rand);
        // print result
        cout << serializeShrinkable(result) << endl;
    }

    auto gen2 = chain(gen, [](const tuple<int, int>&) -> Generator<int> { return interval(0, 4); });

    for(int i = 0; i < 5; i++)
    {
        [[maybe_unused]] auto result = gen2(rand);
        // print result
        cout << serializeShrinkable(result) << endl;
    }
}

TEST(Intervals, basic)
{
    Random rand(getCurrentTime());
    auto gen = intervals({Interval(0, 2), Interval(12, 22)});

    for(int i = 0; i < 20; i++)
    {
        auto result = gen(rand);
        EXPECT_TRUE(result.get() >= 0 && result.get() <= 22);
        EXPECT_TRUE(!(result.get() > 2 && result.get() < 12));
    }
}
