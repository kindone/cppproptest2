#include "proptest/generator/integral.hpp"
#include "proptest/combinator/just.hpp"
#include "proptest/Random.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(GenIntegral, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 10);
    auto val = gen(rand).get();
    EXPECT_TRUE(val >= std::numeric_limits<int>::min());
    EXPECT_TRUE(val <= std::numeric_limits<int>::max());
}

TEST(Generator, GenFunction)
{
    Random rand(getCurrentTime());
    Generator<int> gen = just<int>(1339);
    EXPECT_EQ(gen(rand).get(), 1339);

    GenFunction<int> genFunc = gen;
    EXPECT_EQ(genFunc(rand).get(), 1339);

    Generator<int> gen2 = genFunc;
    EXPECT_EQ(gen2(rand).get(), 1339);

    auto lambda = [](Random& rand) -> Shrinkable<int>
    {
        return make_shrinkable<int>(1339);
    };

    GenFunction<int> genFunc2 = lambda;
    EXPECT_EQ(genFunc2(rand).get(), 1339);
    Generator<int> gen3 = genFunc2;
    EXPECT_EQ(gen3(rand).get(), 1339);
}

TEST(Generator, gen2gen)
{
    Random rand(getCurrentTime());
    Generator<int> gen = just<int>(1339);
    Function<GenFunction<int>(const int&)> gen2gen = [](const int& i) -> Generator<int> // without this signature, it will fail
    {
        return just<int>(i + 1);
    };

    EXPECT_EQ(gen(rand).get(), 1339);
    Generator<int> gen2 = gen2gen(1339);
    EXPECT_EQ(gen2(rand).get(), 1340);
}

TEST(AnyGenerator, basic)
{
    Random rand(getCurrentTime());
    auto gen = interval<int>(0, 10);
    AnyGenerator anyGen = gen;
    auto anyShr = anyGen(rand).get();
    EXPECT_TRUE(anyShr.getRef<int>() >= 0);
    EXPECT_TRUE(anyShr.getRef<int>() <= 10);

    auto shr = anyGen.generate<int>(rand);
    EXPECT_TRUE(shr.getRef() >= 0);
    EXPECT_TRUE(shr.getRef() <= 10);
}