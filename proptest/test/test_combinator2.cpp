#include "proptest/test/testutil.hpp"
#include "proptest/generator/generators.hpp"
#include "proptest/combinator/combinators.hpp"
#include "proptest/Property.hpp"
#include "proptest/util/printing.hpp"

using namespace proptest;


namespace proptest {

// define Arbi of Animal using Construct
template <>
class Arbi<Animal> : public ArbiBase<Animal> {
    Shrinkable<Animal> operator()(Random& rand) const override
    {
        auto tupleGen = tupleOf(Arbi<int>(), Arbi<string>(), Arbi<vector<int>>());
        auto gen = tupleGen.map<Animal>(+[](const tuple<int,string,vector<int>>& tup) {
            return Animal(get<0>(tup), get<1>(tup), get<2>(tup));
        });
        return gen(rand);
    }
};

} // namespace proptest


struct GenSmallInt : public proptest::GeneratorBase<int32_t>
{
    GenSmallInt() : step(0ULL) {}

    proptest::Shrinkable<int32_t> operator()(proptest::Random&) const override
    {
        constexpr size_t num = sizeof(boundaryValues) / sizeof(boundaryValues[0]);
        return proptest::make_shrinkable<int32_t>(boundaryValues[step++ % num]);
    }

    mutable size_t step;
    static constexpr int32_t boundaryValues[13] = {
        INT32_MIN, 0,         INT32_MAX,     -1,           1, -2, 2, INT32_MIN + 1, INT32_MAX - 1,
        INT16_MIN, INT16_MAX, INT16_MIN + 1, INT16_MAX - 1};
};


TEST(PropTest, TestTransform)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Random savedRand = rand;

    Arbi<int> gen;

    {
        Generator<string> stringGen = transform<int,string>(
            gen, +[](const int& value) { return "(" + to_string(value) + ")"; });

        for (int i = 0; i < 10; i++) {
            auto shrinkable = stringGen(rand);
            cout << "string: " << shrinkable.getRef() << endl;
            int j = 0;
            for (auto itr = shrinkable.getShrinks().template iterator<Shrinkable<string>::StreamElementType>(); itr.hasNext() && j < 3; j++) {
                Shrinkable<string> shrinkable2 = itr.next();
                cout << "  shrink: " << shrinkable2.getRef() << endl;
                int k = 0;
                for (auto itr2 = shrinkable2.getShrinks().template iterator<Shrinkable<string>::StreamElementType>(); itr2.hasNext() && k < 3; k++) {
                    cout << "    shrink: " << Shrinkable<string>(itr2.next()).getRef() << endl;
                }
            }
        }

        Generator<vector<string>> vectorGen = transform<string, vector<string>>(
            stringGen, +[](const string& value) {
                vector<string> vec;
                vec.push_back(value);
                return vec;
            });

        for (int i = 0; i < 10; i++) {
            cout << "vector " << vectorGen(rand).getRef()[0] << endl;
        }
    }

    {
        auto stringGen = Arbi<int>().map<string>(+[](const int& value) { return "(" + to_string(value) + ")"; });

        for (int i = 0; i < 10; i++) {
            auto shrinkable = stringGen(savedRand);
            cout << "string2: " << shrinkable.getRef() << endl;
            int j = 0;
            for (auto itr = shrinkable.getShrinks().template iterator<Shrinkable<string>::StreamElementType>(); itr.hasNext() && j < 3; j++) {
                Shrinkable<string> shrinkable2 = itr.next();
                cout << "  shrink2: " << shrinkable2.getRef() << endl;
                int k = 0;
                for (auto itr2 = shrinkable2.getShrinks().template iterator<Shrinkable<string>::StreamElementType>(); itr2.hasNext() && k < 3; k++) {
                    cout << "    shrink2: " << Shrinkable<string>(itr2.next()).getRef() << endl;
                }
            }
        }

        Generator<vector<string>> vectorGen = stringGen.map<vector<string>>(+[](const string& value) {
            vector<string> vec;
            vec.push_back(value);
            return vec;
        });

        for (int i = 0; i < 10; i++) {
            cout << "vector2 " << vectorGen(savedRand).getRef()[0] << endl;
        }
    }
}

TEST(PropTest, TestTranform2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    static Generator<uint8_t> gen = transform<uint8_t, uint8_t>(
        Arbi<uint8_t>(), +[](const uint8_t& vbit) -> uint8_t { return (1 << 0) & vbit; });

    for (int i = 0; i < 10; i++)
        cout << gen(rand).getRef() << endl;
}

TEST(PropTest, TestDependency)
{
    auto intGen = interval(0, 2);
    auto pairGen = dependency<int, vector<int>>(
        intGen, +[](const int& in) {
            auto intGen = interval<int>(0, 8);
            auto vecGen = Arbi<vector<int>>(intGen);
            vecGen.maxSize = in;
            vecGen.minSize = in;
            return vecGen;
        });

    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++) {
        auto pair = pairGen(rand).getRef();
        cout << "(" << pair.first << ", " << pair.second << ")" << endl;
    }
    cout << "exhaustive: " << endl;

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto pairShr = pairGen(rand);
        // exhaustive(pairShr, 0);
    }
}

TEST(PropTest, TestDependency2)
{
    using Dimension = pair<int, uint16_t>;
    using IndexVector = vector<pair<uint16_t, bool>>;
    using RawData = pair<Dimension, IndexVector>;
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto numRowsGen = interval<int>(1000, 1000);
    auto numElementsGen = Arbi<uint16_t>();
    auto dimGen = pairOf(numRowsGen, numElementsGen);

    auto rawGen = dimGen.pairWith<IndexVector>(
        +[](const Dimension& dimension) {
            int numRows = dimension.first;
            uint16_t numElements = dimension.second;
            auto firstGen = interval<uint16_t>(0, numElements);
            auto secondGen = Arbi<bool>();  // TODO true : false should be 2:1
            auto indexGen = pairOf(firstGen, secondGen);
            auto indexVecGen = Arbi<IndexVector>(indexGen);
            indexVecGen.setSize(numRows);
            return indexVecGen;
        });

    cout << "raw." << endl;

    for (int i = 0; i < 10; i++) {
        // cout << "rawGen: " << rawGen(rand).getRef() << " / raw " << i << endl;
        rawGen(rand).getRef();
        cout << "rawGen: " << i <<  endl;
    }

    auto tableDataGen = rawGen.template map<TableData>(
        +[](const RawData& raw) {
            TableData tableData;
            const auto& dimension = raw.first;
            tableData.num_rows = dimension.first;
            tableData.num_elements = dimension.second;
            tableData.indexes = raw.second;
            return tableData;
        });
    cout << "transformed." << endl;

    for (int i = 0; i < 10; i++) {
        tableDataGen(rand).getRef();
    }

    auto tableDataWithValueGen = tableDataGen.template pairWith<vector<bool>>(
        +[](const TableData& td) {
            auto vectorGen = Arbi<vector<bool>>();
            vectorGen.setSize(td.num_elements);
            return vectorGen;
        });

    // exhaustive(tableDataGen(rand), 0);

    // DictionaryCompression::IQTypeInfo ti;
    int i = 0;
    property(
        [&i](pair<TableData, vector<bool>>) {
            // column->set(&index[i].first, index[i].second);
            cout << "running: " << i++ << endl;
            return true;
        },
        tableDataWithValueGen).setNumRuns(100).forAll();
}

TEST(PropTest, TestDependency3)
{
    auto nullableIntegers = dependency<bool, int>(
        Arbi<bool>(), +[](const bool& isNull) -> GenFunction<int> {
            if (isNull)
                return just(0);
            else
                return interval<int>(10, 20);
        });

    auto gen = Arbi<bool>().pairWith<int>(+[](const bool& value) {
        if (value)
            return interval(0, 10);
        else
            return interval(10, 20);
    });

    int64_t seed = getCurrentTime();
    Random rand(seed);

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto pairShr = nullableIntegers(rand);
        serializeShrinkable(pairShr);
        // exhaustive(pairShr, 0);
    }

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto pairShr = gen(rand);
        serializeShrinkable(pairShr);
        // exhaustive(pairShr, 0);
    }
}

TEST(PropTest, TestDependency4)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = elementOf<int>(0, 1, 2, 3);
    auto intStringGen = dependency<int, string>(intGen, [](int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });

    auto stringGen = intStringGen.map<string>([](const pair<int, string>& pair) { return pair.second; });

    Random saveRand = rand;

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto shr = intStringGen(rand);
        serializeShrinkable(shr);
        //exhaustive(shr, 0);
    }

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto shr = stringGen(saveRand);
        serializeShrinkable(shr);
        // exhaustive(shr, 0);
    }
}

TEST(PropTest, TestChain2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto tuple2Gen = Arbi<bool>().tupleWith<int>(+[](const bool& value) {
        if (value)
            return interval(0, 10);
        else
            return interval(10, 20);
    });
    auto tuple3Gen = tuple2Gen.tupleWith<string>(+[](const tuple<bool, int>& tup) {
        cout << tup << endl;
        if (get<0>(tup)) {
            auto gen = Arbi<string>(interval<char>('A', 'M'));
            gen.setSize(1, 3);
            return gen;
        } else {
            auto gen = Arbi<string>(interval<char>('N', 'Z'));
            gen.setSize(1, 3);
            return gen;
        }
    });

    auto tuple3Gen2 = tuple2Gen.tupleWith<int>(+[](const tuple<bool, int>& tup) {
        if (get<0>(tup)) {
            return interval(10, 20);
        } else {
            return interval(20, 30);
        }
    });

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto tupleShr = tuple2Gen(rand);
        serializeShrinkable(tupleShr);
        // exhaustive(tupleShr, 0);
    }

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto tupleShr = tuple3Gen(rand);
        serializeShrinkable(tupleShr);
        // exhaustive(tupleShr, 0);
    }

    for (int i = 0; i < 3; i++) {
        [[maybe_unused]] auto tupleShr = tuple3Gen2(rand);
        serializeShrinkable(tupleShr);
        //exhaustive(tupleShr, 0);
    }
}

TEST(PropTest, TestDerive)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = derive<int, string>(intGen, [](const int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });

    for (int i = 0; i < 10; i++) {
        [[maybe_unused]] auto shr = stringGen(rand);
        //exhaustive(shr, 0);
    }
}

TEST(PropTest, TestDerive2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = intGen.flatMap<string>([](const int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });

    for (int i = 0; i < 10; i++) {
        [[maybe_unused]] auto shr = stringGen(rand);
        serializeShrinkable(shr);
        //exhaustive(shr, 0);
    }
}

TEST(PropTest, TestAccumulate)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen1 = interval<int>(0, 1).map([](const int& num) { return list<int>{num}; });

    [[maybe_unused]] auto gen = accumulate(
        gen1,
        [](const list<int>& nums) {
            auto last = nums.back();
            return interval(last, last + 1).map([nums](const int& num) {
                auto newList = list<int>(nums);
                newList.push_back(num);
                return newList;
            });
        },
        2, 4);

    for (int i = 0; i < 10; i++) {
        Shrinkable<list<int>> shr = gen(rand);
        serializeShrinkable<list<int>>(shr);
        //exhaustive(shr, 0);
    }
}

TEST(PropTest, TestAggregate)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen1 = interval<int>(0, 1);

    [[maybe_unused]] auto gen = aggregate(
        gen1, [](int num) { return interval(num, num + 2); }, 2, 4);

    for (int i = 0; i < 10; i++) {
        Shrinkable<vector<int>> shr = gen(rand);
        serializeShrinkable<vector<int>>(shr);
        // exhaustive(shr, 0);
    }
}

struct Box
{
    Box() {}
    Box(vector<Box>& _children) : children(_children) {}
    vector<Box> children;
};

struct Node
{
    Node(int _value) : value(_value) {}
    Node(int _value, vector<Node>& _children) : value(_value), children(_children) {}

    int value;
    vector<Node> children;
};

namespace proptest {
namespace util {

template <>
struct ShowDefault<Box>
{
    static ostream& show(ostream& os, const Box& n)
    {
        os << "B(";
        if (!n.children.empty()) {
            os << "children: "
               << "[ " << Show<Box>(*n.children.begin());
            for (auto itr = n.children.begin() + 1; itr != n.children.end(); ++itr) {
                os << ", " << Show<Box>(*itr);
            }
            os << " ]";
        }
        os << ")";
        return os;
    }
};
template <>
struct ShowDefault<Node>
{
    static ostream& show(ostream& os, const Node& n)
    {
        os << "N(";
        os << "value: " << n.value;
        if (!n.children.empty()) {
            os << ", children: "
               << "[ " << Show<Node>(*n.children.begin());
            for (auto itr = n.children.begin() + 1; itr != n.children.end(); ++itr) {
                os << ", " << Show<Node>(*itr);
            }
            os << " ]";
        }
        os << ")";
        return os;
    }
};

}  // namespace util

}  // namespace proptest

TEST(PropTest, TestRecursive)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    [[maybe_unused]] auto emptyBoxGen = construct<Box>();
    GenFunction<Box> boxGen = construct<Box, vector<Box>&>(Arbi<vector<Box>>(reference(boxGen)).setSize(0, 2));
    auto tree = boxGen(rand).getRef();
    cout << "tree: " << proptest::Show<Box>(tree) << endl;
}


TEST(PropTest, TestRecursive2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = Arbi<int>();
    auto leafGen = construct<Node, int>(intGen);
    auto leafVecGen = Arbi<vector<Node>>(leafGen).setSize(1, 2);
    auto branch1Gen = construct<Node, int, vector<Node>&>(intGen, leafVecGen);
    GenFunction<Node> branchNGen = construct<Node, int, vector<Node>&>(
        intGen, Arbi<vector<Node>>(oneOf<Node>(branch1Gen, reference(branchNGen))).setSize(1, 2));
    auto tree = branchNGen(rand).getRef();
    cout << "tree: " << proptest::Show<Node>(tree) << endl;
}

TEST(PropTest, TestRecursive3)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    [[maybe_unused]] auto emptyBoxGen = construct<Box>();
    struct BoxGen
    {
        BoxGen(int _level) : level(_level) {}

        Shrinkable<Box> operator()(Random& rand)
        {
            if (level > 0)
                return construct<Box, vector<Box>&>(Arbi<vector<Box>>(BoxGen(level - 1)).setSize(0, 10))(rand);
            else
                return construct<Box>()(rand);
        }

        int level;
    };

    auto boxGen = BoxGen(4);
    auto tree = boxGen(rand).getRef();
    cout << "tree: " << proptest::Show<Box>(tree) << endl;

    Function<GenFunction<Box>(int)> BoxGen2 = [&BoxGen2](int level) -> GenFunction<Box> {
        if (level > 0)
            return construct<Box, vector<Box>&>(Arbi<vector<Box>>(BoxGen2(level - 1)).setSize(0, 10));
        else
            return construct<Box>();
    };

    auto boxGen2 = BoxGen2(2);
    auto tree2 = boxGen2(rand).getRef();
    cout << "tree2: " << proptest::Show<Box>(tree2) << endl;
}

