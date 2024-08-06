#include "proptest/generator/integral.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/generator/vector.hpp"
#include "proptest/generator/tuple.hpp"
#include "proptest/combinator/just.hpp"
#include "proptest/Random.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(GenIntegral, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 10);
    auto val = gen(rand).getRef();
    EXPECT_TRUE(val >= std::numeric_limits<int>::min());
    EXPECT_TRUE(val <= std::numeric_limits<int>::max());
}

TEST(Generator, tupleOf)
{
    Random rand(getCurrentTime());
    auto gen = tupleOf(interval<int>(0, 10), interval<int>(0, 10));
    tuple<int, int> val = gen(rand).getRef();
    EXPECT_TRUE(get<0>(val) >= 0);
    EXPECT_TRUE(get<0>(val) <= 10);
    EXPECT_TRUE(get<1>(val) >= 0);
    EXPECT_TRUE(get<1>(val) <= 10);
}

TEST(Generator, performance_vector)
{
    Random rand(getCurrentTime());
    Arbi<vector<int>> arbi;
    for(int i = 0; i < 1000; i++) {
        auto shr = arbi(rand);
        EXPECT_LE(shr.getRef().size(), Arbi<vector<int>>::defaultMaxSize);
        EXPECT_GE(shr.getRef().size(), Arbi<vector<int>>::defaultMinSize);
    }
}

TEST(Generator, performance_int)
{
    Random rand(getCurrentTime());
    auto gen = interval(0, 10);
    for(int i = 0; i < 100000; i++) {
        auto shr = gen(rand);
        EXPECT_GE(shr.getRef(), 0);
        EXPECT_LE(shr.getRef(), 10);
    }
}


TEST(Generator, GenFunction)
{
    Random rand(getCurrentTime());
    Generator<int> gen = just<int>(1339);
    EXPECT_EQ(gen(rand).getRef(), 1339);

    GenFunction<int> genFunc = gen;
    EXPECT_EQ(genFunc(rand).getRef(), 1339);

    Generator<int> gen2 = genFunc;
    EXPECT_EQ(gen2(rand).getRef(), 1339);

    auto lambda = [](Random&) -> Shrinkable<int>
    {
        return make_shrinkable<int>(1339);
    };

    GenFunction<int> genFunc2 = lambda;
    EXPECT_EQ(genFunc2(rand).getRef(), 1339);
    Generator<int> gen3 = genFunc2;
    EXPECT_EQ(gen3(rand).getRef(), 1339);
}

TEST(Generator, gen2gen)
{
    Random rand(getCurrentTime());
    Generator<int> gen = just<int>(1339);
    Function<GenFunction<int>(const int&)> gen2gen = [](const int& i) -> Generator<int> // without this signature, it will fail
    {
        return just<int>(i + 1);
    };

    EXPECT_EQ(gen(rand).getRef(), 1339);
    Generator<int> gen2 = gen2gen(1339);
    EXPECT_EQ(gen2(rand).getRef(), 1340);
}

TEST(Generator, monadic)
{
    Random rand(getCurrentTime());
    Generator<int> gen = interval<int>(0, 10);
    auto gen2 = gen.map<string>([](int n) { // value passing callable is acceptable
        return to_string(n);
    });

    auto gen3 = gen2.filter([](const string& str) { // reference must be const
        return str.size() > 1;
    });

    auto shr = gen3(rand);
    EXPECT_EQ(shr.getRef(), "10");
}

TEST(AnyGenerator, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 10);
    AnyGenerator anyGen(gen);
    auto anyShr = anyGen(rand);
    EXPECT_GE(anyShr.getAny().getRef<int>(), 0);
    EXPECT_LE(anyShr.getAny().getRef<int>(), 10);

    auto shr = anyGen.generate<int>(rand);
    EXPECT_GE(shr.getRef(), 0);
    EXPECT_LE(shr.getRef(), 10);
}

TEST(AnyGenerator, arbitrary)
{
    Random rand(getCurrentTime());
    AnyGenerator anyGen = Arbi<int>();
    [[maybe_unused]] auto anyShr = anyGen(rand);
    [[maybe_unused]] auto shr = anyGen.generate<int>(rand);
    // TODO: test
}

TEST(AnyGenerator, keep_options)
{
    Random rand(getCurrentTime());
    AnyGenerator anyGen = Arbi<list<int>>(1, 2);

    for(int i = 0; i < 5; i++) {
        [[maybe_unused]] auto shr = anyGen.generate<list<int>>(rand);
        show(cout, shr.getRef());
        cout << endl;
        EXPECT_GE(shr.getRef().size(), 1U);
        EXPECT_LE(shr.getRef().size(), 2U);
    }

    AnyGenerator anyGen2 = anyGen;
    for(int i = 0; i < 5; i++) {
        [[maybe_unused]] auto shr = anyGen2.generate<list<int>>(rand);
        EXPECT_GE(shr.getRef().size(), 1U);
        EXPECT_LE(shr.getRef().size(), 2U);
    }

}
