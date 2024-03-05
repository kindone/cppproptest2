#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/io.hpp"
#include "proptest/test/gtest.hpp"
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
    auto shr2 = shr.with(Stream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    EXPECT_EQ(shr2.get(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), false);
    EXPECT_EQ(shr2.getShrinks().getHeadRef().get(), 200);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");
}

// horizontally append shrinks to the end of each stream
TEST(Shrinkable, concatStatic)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.concatStatic(Stream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    auto shr4 = shr3.concatStatic(Stream<Shrinkable<int>>::one(Shrinkable<int>(3)));
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 3}]}, {value: 1, shrinks: [{value: 3}]}, {value: 3}]}");
}

TEST(Shrinkable, concat)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.concat([](const Shrinkable<int>&) {
        return Stream<Shrinkable<int>>::one(Shrinkable<int>(200));
    });
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    auto shr4 = shr3.concat([](const Shrinkable<int>& shr) {
        return Stream<Shrinkable<int>>::one(Shrinkable<int>(shr.get() + 5));
    });
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 5}]}, {value: 1, shrinks: [{value: 6}]}, {value: 7}]}");
}

// vertically append shrinks to the dead-end of shrinks where no more shrinks are possible
TEST(Shrinkable, andThenStatic)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.andThenStatic(Stream<Shrinkable<int>>::one(Shrinkable<int>(200)));
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    auto shr4 = shr3.andThenStatic(Stream<Shrinkable<int>>::one(Shrinkable<int>(3)));
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 3}]}, {value: 1, shrinks: [{value: 3}]}]}");

}

TEST(Shrinkable, andThen)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    auto shr2 = shr.andThen([](const Shrinkable<int>&) {
        return Stream<Shrinkable<int>>::one(Shrinkable<int>(200));
    });
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    auto shr4 = shr3.andThen([](const Shrinkable<int>& shr) {
        return Stream<Shrinkable<int>>::one(Shrinkable<int>(shr.get() + 5));
    });
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 5}]}, {value: 1, shrinks: [{value: 6}]}]}");
}

TEST(Shrinkable, filter)
{
    Shrinkable<int> shr(100);
    shr = shr.with(Stream<Shrinkable<int>>::one(Shrinkable<int>(200)));
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
    auto shr2 = shr.flatMap<string>([](const int& val) { return make_shrinkable<string>(to_string(val + 1)); });
    EXPECT_EQ("101", shr2.get());
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: \"101\" (31 30 31)}");
}

TEST(Shrinkable, build)
{
    using Shr = Shrinkable<int64_t>;
    using Str = Stream<Shr>;
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
    using Str = Stream<Shr>;
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
    using Str = Stream<Shr>;
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

TEST(Shrinkable, map_exhaustive2)
{
    struct Functor {
        int operator()(const int64_t& val) const { return static_cast<int>(val + start); }

        int start;
    };

    Shrinkable<int64_t> shr = util::binarySearchShrinkable(8);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    auto shr2 = shr.map<int>(Functor{5});
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 13, shrinks: [{value: 5}, {value: 9, shrinks: [{value: 7, shrinks: [{value: 6}]}, {value: 8}]}, {value: 11, shrinks: [{value: 10}]}, {value: 12}]}");
    auto shr3 = shr.map<string>([](const int& val) -> string { return to_string(val); });
    EXPECT_EQ(serializeShrinkable(shr3), "{value: \"8\" (38), shrinks: [{value: \"0\" (30)}, {value: \"4\" (34), shrinks: [{value: \"2\" (32), shrinks: [{value: \"1\" (31)}]}, {value: \"3\" (33)}]}, {value: \"6\" (36), shrinks: [{value: \"5\" (35)}]}, {value: \"7\" (37)}]}");
}

TEST(ShrinkableAny, basic) {
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
}

TEST(ShrinkableAny, with)
{
    ShrinkableAny shr(Shrinkable<int>(100));
    EXPECT_EQ(shr.getAny().getRef<int>(), 100);
    auto shr2 = shr.with(Stream<ShrinkableAny>::one(ShrinkableAny(Shrinkable<int>(200))));
    EXPECT_EQ(shr2.getAny().getRef<int>(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), false);
    EXPECT_EQ(shr2.getShrinks().getHeadRef().getAny().getRef<int>(), 200);
    EXPECT_EQ(serializeShrinkableAny<int>(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkableAny<int>(shr2), "{value: 100, shrinks: [{value: 200}]}");
}

TEST(ShrinkableAny, map)
{
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    auto shr2 = shr.map<int>([](const Any& val) -> int { return static_cast<int>(val.getRef<int64_t>() + 1); });
    EXPECT_EQ(shr2.getAny().getRef<int>(), 9);
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    EXPECT_EQ(serializeShrinkableAny<int>(shr2), "{value: 9, shrinks: [{value: 1}, {value: 5, shrinks: [{value: 3, shrinks: [{value: 2}]}, {value: 4}]}, {value: 7, shrinks: [{value: 6}]}, {value: 8}]}");
}

TEST(ShrinkableAny, flatMap)
{
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    auto shr2 = shr.flatMap<string>([](const Any& val) -> Shrinkable<string> { return make_shrinkable<string>(to_string(val.getRef<int64_t>() + 1)); });
    EXPECT_EQ(shr2.getAny().getRef<string>(), "9");
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    EXPECT_EQ(serializeShrinkableAny<string>(shr2), "{value: \"9\" (39), shrinks: [{value: \"1\" (31)}, {value: \"5\" (35), shrinks: [{value: \"3\" (33), shrinks: [{value: \"2\" (32)}]}, {value: \"4\" (34)}]}, {value: \"7\" (37), shrinks: [{value: \"6\" (36)}]}, {value: \"8\" (38)}]}");
}

TEST(ShrinkableAny, andThenStatic)
{
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    auto shr2 = shr.andThenStatic(Stream<ShrinkableAny>::one(ShrinkableAny(Shrinkable<int64_t>(200))));
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr2), "{value: 8, shrinks: [{value: 0, shrinks: [{value: 200}]}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1, shrinks: [{value: 200}]}]}, {value: 3, shrinks: [{value: 200}]}]}, {value: 6, shrinks: [{value: 5, shrinks: [{value: 200}]}]}, {value: 7, shrinks: [{value: 200}]}]}");
}

TEST(ShrinkableAny, andThen)
{
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    auto shr2 = shr.andThen([](const ShrinkableAny& shr) {
        return Stream<ShrinkableAny>::one(ShrinkableAny(Shrinkable<int64_t>(100 + shr.getAny().getRef<int64_t>())));
    });
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr2), "{value: 8, shrinks: [{value: 0, shrinks: [{value: 100}]}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1, shrinks: [{value: 101}]}]}, {value: 3, shrinks: [{value: 103}]}]}, {value: 6, shrinks: [{value: 5, shrinks: [{value: 105}]}]}, {value: 7, shrinks: [{value: 107}]}]}");
}

TEST(ShrinkableAny, concatStatic)
{
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    auto shr2 = shr.concatStatic(Stream<ShrinkableAny>::one(ShrinkableAny(Shrinkable<int64_t>(200))));
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr2), "{value: 8, shrinks: [{value: 0, shrinks: [{value: 200}]}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1, shrinks: [{value: 200}]}, {value: 200}]}, {value: 3, shrinks: [{value: 200}]}, {value: 200}]}, {value: 6, shrinks: [{value: 5, shrinks: [{value: 200}]}, {value: 200}]}, {value: 7, shrinks: [{value: 200}]}, {value: 200}]}");
}

TEST(ShrinkableAny, concat)
{
    ShrinkableAny shr(util::binarySearchShrinkable(8));
    EXPECT_EQ(shr.getAny().getRef<int64_t>(), 8);
    auto shr2 = shr.concat([](const ShrinkableAny& shr) {
        return Stream<ShrinkableAny>::one(ShrinkableAny(Shrinkable<int64_t>(100 + shr.getAny().getRef<int64_t>())));
    });
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
    EXPECT_EQ(serializeShrinkableAny<int64_t>(shr2), "{value: 8, shrinks: [{value: 0, shrinks: [{value: 100}]}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1, shrinks: [{value: 101}]}, {value: 102}]}, {value: 3, shrinks: [{value: 103}]}, {value: 104}]}, {value: 6, shrinks: [{value: 5, shrinks: [{value: 105}]}, {value: 106}]}, {value: 7, shrinks: [{value: 107}]}, {value: 108}]}");
}
