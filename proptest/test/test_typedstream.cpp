#include "proptest/TypedStream.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "gtest/gtest.h"

using namespace proptest;

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
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
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
}

TEST(TypedStream, transform)
{
    auto stream = TypedStream<int>::two(100, 200);

    auto stream2 = stream.transform<string>(make_function([](const int& value) { return to_string(value); }));

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
    auto stream2 = stream.filter(make_function([](const int& value) { return value > 100; }));

    vector<int> values;
    for(TypedIterator<int> itr = stream2.iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 200);

    values.clear();
    auto stream3 = stream.filter(make_function([](const int& value) { return value < 200; }));

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