#include "proptest/Arbitrary.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/Random.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/util/printing.hpp"

using namespace proptest;

TEST(Arbitrary, bool)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<bool>();
    auto val = arbi(rand).get();
    for(int i = 0; i < 5; i++)
        cout << arbi(rand).get() << endl;
    // TODO: validate
}

TEST(Arbitrary, toGenerator)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<bool>();
    GenFunction<bool> gen = arbi;
    for(int i = 0; i < 5; i++)
        cout << gen(rand).get() << endl;
}

TEST(Arbitrary, int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto val = arbi(rand).get();
    for(int i = 0; i < 5; i++)
        cout << arbi(rand).get() << endl;
    EXPECT_TRUE(val >= std::numeric_limits<int>::min());
    EXPECT_TRUE(val <= std::numeric_limits<int>::max());
}

TEST(Arbitrary, list_int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<list<int>>();
    for(int i = 0; i < 2; i++) {
        auto shr = arbi(rand);
        show(cout, shr.get());
        cout << endl;
    }
    // TODO: validate
}

TEST(Arbitrary, basic)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto val = arbi(rand).get();
    for(int i = 0; i < 10; i++)
        cout << arbi(rand).get() << endl;
    EXPECT_TRUE(val >= std::numeric_limits<int>::min());
    EXPECT_TRUE(val <= std::numeric_limits<int>::max());
}

TEST(Arbitrary, monadic)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto gen = arbi.map<int>([](const int& val) { return val * 2; });
    auto gen2 = gen.filter([](const int& val) { return val > 0; });

    for(int i = 0; i < 10; i++) {
        auto val = gen2(rand).get();
        cout << val << endl;
        EXPECT_TRUE(val % 2 == 0 && val > 0);
    }
}