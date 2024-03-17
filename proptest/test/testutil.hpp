#pragma once

#include "proptest/test/gtest.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/AnyShrinkable.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/std/tuple.hpp"
#include "proptest/std/list.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/set.hpp"
#include "proptest/std/map.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/optional.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/utf16string.hpp"

template <typename T>
struct NumericTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct SignedNumericTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct IntegralTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct SignedIntegralTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct UnsignedIntegralTest : public testing::Test
{
    using NumericType = T;
};

template <typename T>
struct StringLikeTest : public testing::Test
{
    using StringType = T;
};

using NumericTypes =
    testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double>;
using SignedNumericTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, float, double>;
using IntegralTypes = testing::Types<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>;
using SignedIntegralTypes = testing::Types<int8_t, int16_t, int32_t, int64_t>;
using UnsignedIntegralTypes = testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
using FloatingTypes = testing::Types<float, double>;
using StringLikeTypes = testing::Types<proptest::UTF8String, proptest::CESU8String, proptest::UTF16BEString, proptest::UTF16LEString>;

TYPED_TEST_SUITE(NumericTest, NumericTypes);
TYPED_TEST_SUITE(SignedNumericTest, SignedNumericTypes);
TYPED_TEST_SUITE(IntegralTest, IntegralTypes);
TYPED_TEST_SUITE(SignedIntegralTest, SignedIntegralTypes);
TYPED_TEST_SUITE(UnsignedIntegralTest, UnsignedIntegralTypes);
TYPED_TEST_SUITE(FloatingTest, FloatingTypes);
TYPED_TEST_SUITE(StringLikeTest, StringLikeTypes);


template <typename T>
void printShrinkable(const proptest::Shrinkable<T>& shrinkable, int level) {
    for (int i = 0; i < level; i++)
        proptest::cout << "  ";

    proptest::cout << "shrinkable: " << proptest::Show<T>(shrinkable.get()) << proptest::endl;
}

template <typename T>
void printExhaustive(const proptest::Shrinkable<T>& shrinkable, int level = 0)
{
    printShrinkable(shrinkable, level);

    auto shrinks = shrinkable.getShrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        printExhaustive(shrinkable2, level + 1);
    }
}

template <typename T>
void printExhaustive(const proptest::Shrinkable<T>& shrinkable, int level, proptest::function<void(const proptest::Shrinkable<T>&, int)> func)
{
    func(shrinkable, level);

    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.template iterator<proptest::Shrinkable<T>>(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        printExhaustive(shrinkable2, level + 1, func);
    }
}

template <typename T>
bool compareShrinkable(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs, size_t maxElements = 1000)
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
void outShrinkable(proptest::ostream& stream, const proptest::Shrinkable<T>& shrinkable) {
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
proptest::string serializeShrinkable(const proptest::Shrinkable<T>& shr)
{
    proptest::stringstream stream;
    outShrinkable(stream, shr);
    return stream.str();
}

template <typename T>
proptest::string serializeShrinkableAny(const proptest::ShrinkableAny& shr)
{
    proptest::stringstream stream;
    outShrinkable<T>(stream, shr.map<T>([](const proptest::Any& any) -> T { return any.getRef<T>(); }));
    return stream.str();
}

template <typename T>
proptest::string serializeAnyShrinkable(const proptest::AnyShrinkable& shr)
{
    proptest::stringstream stream;
    outShrinkable<T>(stream, shr.getShrinkableAny().map<T>([](const proptest::Any& any) -> T { return any.getRef<T>(); }));
    return stream.str();
}

proptest::ostream& operator<<(proptest::ostream& os, const proptest::vector<int>& input);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::vector<int8_t>& input);

template <typename ARG1, typename ARG2>
proptest::ostream& operator<<(proptest::ostream& os, const proptest::tuple<ARG1, ARG2>& tup)
{
    os << proptest::Show<proptest::tuple<ARG1, ARG2>>(tup);
    return os;
}

template <typename ARG1, typename ARG2, typename ARG3>
proptest::ostream& operator<<(proptest::ostream& os, const proptest::tuple<ARG1, ARG2, ARG3>& tup)
{
    os << proptest::Show<proptest::tuple<ARG1, ARG2, ARG3>>(tup);
    return os;
}

template <typename ARG1, typename ARG2>
proptest::ostream& operator<<(proptest::ostream& os, const proptest::pair<ARG1, ARG2>& pr)
{
    os << proptest::Show<proptest::pair<ARG1, ARG2>>(pr);
    return os;
}

struct Animal
{
    Animal(int f, proptest::string n, const proptest::vector<int>& m)
        : numFeet(f), name(n /*, allocator()*/), measures(m /*, allocator()*/)
    {
    }
    int numFeet;
    proptest::string name;
    proptest::vector<int> measures;
};

struct TableData
{
    int num_rows;
    uint16_t num_elements;
    proptest::vector<proptest::pair<uint16_t, bool>> indexes;
};

struct Foo
{
    Foo(int a) : a(a) {}
    int a;
};

proptest::ostream& operator<<(proptest::ostream& os, const proptest::UTF8String&);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::UTF16BEString&);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::UTF16LEString&);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::CESU8String&);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::vector<Foo>& vec);
proptest::ostream& operator<<(proptest::ostream& os, const TableData& td);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::vector<proptest::tuple<uint16_t, bool>>& indexVec);
proptest::ostream& operator<<(proptest::ostream& os,
                         const proptest::pair<proptest::tuple<int, uint16_t>, proptest::vector<proptest::tuple<uint16_t, bool>>>& input);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::set<int>& input);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::map<int, int>& input);
proptest::ostream& operator<<(proptest::ostream& os, const proptest::list<int>& input);

template <typename T>
proptest::ostream& operator<<(proptest::ostream& os, const proptest::shared_ptr<T>& ptr)
{
    if (static_cast<bool>(ptr))
        os << *ptr;
    else
        os << "(null)";
    return os;
}

template <typename T>
proptest::ostream& operator<<(proptest::ostream& os, const proptest::optional<T>& opt)
{
    if(opt)
        os << *opt;
    else
        os << "(empty)";
    return os;
}
