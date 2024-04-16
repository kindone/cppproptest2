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

    cout << sizeof(shr) << endl;
}

TEST(Shrinkable, with)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<int> shr2 = shr.with(Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(200)));
    EXPECT_EQ(shr2.get(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), false);
    EXPECT_EQ(Shrinkable<int>(shr2.getShrinks().getHeadRef<Shrinkable<int>::StreamElementType>()).get(), 200);
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");
}

// horizontally append shrinks to the end of each stream
TEST(Shrinkable, concatStatic)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<int> shr2 = shr.concatStatic(Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(200)));
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    Shrinkable<int> shr4 = shr3.concatStatic(Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(3)));
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 3}]}, {value: 1, shrinks: [{value: 3}]}, {value: 3}]}");
}

TEST(Shrinkable, concat)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<int> shr2 = shr.concat([](const Shrinkable<int>&) {
        return Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(200));
    });
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    Shrinkable<int> shr4 = shr3.concat([](const Shrinkable<int>& shr) {
        return Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(shr.get() + 5));
    });
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 5}]}, {value: 1, shrinks: [{value: 6}]}, {value: 7}]}");
}

// vertically append shrinks to the dead-end of shrinks where no more shrinks are possible
TEST(Shrinkable, andThenStatic)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<int> shr2 = shr.andThenStatic(Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(200)));
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    Shrinkable<int> shr4 = shr3.andThenStatic(Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(3)));
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 3}]}, {value: 1, shrinks: [{value: 3}]}]}");

}

TEST(Shrinkable, andThen)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<int> shr2 = shr.andThen([](const Shrinkable<int>&) {
        return Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(200));
    });
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100, shrinks: [{value: 200}]}");

    Shrinkable<int> shr3 = shrinkIntegral<int>(2);
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 2, shrinks: [{value: 0}, {value: 1}]}");
    Shrinkable<int> shr4 = shr3.andThen([](const Shrinkable<int>& shr) {
        return Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(shr.get() + 5));
    });
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 2, shrinks: [{value: 0, shrinks: [{value: 5}]}, {value: 1, shrinks: [{value: 6}]}]}");
}

TEST(Shrinkable, filter)
{
    Shrinkable<int> shr(100);
    shr = shr.with(Shrinkable<int>::StreamType::one<Shrinkable<int>::StreamElementType>(Shrinkable<int>(200)));
    Shrinkable<int> shr2 = shr.filter([](const int& val) { return val == 100; });
    Shrinkable<int> shr3 = shr.filter([](const int& val) { return val <= 200; });
    Shrinkable<int> shr4 = shr.filter([](int val) { return val <= 100; });
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100, shrinks: [{value: 200}]}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr3), "{value: 100, shrinks: [{value: 200}]}");
    EXPECT_EQ(serializeShrinkable(shr4), "{value: 100}");
}

TEST(Shrinkable, map)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<int> shr2 = shr.map<int>([](const int& val) { return val + 1; });
    EXPECT_EQ(101, shr2.get());
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 101}");
}

TEST(Shrinkable, flatMap)
{
    Shrinkable<int> shr(100);
    EXPECT_EQ(shr.get(), 100);
    Shrinkable<string> shr2 = shr.flatMap<string>([](const int& val) { return make_shrinkable<string>(to_string(val + 1)); });
    EXPECT_EQ("101", shr2.get());
    EXPECT_EQ(serializeShrinkable(shr), "{value: 100}");
    EXPECT_EQ(serializeShrinkable(shr2), "{value: \"101\" (31 30 31)}");
}

TEST(Shrinkable, build)
{
    using Shr = Shrinkable<int64_t>;
    using Str = Shr::StreamType;
    using Elem = Shr::StreamElementType;
    printExhaustive(Shr(8).with(Str::of<Elem>(
        Shr(0),
        Shr(4).with(Str::of<Elem>(
            Shr(2).with(Str::of<Elem>(
                Shr(1)
            ))
        )),
        Shr(6).with(Str::of<Elem>(
            Shr(5)
        )),
        Shr(7)
    )));
}

TEST(Shrinker, integral)
{
    using Shr = Shrinkable<int64_t>;
    using Str = Shr::StreamType;
    using Elem = Shr::StreamElementType;
    auto shr= util::binarySearchShrinkable(8);
    printExhaustive(shr);
    EXPECT_TRUE(compareShrinkable(shr, Shr(8).with(Str::of<Elem>(
        Shr(0),
        Shr(4).with(Str::of<Elem>(
            Shr(2).with(Str::of<Elem>(
                Shr(1)
            ))
        )),
        Shr(6).with(Str::of<Elem>(
            Shr(5)
        )),
        Shr(7)
    ))));

    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: [{value: 0}, {value: 4, shrinks: [{value: 2, shrinks: [{value: 1}]}, {value: 3}]}, {value: 6, shrinks: [{value: 5}]}, {value: 7}]}");
}

TEST(Shrinkable, filter_exhaustive)
{
    using Shr = Shrinkable<int64_t>;
    using Str = Shr::StreamType;
    using Elem = Shr::StreamElementType;
    Shr shr = util::binarySearchShrinkable(8).filter([](const int64_t& val) { return val > 4; });
    printExhaustive(shr);
    EXPECT_TRUE(compareShrinkable(shr, Shr(8).with(Str::of<Elem>(
        Shr(6).with(Str::of<Elem>(
            Shr(5)
        )),
        Shr(7)
    ))));
    EXPECT_EQ(serializeShrinkable(shr), "{value: 8, shrinks: [{value: 6, shrinks: [{value: 5}]}, {value: 7}]}");

    Shr shr2 = util::binarySearchShrinkable(8).filter([](const int64_t& val) { return val > 5; });
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
    Shrinkable<int> shr2 = shr.map<int>([](const int64_t& val) -> int { return static_cast<int>(val + 1); });
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 9, shrinks: [{value: 1}, {value: 5, shrinks: [{value: 3, shrinks: [{value: 2}]}, {value: 4}]}, {value: 7, shrinks: [{value: 6}]}, {value: 8}]}");
    Shrinkable<string> shr3 = shr.map<string>([](const int64_t& val) -> string { return to_string(val); });
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
    Shrinkable<int> shr2 = shr.map<int>(Functor{5});
    EXPECT_EQ(serializeShrinkable(shr2), "{value: 13, shrinks: [{value: 5}, {value: 9, shrinks: [{value: 7, shrinks: [{value: 6}]}, {value: 8}]}, {value: 11, shrinks: [{value: 10}]}, {value: 12}]}");
    Shrinkable<string> shr3 = shr.map<string>([](const int64_t& val) -> string { return to_string(val); });
    EXPECT_EQ(serializeShrinkable(shr3), "{value: \"8\" (38), shrinks: [{value: \"0\" (30)}, {value: \"4\" (34), shrinks: [{value: \"2\" (32), shrinks: [{value: \"1\" (31)}]}, {value: \"3\" (33)}]}, {value: \"6\" (36), shrinks: [{value: \"5\" (35)}]}, {value: \"7\" (37)}]}");
}
