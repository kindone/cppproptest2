#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/io.hpp"
#include "gtest/gtest.h"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(Shrinkable, basic)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    EXPECT_EQ(shr.getAny().getRef<int>(), 100);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
}

TEST(Shrinkable, with)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.with(TypedStream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    EXPECT_EQ(shr2.get(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), false);
    EXPECT_EQ(shr2.getShrinks().getHeadRef().get(), 200);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");
}

TEST(Shrinkable, filter)
{
    Shrinkable<int> shr(100);
    shr = shr.with(TypedStream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    auto shr2 = shr.filter([](const int& val) { return val == 100; });
    auto shr3 = shr.filter([](const int& val) { return val <= 200; });
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100, shrinks: [{value: 200}]}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 100, shrinks: [{value: 200}]}");
}

TEST(Shrinkable, map)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.map<int>([](const int& val) { return val + 1; });
    EXPECT_EQ(101, shr2.get());
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 101}");
}

TEST(Shrinkable, flatMap)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.flatMap<int>([](const int& val) { return make_shrinkable<int>(val + 1); });
    EXPECT_EQ(101, shr2.get());
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 101}");
}

TEST(Shrinkable, build)
{
    using Shr = Shrinkable<int64_t>;
    using Str = TypedStream<Shr>;
    printExhaustive(Shr(8).with(Str::of(
        Shr(0),
        Shr(4).with(Str::of(
            Shr(2).with(Str::of(
                Shr(1)
            ))
        )),
        Shr(6).with(Str::of(
            Shr(5)
        )),
        Shr(7)
    )));
}

TEST(Shrinker, integral)
{
    using Shr = Shrinkable<int64_t>;
    using Str = TypedStream<Shr>;
    auto shr= util::binarySearchShrinkable(8);
    printExhaustive(shr);
    EXPECT_TRUE(compareShrinkable(shr, Shr(8).with(Str::of(
        Shr(0),
        Shr(4).with(Str::of(
            Shr(2).with(Str::of(
                Shr(1)
            ))
        )),
        Shr(6).with(Str::of(
            Shr(5)
        )),
        Shr(7)
    ))));

    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
}

TEST(Shrinkable, filter_exhaustive)
{
    using Shr = Shrinkable<int64_t>;
    using Str = TypedStream<Shr>;
    auto shr = util::binarySearchShrinkable(8).filter([](const int& val) { return val > 4; });
    printExhaustive(shr);
    EXPECT_TRUE(compareShrinkable(shr, Shr(8).with(Str::of(
        Shr(6).with(Str::of(
            Shr(5)
        )),
        Shr(7)
    ))));
    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: [{value: 6, shrinks: [{value: 5}]}, {value: 7}]}");

    auto shr2 = util::binarySearchShrinkable(8).filter([](const int& val) { return val > 5; });
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 8, shrinks: [{value: 6}, {value: 7}]}");
}

TEST(Shrinkable, filter_tolerance)
{

    // for(int i = 0; i < 8; i++) {
    //     try {
    //         auto shr = util::binarySearchShrinkable(256).filter([](const int& val) { return val % 128 == 0; });
    //         cout << "i: " << i << endl;
    //         exhaustive(shr);
    //     }
    //     catch(exception& e) {
    //         cout << "exception: " << e.what() << endl;
    //     }
    // }
}

TEST(Shrinkable, map_exhaustive)
{
    Shrinkable<int64_t> shr = util::binarySearchShrinkable(8);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    auto shr2 = shr.map<int>([](const int64_t& val) -> int { return static_cast<int>(val + 1); });
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 9, shrinks: [{value: 1}, {value: 5, shrinks: [{value: 3, shrinks: [{value: 2}]}, {value: 4}]}, {value: 7, shrinks: [{value: 6}]}, {value: 8}]}");
    auto shr3 = shr.map<string>([](const int& val) -> string { return to_string(val); });
    EXPECT_EQ(serializeShrinkable(shr3), "{value: \"8\" (38), shrinks: [{value: \"0\" (30)}, {value: \"4\" (34), shrinks: [{value: \"2\" (32), shrinks: [{value: \"1\" (31)}]}, {value: \"3\" (33)}]}, {value: \"6\" (36), shrinks: [{value: \"5\" (35)}]}, {value: \"7\" (37)}]}");

}
