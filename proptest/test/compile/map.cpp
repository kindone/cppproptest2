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

    auto pairGen1 = gen::pairOf(intArbi, intArbi);
    auto pairGen2 = gen::pairOf(intArbi, intGen);
    auto pairGen3 = gen::pairOf(intArbi, intGenFunc);
    auto pairGen4 = gen::pairOf(intGen, intArbi);
    auto pairGen5 = gen::pairOf(intGen, intGen);
    auto pairGen6 = gen::pairOf(intGen, intGenFunc);
    auto pairGen7 = gen::pairOf(intGenFunc, intArbi);
    auto pairGen8 = gen::pairOf(intGenFunc, intGen);
    auto pairGen9 = gen::pairOf(intGenFunc, intGenFunc);
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
