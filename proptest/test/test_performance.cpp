#include "proptest/proptest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(Performance, StreamConstructor)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        EXPECT_EQ(stream.getHeadRef<int>(), i);
    }
}

TEST(Performance, StreamCopy)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        auto stream2 = stream;
        EXPECT_EQ(stream2.getHeadRef<int>(), i);
    }
}

TEST(Performance, StreamVector)
{
    vector<Stream<int>> vec;
    vec.reserve(10000);
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        vec.push_back(stream);
    }

    for(int i = 0; i < 10000; i++)
    {
        const auto& stream = vec[i];
        EXPECT_EQ(stream.getHeadRef<int>(), i);
    }
}

TEST(Performance, StreamFilter)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        stream = stream.filter<int>(+[](const int&) {
            return true;
        });
        EXPECT_EQ(stream.getHeadRef<int>(), i);
    }
}

TEST(Performance, StreamTransform)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        stream = stream.transform<int,int>(+[](const int& n) {
            return n + 1;
        });
        EXPECT_EQ(stream.getHeadRef<int>(), i+1);
    }
}

TEST(Performance, StreamTransformMany)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        for(int j = 0; j < 10; j++) {
            stream = stream.transform<int,int>(+[](const int& n) {
                return n + 1;
            });
        }
        EXPECT_EQ(stream.getHeadRef<int>(), i+10);
    }
}

TEST(Performance, StreamConcatStream)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        stream = stream.concat(Stream<int>::one(i+1));
        EXPECT_EQ(stream.getHeadRef<int>(), i);
    }
}

TEST(Performance, StreamConcatFunc)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<int>::one<int>(i);
        stream = stream.concat([=]() { return Stream<int>::one(i+1); });
        EXPECT_EQ(stream.getHeadRef<int>(), i);
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
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i);
    }
}

TEST(Performance, StreamCopyLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        auto stream2 = stream;
        EXPECT_EQ(stream2.getHeadRef<LargeObject>().array[0], i);
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
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i);
    }
}

TEST(Performance, StreamFilterLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.filter<LargeObject>(+[](const LargeObject&) {
            return true;
        });
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i);
    }
}

TEST(Performance, StreamTransformLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.transform<LargeObject,LargeObject>(+[](const LargeObject& n) {
            return LargeObject(n.array[0]+1);
        });
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i+1);
    }
}

TEST(Performance, StreamTransformManyLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one<LargeObject>(i);
        for(int j = 0; j < 10; j++) {
            stream = stream.transform<LargeObject,LargeObject>(+[](const LargeObject& n) {
                return LargeObject(n.array[0]+1);
            });
        }
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i+10);
    }
}

TEST(Performance, StreamConcatStreamLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.concat(Stream<LargeObject>::one(LargeObject(i+1)));
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i);
    }
}

TEST(Performance, StreamConcatFuncLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>::one(LargeObject(i));
        stream = stream.concat([=]() { return Stream<LargeObject>::one(LargeObject(i+1)); });
        EXPECT_EQ(stream.getHeadRef<LargeObject>().array[0], i);
    }
}

TEST(Performance, StreamIteratorLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto stream = Stream<LargeObject>(LargeObject(i), [=]() { return Stream<LargeObject>::one(LargeObject(i)); });
        int j = 0;
        for(StreamIterator<LargeObject> itr = stream.template iterator<LargeObject>(); itr.hasNext() && j < 100; j++) {
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

TEST(Performance, ShrinkableVector)
{
    vector<Shrinkable<int>> vec;
    vec.reserve(10000);
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        vec.push_back(shr);
    }

    for(int i = 0; i < 10000; i++)
    {
        const auto& shr = vec[i];
        EXPECT_EQ(shr.getRef(), i);
    }
}

TEST(Performance, ShrinkableFilter)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.filter<int>(+[](const int&) {
            return true;
        });
        EXPECT_EQ(shr.getRef(), i);
    }
}

TEST(Performance, ShrinkableMap)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.map<int,int>(+[](const int& n) {
            return n + 1;
        });
        EXPECT_EQ(shr.getRef(), i+1);
    }
}

TEST(Performance, ShrinkableMapMany)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        for(int j = 0; j < 10; j++) {
            shr = shr.map<int,int>(+[](const int& n) {
                return n + 1;
            });
            EXPECT_EQ(shr.getRef(), i+1+j);
        }
    }
}

TEST(Performance, ShrinkableFlatMap)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.flatMap<int,int>(+[](const int& n) {
            return make_shrinkable<int>(n + 1);
        });
        EXPECT_EQ(shr.getRef(), i+1);
    }
}

TEST(Performance, ShrinkableFlatMapMany)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        for(int j = 0; j < 10; j++) {
            shr = shr.flatMap<int,int>(+[](const int& n) {
                return make_shrinkable<int>(n + 1);
            });
            EXPECT_EQ(shr.getRef(), i+1+j);
        }
    }
}

// TODO: slower than proptest1 due to eagerness
TEST(Performance, ShrinkableConcatStatic)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.concatStatic(Stream<Shrinkable<int>>::one(make_shrinkable<int>(i+1)));
        EXPECT_EQ(shr.getRef(), i);
    }
}

TEST(Performance, ShrinkableConcat)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.concat([](const Shrinkable<int>& s) {
            return Stream<Shrinkable<int>>::one(s);
        });
        EXPECT_EQ(shr.getRef(), i);
    }
}

// TODO: slower than proptest1 due to eagerness
TEST(Performance, ShrinkableAndThenStatic)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.andThenStatic(Stream<Shrinkable<int>>::one(make_shrinkable<int>(i+1)));
        EXPECT_EQ(shr.getRef(), i);
    }
}

TEST(Performance, ShrinkableAndthen)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<int>(i);
        shr = shr.andThen([](const Shrinkable<int>& s) {
            return Stream<Shrinkable<int>>::one(s);
        });
        EXPECT_EQ(shr.getRef(), i);
    }
}

////////////////////// Large Object

TEST(Performance, ShrinkableConstructLarge)
{
    for(int i = 0; i < 10000; i++) {
        Shrinkable<LargeObject> shr = make_shrinkable<LargeObject>(i);
        EXPECT_EQ(shr.getRef().array[0], i);
    }
}

TEST(Performance, ShrinkableCopyLarge)
{
    for(int i = 0; i < 10000; i++) {
        Shrinkable<LargeObject> shr = make_shrinkable<LargeObject>(i);
        auto shr2 = shr;
        EXPECT_EQ(shr.getRef().array[0], i);
    }
}

TEST(Performance, ShrinkableVectorLarge)
{
    vector<Shrinkable<LargeObject>> vec;
    vec.reserve(10000);
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<LargeObject>(i);
        vec.push_back(shr);
    }

    for(int i = 0; i < 10000; i++)
    {
        const auto& shr = vec[i];
        EXPECT_EQ(shr.getRef().array[0], i);
    }
}

TEST(Performance, ShrinkableFilterLarge)
{
    for(int i = 0; i < 10000; i++)
    {
        auto shr = make_shrinkable<LargeObject>(i);
        shr = shr.filter<LargeObject>(+[](const LargeObject&) {
            return true;
        });
        EXPECT_EQ(shr.getRef().array[0], i);
    }
}

constexpr uint64_t seed = 1709963283213UL;

TEST(Performance, ArbiBool)
{
    Random rand(seed);
    for(int i = 0; i < 10000; i++)
    {
        auto gen = Arbi<bool>();
        gen(rand);
    }
}

TEST(Performance, ArbiInt)
{
    Random rand(seed);
    for(int i = 0; i < 10000; i++)
    {
        auto gen = Arbi<int>();
        gen(rand);
    }
}

TEST(Performance, ArbiString)
{
    Random rand(seed);
    for(int i = 0; i < 10000; i++)
    {
        auto gen = Arbi<string>();
        gen(rand);
    }
}

TEST(Performance, ArbiList)
{
    Random rand(seed);
    for(int i = 0; i < 10; i++)
    {
        auto gen = Arbi<list<int>>(1, 2);;
        gen(rand);
    }
}