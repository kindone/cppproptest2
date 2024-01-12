#include "proptest/TypedStream.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/string.hpp"
#include "proptest/util/printing.hpp"
#include "gtest/gtest.h"

using namespace proptest;

template <typename T>
void outStream(ostream& ostr, const TypedStream<T>& stream) {
    ostr << "[";
    for (auto itr = stream.iterator(); itr.hasNext();) {
        stream << proptest::Show<T>(itr.next());
        if(itr.hasNext())
            ostr << ", ";
    }
    ostr << "]";
}

template <typename T>
void outUntypedStream(ostream& ostr, const UntypedStream& stream) {
    ostr << "[";
    for (auto itr = UntypedIterator(stream); itr.hasNext();) {
        ostr << proptest::Show<T>(itr.next<T>());
        if(itr.hasNext())
            ostr << ", ";
    }
    ostr << "]";
}

template <typename T>
string serializeStream(const TypedStream<T>& stream)
{
    stringstream ostr;
    outStream(ostr, stream);
    return ostr.str();
}

template <typename T>
string serializeUntypedStream(const UntypedStream& stream)
{
    stringstream ostr;
    outUntypedStream<T>(ostr, stream);
    return ostr.str();
}

TEST(TypedStream, empty)
{
    auto stream = TypedStream<int>::empty(); // empty stream
    EXPECT_EQ(stream.isEmpty(), true);
}

TEST(TypedStream, one)
{
    auto stream = TypedStream<int>::one(100);
    EXPECT_EQ(stream.isEmpty(), false);

    TypedIterator<int> itr = stream.iterator();
    ASSERT_EQ(itr.hasNext(), true);

    auto value = itr.next();
    EXPECT_EQ(value, 100);
    ASSERT_EQ(itr.hasNext(), false);

    // copy
    TypedStream<int> streamCopy = stream;
    EXPECT_EQ(streamCopy.isEmpty(), false);
}

TEST(TypedStream, two)
{
    auto stream = TypedStream<int>::two(100, 200);
    EXPECT_EQ(stream.isEmpty(), false);

    TypedIterator<int> itr = stream.iterator();
    ASSERT_EQ(itr.hasNext(), true);

    int value = itr.next();
    EXPECT_EQ(value, 100);
    ASSERT_EQ(itr.hasNext(), true);

    value = itr.next();
    EXPECT_EQ(value, 200);
    ASSERT_EQ(itr.hasNext(), false);
}

TEST(TypedStream, values)
{
    auto stream = TypedStream<int>::of(1,2,3,4,5,6,7,8);
    vector<int> values;
    for(TypedIterator<int> itr = stream.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 8);
    EXPECT_EQ(values, vector<int>({1,2,3,4,5,6,7,8}));
}

TEST(TypedStream, values2)
{
    auto stream = TypedStream<int>::values({1,2,3,4,5,6,7,8});
    vector<int> values;
    for(TypedIterator<int> itr = stream.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 8);
    EXPECT_EQ(values, vector<int>({1,2,3,4,5,6,7,8}));
}

TEST(TypedStream, iterator)
{
    auto stream = TypedStream<int>::two(100, 200);
    EXPECT_EQ(stream.isEmpty(), false);

    vector<int> values;
    for(TypedIterator<int> itr = stream.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values, vector<int>({100, 200}));
}

TEST(TypedStream, string)
{
    auto stream = TypedStream<string>::two("hello", "world");
    EXPECT_EQ(stream.isEmpty(), false);

    vector<string> values;
    for(TypedIterator<string> itr = stream.iterator(); itr.hasNext(); ) {
        string value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], "hello");
    EXPECT_EQ(values[1], "world");
    EXPECT_EQ(values, vector<string>({"hello", "world"}));
}

TEST(TypedStream, transform)
{
    auto stream = TypedStream<int>::two(100, 200);

    auto stream2 = stream.transform<string>(util::make_function([](const int& value) { return to_string(value); }));

    vector<string> values;
    for(TypedIterator<string> itr = stream2.iterator(); itr.hasNext(); ) {
        string value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], "100");
    EXPECT_EQ(values[1], "200");
}

TEST(TypedStream, filter)
{
    auto stream = TypedStream<int>::two(100, 200);
    auto stream2 = stream.filter(util::make_function([](const int& value) { return value > 100; }));

    vector<int> values;
    for(TypedIterator<int> itr = stream2.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 200);

    values.clear();
    auto stream3 = stream.filter(util::make_function([](const int& value) { return value < 200; }));

    for(TypedIterator<int> itr = stream3.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 100);
}

TEST(TypedStream, concat)
{
    auto stream = TypedStream<int>::two(100, 200);
    auto stream2 = TypedStream<int>::two(300, 400);
    auto stream3 = stream.concat(stream2);

    vector<int> values;
    for(TypedIterator<int> itr = stream3.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
    EXPECT_EQ(values[2], 300);
    EXPECT_EQ(values[3], 400);
}

TEST(TypedStream, take)
{
    auto stream = TypedStream<int>::two(100, 200);
    auto stream2 = stream.take(1);

    vector<int> values;
    for(TypedIterator<int> itr = stream2.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 100);

    auto stream3 = stream.take(3); // taking exceeding size is ok
    values.clear();
    for(TypedIterator<int> itr = stream3.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
}

// around 30ms with N=12
TEST(TypedStream, performance)
{
    const int N = 12;
    auto stream = TypedStream<int>::one(100);
    for(int i = 0; i < N; i++)
        stream = stream.concat(stream);

    vector<int> values;
    for(TypedIterator<int> itr = stream.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4096);
    EXPECT_EQ(values[4095], 100);
}

TEST(UntypedStream, basic)
{
    UntypedStream untypedStream(Any(100), util::make_anyfunction([]() -> TypedStream<int> { return TypedStream<int>::empty(); }));
}

TEST(UntypedStream, from_TypedStream)
{
    auto stream = TypedStream<int>::empty(); // empty stream
    EXPECT_EQ(stream.isEmpty(), true);
    UntypedStream untypedStream = stream;
    EXPECT_EQ(untypedStream.isEmpty(), true);
}

TEST(UntypedStream, iterator)
{
    UntypedStream untypedStream = TypedStream<int>::two(100, 200);
    EXPECT_EQ(untypedStream.isEmpty(), false);

    vector<int> values;
    for(UntypedIterator itr(untypedStream); itr.hasNext(); ) {
        int value = itr.next<int>();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
}

TEST(UntypedStream, transform)
{
    UntypedStream stream = TypedStream<int>::of(1,2,3,4,5,6,7,8);
    auto stream2 = stream.transform(util::make_function([](const Any& value) -> Any { return to_string(value.getRef<int>()); }));
    EXPECT_EQ(serializeUntypedStream<int>(stream), "[1, 2, 3, 4, 5, 6, 7, 8]");
    EXPECT_EQ(serializeUntypedStream<string>(stream2), "[\"1\" (31), \"2\" (32), \"3\" (33), \"4\" (34), \"5\" (35), \"6\" (36), \"7\" (37), \"8\" (38)]");
}

TEST(UntypedStream, filter)
{
    UntypedStream stream = TypedStream<int>::of(1,2,3,4,5,6,7,8);
    auto stream2 = stream.filter(util::make_function([](const Any& value) -> bool { return value.getRef<int>() > 3; }));
    EXPECT_EQ(serializeUntypedStream<int>(stream2), "[4, 5, 6, 7, 8]");

    auto stream3 = stream.filter(util::make_function([](const Any& value) -> bool { return value.getRef<int>() < 4; }));
    EXPECT_EQ(serializeUntypedStream<int>(stream3), "[1, 2, 3]");
}

TEST(UntypedStream, concat)
{
    UntypedStream untypedStream = TypedStream<int>::two(100, 200);
    auto stream2 = TypedStream<int>::two(300, 400);
    auto stream3 = untypedStream.concat(stream2);
    EXPECT_EQ(serializeUntypedStream<int>(stream3), "[100, 200, 300, 400]");
}

TEST(UntypedStream, take)
{
    UntypedStream untypedStream = TypedStream<int>::of(1,2,3,4,5,6,7,8);
    auto stream2 = untypedStream.take(1);
    EXPECT_EQ(serializeUntypedStream<int>(stream2), "[1]");

    auto stream3 = untypedStream.take(3);
    EXPECT_EQ(serializeUntypedStream<int>(stream3), "[1, 2, 3]");

    auto stream4 = untypedStream.take(10); // taking exceeding size is ok
    EXPECT_EQ(serializeUntypedStream<int>(stream4), "[1, 2, 3, 4, 5, 6, 7, 8]");
}

TEST(UntypedStream, performance)
{
    const int N = 12;
    auto stream = TypedStream<int>::one(100);
    for(int i = 0; i < N; i++)
        stream = stream.concat(stream);

    vector<int> values;
    for(UntypedIterator itr(stream); itr.hasNext(); ) {
        int value = itr.next<int>();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4096);
    EXPECT_EQ(values[4095], 100);
}