#include "proptest/Stream.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "gtest/gtest.h"

using namespace proptest;

TEST(Stream, empty)
{
    auto stream = Stream<int>::empty(); // empty stream
    EXPECT_EQ(stream->isEmpty(), true);
}

TEST(Stream, one)
{
    auto stream = Stream<int>::one(100);
    EXPECT_EQ(stream->isEmpty(), false);

    Iterator<int> itr = stream->iterator();
    ASSERT_EQ(itr.hasNext(), true);

    auto value = itr.next();
    EXPECT_EQ(value, 100);
    ASSERT_EQ(itr.hasNext(), false);

    // copy
    Stream<int> streamCopy = *stream;
    EXPECT_EQ(streamCopy.isEmpty(), false);
}

TEST(Stream, two)
{
    auto stream = Stream<int>::two(100, 200);
    EXPECT_EQ(stream->isEmpty(), false);

    Iterator<int> itr = stream->iterator();
    ASSERT_EQ(itr.hasNext(), true);

    int value = itr.next();
    EXPECT_EQ(value, 100);
    ASSERT_EQ(itr.hasNext(), true);

    value = itr.next();
    EXPECT_EQ(value, 200);
    ASSERT_EQ(itr.hasNext(), false);
}

TEST(Stream, iterator)
{
    auto stream = Stream<int>::two(100, 200);
    EXPECT_EQ(stream->isEmpty(), false);

    vector<int> values;
    for(Iterator<int> itr = stream->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
}

TEST(Stream, string)
{
    auto stream = Stream<string>::two("hello", "world");
    EXPECT_EQ(stream->isEmpty(), false);

    vector<string> values;
    for(Iterator<string> itr = stream->iterator(); itr.hasNext(); ) {
        string value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], "hello");
    EXPECT_EQ(values[1], "world");
}

TEST(Stream, transform)
{
    auto stream = Stream<int>::two(100, 200);

    auto stream2 = stream->transform<string>(make_function([](const int& value) { return to_string(value); }));

    vector<string> values;
    for(Iterator<string> itr = stream2->iterator(); itr.hasNext(); ) {
        string value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], "100");
    EXPECT_EQ(values[1], "200");
}

TEST(Stream, filter)
{
    auto stream = Stream<int>::two(100, 200);
    auto stream2 = stream->filter(make_function([](const int& value) { return value > 100; }));

    vector<int> values;
    for(Iterator<int> itr = stream2->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 200);

    values.clear();
    auto stream3 = stream->filter(make_function([](const int& value) { return value < 200; }));

    for(Iterator<int> itr = stream3->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 100);
}

TEST(Stream, concat)
{
    auto stream = Stream<int>::two(100, 200);
    auto stream2 = Stream<int>::two(300, 400);
    auto stream3 = stream->concat(stream2);

    vector<int> values;
    for(Iterator<int> itr = stream3->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
    EXPECT_EQ(values[2], 300);
    EXPECT_EQ(values[3], 400);
}

TEST(Stream, take)
{
    auto stream = Stream<int>::two(100, 200);
    auto stream2 = stream->take(1);

    vector<int> values;
    for(Iterator<int> itr = stream2->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 100);

    auto stream3 = stream->take(3); // taking exceeding size is ok
    values.clear();
    for(Iterator<int> itr = stream3->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
}

// around 30ms with N=12
TEST(Stream, performance)
{
    const int N = 12;
    auto stream = Stream<int>::one(100);
    for(int i = 0; i < N; i++)
        stream = stream->concat(stream);

    vector<int> values;
    for(Iterator<int> itr = stream->iterator(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4096);
    EXPECT_EQ(values[4095], 100);
}