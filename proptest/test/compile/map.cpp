#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, map)
{
    Random rand(3);
    auto gen = gen::map<string, string>();
    gen(rand);
    gen.map([](const map<string, string>&) { return 0; });
    gen.map<int>([](const map<string, string>&) { return 0; });
}

TEST(Compile, map_keygen)
{
    auto mapGen = gen::map<int, int>();
    auto intArbi = gen::int32();
    auto intGen = gen::inRange(0,1);
    auto intGenFunc = +[](Random&) -> Shrinkable<int> {
        return make_shrinkable<int>(0);
    };

    auto pairGen1 = gen::pair(intArbi, intArbi);
    auto pairGen2 = gen::pair(intArbi, intGen);
    auto pairGen3 = gen::pair(intArbi, intGenFunc);
    auto pairGen4 = gen::pair(intGen, intArbi);
    auto pairGen5 = gen::pair(intGen, intGen);
    auto pairGen6 = gen::pair(intGen, intGenFunc);
    auto pairGen7 = gen::pair(intGenFunc, intArbi);
    auto pairGen8 = gen::pair(intGenFunc, intGen);
    auto pairGen9 = gen::pair(intGenFunc, intGenFunc);
    mapGen.setPairGen(pairGen1);
    mapGen.setPairGen(pairGen2);
    mapGen.setPairGen(pairGen3);
    mapGen.setPairGen(pairGen4);
    mapGen.setPairGen(pairGen5);
    mapGen.setPairGen(pairGen6);
    mapGen.setPairGen(pairGen7);
    mapGen.setPairGen(pairGen8);
    mapGen.setPairGen(pairGen9);
}
