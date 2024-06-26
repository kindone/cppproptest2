#include "proptest/Stream.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/string.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

template <typename T>
void outStream(ostream& ostr, const Stream& stream) {
    ostr << "[";
    for (auto itr = stream.template iterator<T>(); itr.hasNext();) {
        stream << proptest::Show<T>(itr.next());
        if(itr.hasNext())
            ostr << ", ";
    }
    ostr << "]";
}

/*
template <typename T>
void outAnyStream(ostream& ostr, const AnyStream& stream) {
    ostr << "[";
    for (auto itr = AnyStreamIterator(stream); itr.hasNext();) {
        ostr << proptest::Show<T>(itr.next<T>());
        if(itr.hasNext())
            ostr << ", ";
    }
    ostr << "]";
}
*/

template <typename T>
string serializeStream(const Stream& stream)
{
    stringstream ostr;
    outStream<T>(ostr, stream);
    return ostr.str();
}

/*
template <typename T>
string serializeAnyStream(const AnyStream& stream)
{
    stringstream ostr;
    outAnyStream<T>(ostr, stream);
    return ostr.str();
}
*/

TEST(Stream, empty)
{
    auto stream = Stream::empty(); // empty stream
    EXPECT_EQ(stream.isEmpty(), true);
}

TEST(Stream, one)
{
    auto stream = Stream::one(100);
    EXPECT_EQ(stream.isEmpty(), false);

    StreamIterator<int> itr = stream.iterator<int>();
    ASSERT_EQ(itr.hasNext(), true);

    auto value = itr.next();
    EXPECT_EQ(value, 100);
    ASSERT_EQ(itr.hasNext(), false);

    // copy
    Stream streamCopy = stream;
    EXPECT_EQ(streamCopy.isEmpty(), false);
}

TEST(Stream, two)
{
    auto stream = Stream::two(100, 200);
    EXPECT_EQ(stream.isEmpty(), false);

    StreamIterator<int> itr = stream.iterator<int>();
    ASSERT_EQ(itr.hasNext(), true);

    int value = itr.next();
    EXPECT_EQ(value, 100);
    ASSERT_EQ(itr.hasNext(), true);

    value = itr.next();
    EXPECT_EQ(value, 200);
    ASSERT_EQ(itr.hasNext(), false);
}

TEST(Stream, values)
{
    auto stream = Stream::of<int>(1,2,3,4,5,6,7,8);
    vector<int> values;
    for(StreamIterator<int> itr = stream.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 8U);
    EXPECT_EQ(values, vector<int>({1,2,3,4,5,6,7,8}));
}

TEST(Stream, values2)
{
    auto stream = Stream::values({1,2,3,4,5,6,7,8});
    vector<int> values;
    for(StreamIterator<int> itr = stream.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 8U);
    EXPECT_EQ(values, vector<int>({1,2,3,4,5,6,7,8}));
}

TEST(Stream, iterator)
{
    auto stream = Stream::two(100, 200);
    EXPECT_EQ(stream.isEmpty(), false);

    vector<int> values;
    for(StreamIterator<int> itr = stream.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values, vector<int>({100, 200}));
}

TEST(Stream, string)
{
    auto stream = Stream::two<string>("hello", "world");
    EXPECT_EQ(stream.isEmpty(), false);

    vector<string> values;
    for(StreamIterator<string> itr = stream.iterator<string>(); itr.hasNext(); ) {
        string value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], "hello");
    EXPECT_EQ(values[1], "world");
    EXPECT_EQ(values, vector<string>({"hello", "world"}));
}

TEST(Stream, transform)
{
    auto stream = Stream::two(100, 200);

    auto stream2 = stream.transform<string,int>([](const int& value) { return to_string(value); });

    vector<string> values;
    for(StreamIterator<string> itr = stream2.iterator<string>(); itr.hasNext(); ) {
        string value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], "100");
    EXPECT_EQ(values[1], "200");
}

TEST(Stream, filter)
{
    auto stream = Stream::two(100, 200);
    auto stream2 = stream.filter<int>([](const int& value) { return value > 100; });

    vector<int> values;
    for(StreamIterator<int> itr = stream2.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1U);
    EXPECT_EQ(values[0], 200);

    values.clear();
    auto stream3 = stream.filter<int>([](const int& value) { return value < 200; });

    for(StreamIterator<int> itr = stream3.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1U);
    EXPECT_EQ(values[0], 100);
}

TEST(Stream, concat)
{
    auto stream = Stream::two(100, 200);
    auto stream2 = Stream::two(300, 400);
    auto stream3 = stream.concat(stream2);

    vector<int> values;
    for(StreamIterator<int> itr = stream3.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4U);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
    EXPECT_EQ(values[2], 300);
    EXPECT_EQ(values[3], 400);
}

TEST(Stream, take)
{
    auto stream = Stream::two(100, 200);
    auto stream2 = stream.take(1);

    vector<int> values;
    for(StreamIterator<int> itr = stream2.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 1U);
    EXPECT_EQ(values[0], 100);

    auto stream3 = stream.take(3); // taking exceeding size is ok
    values.clear();
    for(StreamIterator<int> itr = stream3.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
}

// around 30ms with N=12
TEST(Stream, performance)
{
    const int N = 12;
    auto stream = Stream::one(100);
    for(int i = 0; i < N; i++)
        stream = stream.concat(stream);

    vector<int> values;
    for(StreamIterator<int> itr = stream.iterator<int>(); itr.hasNext(); ) {
        int value = itr.next();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4096U);
    EXPECT_EQ(values[4095], 100);
}

/*
TEST(AnyStream, basic)
{
    AnyStream anyStream(Any(100), util::make_anyfunction([]() -> Stream { return Stream::empty(); }));
}

TEST(AnyStream, from_TypedStream)
{
    auto stream = Stream::empty(); // empty stream
    EXPECT_EQ(stream.isEmpty(), true);
    AnyStream anyStream = stream;
    EXPECT_EQ(anyStream.isEmpty(), true);
}

TEST(AnyStream, iterator)
{
    AnyStream anyStream = Stream::two(100, 200);
    EXPECT_EQ(anyStream.isEmpty(), false);

    vector<int> values;
    for(AnyStreamIterator itr(anyStream); itr.hasNext(); ) {
        int value = itr.next<int>();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 200);
}

TEST(AnyStream, transform)
{
    AnyStream stream = Stream::of(1,2,3,4,5,6,7,8);
    auto stream2 = stream.transform(util::make_function([](const Any& value) -> Any { return to_string(value.getRef<int>()); }));
    EXPECT_EQ(serializeAnyStream<int>(stream), "[1, 2, 3, 4, 5, 6, 7, 8]");
    EXPECT_EQ(serializeAnyStream<string>(stream2), "[\"1\" (31), \"2\" (32), \"3\" (33), \"4\" (34), \"5\" (35), \"6\" (36), \"7\" (37), \"8\" (38)]");
}

TEST(AnyStream, filter)
{
    AnyStream stream = Stream::of(1,2,3,4,5,6,7,8);
    auto stream2 = stream.filter(util::make_function([](const Any& value) -> bool { return value.getRef<int>() > 3; }));
    EXPECT_EQ(serializeAnyStream<int>(stream2), "[4, 5, 6, 7, 8]");

    auto stream3 = stream.filter(util::make_function([](const Any& value) -> bool { return value.getRef<int>() < 4; }));
    EXPECT_EQ(serializeAnyStream<int>(stream3), "[1, 2, 3]");
}

TEST(AnyStream, concat)
{
    AnyStream anyStream = Stream::two(100, 200);
    auto stream2 = Stream::two(300, 400);
    auto stream3 = anyStream.concat(stream2);
    EXPECT_EQ(serializeAnyStream<int>(stream3), "[100, 200, 300, 400]");
}

TEST(AnyStream, take)
{
    AnyStream anyStream = Stream::of(1,2,3,4,5,6,7,8);
    auto stream1 = anyStream.take(0);
    EXPECT_EQ(serializeAnyStream<int>(stream1), "[]");
    auto stream2 = anyStream.take(1);
    EXPECT_EQ(serializeAnyStream<int>(stream2), "[1]");

    auto stream3 = anyStream.take(3);
    EXPECT_EQ(serializeAnyStream<int>(stream3), "[1, 2, 3]");

    auto stream4 = anyStream.take(10); // taking exceeding size is ok
    EXPECT_EQ(serializeAnyStream<int>(stream4), "[1, 2, 3, 4, 5, 6, 7, 8]");
}

TEST(AnyStream, performance)
{
    const int N = 12;
    auto stream = Stream::one(100);
    for(int i = 0; i < N; i++)
        stream = stream.concat(stream);

    vector<int> values;
    for(AnyStreamIterator itr(stream); itr.hasNext(); ) {
        int value = itr.next<int>();
        values.push_back(value);
    }
    EXPECT_EQ(values.size(), 4096U);
    EXPECT_EQ(values[4095], 100);
}
*/
