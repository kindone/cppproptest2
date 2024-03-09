#include "proptest/proptest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(Performance, StreamConstructor)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        EXPECT_EQ(stream.getHeadRef(), i);
    }
}

TEST(Performance, StreamCopy)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        auto stream2 = stream;
        EXPECT_EQ(stream2.getHeadRef(), i);
    }
}

TEST(Performance, StreamVector)
{
    vector<Stream<int>> vec;
    vec.reserve(10000);
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        vec.push_back(stream);
    }

    for(int i = 0; i < 10000; i++)
    {
        const auto& stream = vec[i];
        EXPECT_EQ(stream.getHeadRef(), i);
    }
}

TEST(Performance, StreamFilter)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        stream = stream.filter(+[](const int& n) {
            return true;
        });
        EXPECT_EQ(stream.getHeadRef(), i);
    }
}

TEST(Performance, StreamTransform)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        stream = stream.transform<int>(+[](const int& n) {
            return n + 1;
        });
        EXPECT_EQ(stream.getHeadRef(), i+1);
    }
}

TEST(Performance, StreamTransformMany)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        for(int j = 0; j < 10; j++) {
            stream = stream.transform<int>(+[](const int& n) {
                return n + 1;
            });
        }
        EXPECT_EQ(stream.getHeadRef(), i+10);
    }
}

TEST(Performance, StreamConcatStream)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        stream = stream.concat(Stream<int>::one(i+1));
        EXPECT_EQ(stream.getHeadRef(), i);
    }
}

TEST(Performance, StreamConcatFunc)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one(i);
        stream = stream.concat([=]() { return Stream<int>::one(i+1); });
        EXPECT_EQ(stream.getHeadRef(), i);
    }
}

TEST(Performance, StreamIterator)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>(i, [=]() { return Stream<int>::one(i); });
        int j = 0;
        for(StreamIterator<int> itr(stream); itr.hasNext() && j < 100; j++) {
            auto res = itr.next();
            EXPECT_EQ(res, i);
        }
    }
}

////////////////////// Large Object

struct LargeObject {
    LargeObject(int i) : array(1000, i) {}
    vector<int> array;
};

TEST(Performance, StreamConstructorLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        EXPECT_EQ(stream.getHeadRef().array[0], i);
    }
}

TEST(Performance, StreamCopyLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        auto stream2 = stream;
        EXPECT_EQ(stream2.getHeadRef().array[0], i);
    }
}

TEST(Performance, StreamVectorLarge)
{
    vector<Stream<LargeObject>> vec;
    vec.reserve(10000);
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        vec.push_back(stream);
    }

    for(int i = 0; i < 10000; i++)
    {
        const auto& stream = vec[i];
        EXPECT_EQ(stream.getHeadRef().array[0], i);
    }
}

TEST(Performance, StreamFilterLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.filter(+[](const LargeObject& n) {
            return true;
        });
        EXPECT_EQ(stream.getHeadRef().array[0], i);
    }
}

TEST(Performance, StreamTransformLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.transform<LargeObject>(+[](const LargeObject& n) {
            return LargeObject(n.array[0]+1);
        });
        EXPECT_EQ(stream.getHeadRef().array[0], i+1);
    }
}

TEST(Performance, StreamTransformManyLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(i);
        for(int j = 0; j < 10; j++) {
            stream = stream.transform<LargeObject>(+[](const LargeObject& n) {
                return LargeObject(n.array[0]+1);
            });
        }
        EXPECT_EQ(stream.getHeadRef().array[0], i+10);
    }
}

TEST(Performance, StreamConcatStreamLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.concat(Stream<LargeObject>::one(LargeObject(i+1)));
        EXPECT_EQ(stream.getHeadRef().array[0], i);
    }
}

TEST(Performance, StreamConcatFuncLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.concat([=]() { return Stream<LargeObject>::one(LargeObject(i+1)); });
        EXPECT_EQ(stream.getHeadRef().array[0], i);
    }
}

TEST(Performance, StreamIteratorLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>(LargeObject(i), [=]() { return Stream<LargeObject>::one(LargeObject(i)); });
        int j = 0;
        for(StreamIterator<LargeObject> itr = stream.iterator(); itr.hasNext() && j < 100; j++) {
            auto res = itr.next();
            EXPECT_EQ(res.array[0], i);
        }
    }
}

TEST(Performance, ShrinkableConstruct)
{
    for(int i = 0; i < 10000; i++) {
        Shrinkable<int> shr = make_shrinkable<int>(i);
        EXPECT_EQ(shr.getRef(), i);
    }
}

TEST(Performance, ShrinkableCopy)
{
    for(int i = 0; i < 10000; i++) {
        Shrinkable<int> shr = make_shrinkable<int>(i);
        auto shr2 = shr;
        EXPECT_EQ(shr2.getRef(), i);
    }
}