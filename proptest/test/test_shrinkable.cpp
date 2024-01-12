#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/std/io.hpp"
#include "gtest/gtest.h"

using namespace proptest;


template <typename T>
void printShrinkable(const proptest::Shrinkable<T>& shrinkable, int level) {
    for (int i = 0; i < level; i++)
        cout << "  ";

    cout << "shrinkable: " << proptest::Show<T>(shrinkable.get()) << endl;
}

template <typename T>
void exhaustive(const proptest::Shrinkable<T>& shrinkable, int level = 0)
{
    printShrinkable(shrinkable, level);

    auto shrinks = shrinkable.getShrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        exhaustive(shrinkable2, level + 1);
    }
}

template <typename T>
void exhaustive(const proptest::Shrinkable<T>& shrinkable, int level, proptest::function<void(const proptest::Shrinkable<T>&, int)> func)
{
    func(shrinkable, level);

    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.template iterator<proptest::Shrinkable<T>>(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        exhaustive(shrinkable2, level + 1, func);
    }
}

template <typename T>
bool compareShrinkable(const Shrinkable<T>& lhs, const Shrinkable<T>& rhs, size_t maxElements = 1000)
{
    if(lhs.get() != rhs.get())
        return false;

    maxElements --;

    auto lhsShrinks = lhs.getShrinks();
    auto rhsShrinks = rhs.getShrinks();

    for(auto litr = lhsShrinks.iterator(), ritr = lhsShrinks.iterator() ; litr.hasNext() || ritr.hasNext();)
    {
        if(litr.hasNext() != ritr.hasNext())
            return false;
        if(!compareShrinkable(litr.next(), ritr.next(), maxElements))
            return false;
        maxElements --;
    }
    return true;
}

template <typename T>
void outShrinkable(ostream& stream, const proptest::Shrinkable<T>& shrinkable) {
    stream << "{value: " << proptest::Show<T>(shrinkable.get());
    auto shrinks = shrinkable.getShrinks();
    if(!shrinks.isEmpty()) {
        stream << ", shrinks: [";
        for (auto itr = shrinks.iterator(); itr.hasNext();) {
            auto shrinkable2 = itr.next();
            outShrinkable(stream, shrinkable2);
            if(itr.hasNext())
                stream << ", ";
        }
        stream << "]";
    }
    stream << "}";
}

template <typename T>
string serializeShrinkable(const Shrinkable<T>& shr)
{
    stringstream stream;
    outShrinkable(stream, shr);
    return stream.str();
}


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
    exhaustive(Shr(8).with(Str::of(
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
    exhaustive(shr);
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
    exhaustive(shr);
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
    // using Shr = Shrinkable<int64_t>;
    // using Str = TypedStream<Shr>;
    for(int i = 0; i < 8; i++) {
        auto shr = util::binarySearchShrinkable(8).filter([](const int& val) { return true; }, i);
        cout << "i: " << i << endl;
        exhaustive(shr);
    }
}

TEST(Shrinkable, map_exhaustive)
{
    auto shr = util::binarySearchShrinkable(8).map<string>([](const int& val) -> string { return to_string(val); });
    exhaustive(shr);
}
