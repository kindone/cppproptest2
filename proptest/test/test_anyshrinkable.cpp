#include "proptest/AnyShrinkable.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(AnyShrinkable, basic) {
    AnyShrinkable shr(Shrinkable<int>(100));
    EXPECT_EQ(shr.getAny().getRef<int>(), 100);
    EXPECT_EQ(serializeAnyShrinkable<int>(shr), "{value: 100}");
}

TEST(AnyShrinkable, with) {
    AnyShrinkable shr(Shrinkable<int>(100));
    EXPECT_EQ(shr.getAny().getRef<int>(), 100);
    auto shr2 = shr.with(Stream::one(AnyShrinkable(Shrinkable<int>(200))));
    EXPECT_EQ(shr2.getAny().getRef<int>(), 100);
    EXPECT_EQ(shr2.getShrinks().isEmpty(), false);
    EXPECT_EQ(shr2.getShrinks().getHeadRef<AnyShrinkable>().getAny().getRef<int>(), 200);
    EXPECT_EQ(serializeAnyShrinkable<int>(shr), "{value: 100}");
    EXPECT_EQ(serializeAnyShrinkable<int>(shr2), "{value: 100, shrinks: [{value: 200}]}");
}
