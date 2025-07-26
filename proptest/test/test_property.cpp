#include "proptest/Property.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/generator/generators.hpp"
#include "proptest/combinator/combinators.hpp"
#include "proptest/gen.hpp"
#include "proptest/std/chrono.hpp"

using namespace proptest;

struct NonCopyable
{
    NonCopyable() = delete;
    NonCopyable(int a) : a(a) { }
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&& other) = delete;
    // NonCopyable(NonCopyable&& other) noexcept : a(other.a) {
    //     other.a = 0; // reset moved-from object
    // };
    ~NonCopyable() {
        cout << "~NonCopyable" << endl;
    }
    int a;
};

struct NonEmptyConstructible
{
    NonEmptyConstructible() = delete;
    NonEmptyConstructible(int a, int b) : a(a), b(b) { }
    NonEmptyConstructible(const NonEmptyConstructible& other) { a = other.a; }
    NonEmptyConstructible(NonEmptyConstructible&& other) = delete;

    int a;
    int b;
};

TEST(Property, NonCopyable)
{
    auto nonCopyableGen = interval(0, 10).map<NonCopyable>([](int n) {
        return util::make_unique<NonCopyable>(n);
    });

    forAll([](const NonCopyable& nc) {
        PROP_STAT(nc.a >= 0 && nc.a <= 10);
        return true;
    }, nonCopyableGen);
}

TEST(Property, matrix)
{
    matrix([](int, int) {
        return true;
    }, {1,2,3}, {2,3,4});
}

TEST(Property, forAll)
{
    forAll([](int a, int b) {
        PROP_ASSERT(0 <= a && a <= 10);
        PROP_ASSERT(20 <= b && b <= 30);
        return true;
    }, interval(0, 10), interval(20, 30));
}


TEST(Property, check_assert)
{
    forAll([](string a, int i, string b) -> bool {
        if (i % 2 == 0)
            PROP_DISCARD();
        PROP_EXPECT_STREQ2(a.c_str(), b.c_str(), a.size(), b.size());
        return true;
    });

    forAll([](string a, int i, string b) -> bool {
        if (i % 2 == 0)
            PROP_SUCCESS();

        PROP_ASSERT_STREQ2(a.c_str(), b.c_str(), a.size(), b.size());
        PROP_STAT(i < 0);
        return true;
    });
}

TEST(Property, propertyBasic)
{
    property([](const vector<int>& a) -> bool {
        PROP_STAT(a.size() > 5);
        return true;
    }).forAll();

    auto func = [](const vector<int>&) -> bool { return true; };

    property(func).forAll();
    forAll(func);

    auto prop = property([](string, int i, string) -> bool {
        PROP_STAT(i > 0);
        PROP_ASSERT(false);
        return true;
    });

    // chaining
    prop.setSeed(0).forAll();
    // with specific arguments
    prop.example(string("hello"), 10, string("world"));
    prop.forAll(elementOf<string>("", string("a")), just(10), elementOf<string>(string(""), string("b")));

    // with specific generators
    string empty("s");
    prop.forAll(just<string>(empty), Arbi<int>(), just<string>(to_string(1)));
}

TEST(Property, property_example)
{
    auto func = [](string, int i, string) -> bool {
        PROP_STAT(i > 0);
        return false;
    };
    auto prop = property(func);
    // with specific arguments
    EXPECT_FALSE(prop.example(string("hello"), 10, string("world")));
}

TYPED_TEST(SignedNumericTest, check_fail)
{
    forAll([](TypeParam a, TypeParam b /*,string str, vector<int> vec*/) -> bool {
        PROP_ASSERT(-10 < a && a < 100 && -20 < b && b < 200);
        return true;
    });
}

TEST(Property, TestCheckBasic)
{
    forAll([](const int& a, const int& b) -> bool {
        EXPECT_EQ(a + b, b + a);
        PROP_STAT(a + b > 0);
        return true;
    });

    forAll([](int a) -> bool {
        PROP_STAT(a > 0);
        return true;
    });

    string a = "Hello";
    string b = "World";
    forAll([](string a, string b) -> bool {
        string c /*(allocator())*/, d /*(allocator())*/;
        c = a + b;
        d = b + a;
        EXPECT_EQ(c.size(), d.size());
        EXPECT_EQ((a + b).substr(0, a.length()), a);
        EXPECT_EQ((a + b).substr(a.length()), b);
        return true;
    });

    forAll([=](UTF8String a, UTF8String b) -> bool {
        string c /*(allocator())*/, d /*(allocator())*/;
        c = a + b;
        d = b + a;
        PROP_ASSERT(c.size() == d.size());
        EXPECT_EQ(c.size(), d.size());
        // EXPECT_EQ(c, d);// << "a: " << a << " + b: " << b << ", a+b: " << (a+b) << ", b+a: " << (b+a);
        return true;
    });
}

struct Bit
{
    uint8_t v;
    const uint16_t len;
    bool null;

    Bit(uint8_t vbit, bool null) : v(vbit), len(sizeof(uint8_t)), null(null) {}
    ~Bit() = default;
};

namespace proptest {

template <>
class Arbi<Bit> : public ArbiBase<Bit> {
public:
    Shrinkable<Bit> operator()(Random& rand) const override
    {
        static auto gen_v =
            proptest::transform<uint8_t, uint8_t>(Arbi<uint8_t>(), [](uint8_t& vbit) -> uint8_t { return (1 << 0) & vbit; });
        static auto gen_bit = construct<Bit, uint8_t, bool>(gen_v, Arbi<bool>());
        return gen_bit(rand);
    }

    shared_ptr<GeneratorBase> clone() const override {
        return util::make_shared<Arbi>();
    }
};
}  // namespace proptest

TEST(Property, TestCheckBit)
{
    forAll([](Bit bit) {
        PROP_STAT(bit.v == 1);
        PROP_STAT(bit.v != 1 && bit.v != 0);
    });
}

struct GenSmallInt : public proptest::GeneratorBase<int32_t>
{
    GenSmallInt() : step(0ULL) {}
    proptest::Shrinkable<int32_t> operator()(proptest::Random&) const override
    {
        constexpr size_t num = sizeof(boundaryValues) / sizeof(boundaryValues[0]);
        return proptest::make_shrinkable<int32_t>(boundaryValues[step++ % num]);
    }

    shared_ptr<GeneratorBase> clone() const override {
        return util::make_shared<GenSmallInt>();
    }

    mutable size_t step;
    static constexpr int32_t boundaryValues[13] = {
        INT32_MIN, 0,         INT32_MAX,     -1,           1, -2, 2, INT32_MIN + 1, INT32_MAX - 1,
        INT16_MIN, INT16_MAX, INT16_MIN + 1, INT16_MAX - 1};
};


TEST(Property, TestCheckWithGen)
{
    /*check([](vector<int> a) -> bool {
        cout << "a: " << a << endl;
        PROP_TAG("a.size() > 0", a.size() > 5);
        return true;
    });*/

    // supply custom generator
    forAll(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        GenSmallInt(), GenSmallInt());

    //
    forAll(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        GenSmallInt());

    GenSmallInt genSmallInt;

    forAll(
        [](int a, int b) {
            PROP_STAT(a > 0);
            PROP_STAT(b > 0);
        },
        genSmallInt, genSmallInt);

    forAll(
        [](int a, string b) {
            PROP_STAT(a > 0);
            PROP_STAT(b.size() > 0);
        },
        genSmallInt);
}

TEST(Property, TestStringCheckFail)
{
    forAll([](string a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 5);
    });
}

TEST(Property, TestUnicodeStringCheckFail)
{
    forAll([](UTF8String a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });

    forAll([](CESU8String a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });

    forAll([](UTF16BEString a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });

    forAll([](UTF16LEString a) {
        PROP_STAT(a.size() > 3);
        PROP_ASSERT(a.size() < 100);
    });
}

TEST(Property, TestStringCheckFail2)
{
    forAll([](string a) {
        PROP_STAT(a.size() > 3);
        PROP_EXPECT(a.size() < 5);
    });

    forAll([](string a) {
        PROP_STAT(a.size() > 3);
        return a.size() < 5;
    });

    forAll([](string a) {
        PROP_STAT(a.size() > 3);
        PROP_EXPECT(a.size() < 5);
        PROP_EXPECT_LT(a.size(), 6);
    });
}

TEST(Property, TestVectorCheckFail)
{
    vector<int> vec;
    vec.push_back(5);
    auto tup = util::make_tuple(vec);
    // cout << "tuple: ";
    show(cout, tup);
    cout << endl;

    auto vecGen = Arbi<vector<int>>();
    vecGen.setMaxSize(32);

    forAll(
        [](vector<int> a) {
            PROP_STAT(a.size() > 3);
            show(cout, a);
            cout << endl;
            PROP_EXPECT_LT(a.size(), 5) << "synthesized failure1";
            PROP_EXPECT_LT(a.size(), 4) << "synthesized failure2";
        },
        vecGen);
}

TEST(Property, TestExampleForAll)
{
    proptest::matrix([](int a, int b) {
        cout << "a: " << a << ", b: " << b << endl;
    }, {1,2,3}, {4,5,6});

}

TEST(Property, TestTupleCheckFail)
{
    forAll([](tuple<int, tuple<int>> tup) {
        int a = get<0>(tup);
        tuple<int> subtup = get<1>(tup);
        int b = get<0>(subtup);
        PROP_ASSERT((-10 < a && a < 100) || (-20 < b && b < 200));
    });
}

TEST(Property, TestArgCheckFail)
{
    forAll([](int a, int b) {
        PROP_ASSERT((-10 < a && a < 100) || (-20 < b && b < 200));
    });
}

bool propertyAsFunc(string, int, vector<int>)
{
    return true;
}

class PropertyAsClass {
public:
    bool operator()(string, int, vector<int>) { return true; }

    static bool propertyAsMethod(string, int, vector<int>) { return true; }
};

TEST(Property, TestPropertyFunctionLambdaMethod)
{
    property(propertyAsFunc).forAll();
    forAll(propertyAsFunc);

    PropertyAsClass propertyAsClass;
    property(propertyAsClass).forAll();
    forAll(propertyAsClass);

    property(PropertyAsClass::propertyAsMethod).forAll();
    forAll(PropertyAsClass::propertyAsMethod);
}

namespace proptest {
namespace util {

template <>
struct ShowDefault<Animal>
{
    static ostream& show(ostream& os, const Animal& a)
    {
        os << "numFeet: " << a.numFeet << ", name: " << a.name << ", measures: ";
        if (!a.measures.empty()) {
            os << a.measures[0];
            for (auto measure = a.measures.begin() + 1; measure != a.measures.end(); ++measure) {
                os << ", " << *measure;
            }
        }
        return os;
    }
};

template <>
struct ShowDefault<NonEmptyConstructible>
{
    static ostream& show(ostream& os, const NonEmptyConstructible& c)
    {
        os << "a: " << c.a << ", b: " << c.b;
        return os;
    }
};


}  // namespace util

}  // namespace proptest

TEST(Property, TestCheckArbitraryWithConstructNonCopyable)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto vecGen = Arbi<vector<int>>();
    vecGen.setMinSize(1);
    vecGen.setMaxSize(20);
    auto tupleGen = tupleOf(gen::interval(1,10));
    auto nonCopyableGen = tupleOf(gen::interval(1,10)).flatMap<NonCopyable>(
        [](const tuple<int>& tup) {
            return gen::just(util::make_any<NonCopyable>(get<0>(tup)));
        });

    EXPECT_FOR_ALL(
        [](const NonCopyable& c) {
            PROP_EXPECT_GE(c.a, 1);
            PROP_EXPECT_LE(c.a, 10);
        },
        nonCopyableGen);

    auto nonCopyableGen2 = tupleOf(gen::interval(1,10)).map([](const tuple<int>& tup) {
        return util::make_unique<NonCopyable>(get<0>(tup));
    });

    EXPECT_FOR_ALL(
        [](const NonCopyable& c) {
            PROP_EXPECT_GE(c.a, 1);
            PROP_EXPECT_LE(c.a, 10);
        },
        nonCopyableGen2);
}

TEST(Property, TestCheckArbitraryWithConstructNonEmptyConstructible)
{
    static_assert(is_constructible_v<NonEmptyConstructible, const NonEmptyConstructible&>);

    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto objGen2 = gen::just(9).map<NonEmptyConstructible>(
        [](const int& val) {
            return NonEmptyConstructible(val, val+1);
        });

    Function<NonEmptyConstructible(const NonEmptyConstructible&)> func = [](const NonEmptyConstructible& c) {
        EXPECT_GE(c.a, 1);
        EXPECT_LE(c.a, 10);
        return c;
    };

    for(int i = 0; i < 10;i ++)
        func(objGen2(rand).getAny().getRef<NonEmptyConstructible>());

    EXPECT_FOR_ALL(
        [](const NonEmptyConstructible& c) {
            PROP_EXPECT_GE(c.a, 1);
            PROP_EXPECT_LE(c.a, 10);
        },
        objGen2);
}

TEST(Property, TestCheckArbitraryWithConstruct)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto vecGen = Arbi<vector<int>>();
    vecGen.setMinSize(1);
    vecGen.setMaxSize(20);
    auto tupleGen = tupleOf(gen::interval(1,10), Arbi<string>(), vecGen);
    auto animalGen = tupleOf(gen::interval(1,10), Arbi<string>(), vecGen).map<Animal>(
        [](const tuple<int,string,vector<int>>& tup) { return Animal(get<0>(tup), get<1>(tup), get<2>(tup)); });
    auto animalGen2 = construct<Animal, int, string, vector<int>&>(gen::interval(1,10), Arbi<string>(), vecGen);
    auto animalVecGen = Arbi<vector<Animal>>(animalGen);
    animalVecGen.setMaxSize(20);

    auto func = [](const proptest::tuple<int, string, vector<int>>& tup) {
        EXPECT_GE(get<0>(tup), 1);
        EXPECT_LE(get<0>(tup), 10);
    };

    for(int i = 0; i < 100;i ++)
        func(tupleGen(rand).getAny().getRef<proptest::tuple<int, string, vector<int>>>());

    EXPECT_FOR_ALL(
        [](const proptest::tuple<int, string, vector<int>>& tup) {
            PROP_EXPECT_GE(get<0>(tup), 1);
            PROP_EXPECT_LE(get<0>(tup), 10);
            if (get<1>(tup).size() < 5 && get<2>(tup).size() < 5) {
                show(cout, tup);
                cout << endl;
            }
            PROP_STAT(get<1>(tup).size() > 0);
        },
        tupleGen);

    EXPECT_FOR_ALL(
        [](const Animal& animal) {
            PROP_EXPECT_GE(animal.numFeet, 1);
            PROP_EXPECT_LE(animal.numFeet, 10);
            if (animal.name.size() < 5 && animal.measures.size() < 5) {
                show(cout, animal);
                cout << endl;
            }
            PROP_STAT(animal.name.size() > 0);
        },
        animalGen);

    EXPECT_FOR_ALL(
        [](const Animal& animal) {
            PROP_EXPECT_GE(animal.numFeet, 1);
            PROP_EXPECT_LE(animal.numFeet, 10);
            if (animal.name.size() < 5 && animal.measures.size() < 5) {
                show(cout, animal);
                cout << endl;
            }
            PROP_STAT(animal.name.size() > 0);
        },
        animalGen2);

    EXPECT_FOR_ALL(
        [](vector<Animal> animals) {
            // cout << "animal " << i++ << endl;
            if (!animals.empty()) {
                for (auto animal : animals) {
                    PROP_EXPECT_GE(animal.numFeet, 1);
                    PROP_EXPECT_LE(animal.numFeet, 10);
                    if (animal.name.size() < 5 && animal.measures.size() < 5) {
                        show(cout, animal);
                        cout << endl;
                    }
                }
                PROP_STAT(animals.size() > 3);
            }
        },
        animalVecGen);
}

decltype(auto) dummyProperty()
{
    using Type = Function<int()>;
    shared_ptr<Type> modelPtr = util::make_shared<Type>([]() { return 0; });
    return property([modelPtr](int) {
        auto model = *modelPtr;
        PROP_STAT(model() > 2);
    });
}

TEST(Property, PropertyCapture)
{
    auto prop = dummyProperty();
    prop.forAll();
}

TEST(Property, DISABLED_TestExpectDeath)
{
    forAll([](vector<int> vec, uint64_t n) {
        auto dangerous = [&vec, n]() { vec[vec.size() - 1 + n] = 100; };
        dangerous();
        // EXPECT_DEATH(, ".*") << "vector: " << vec.size() << ", n: " << n;
    });
}

TEST(Property, PropertyTimed)
{
    auto prop = property([](int value) {
        PROP_STAT(value > 0);
    });
    auto startTime = steady_clock::now();
    prop.setMaxDurationMs(2000).setNumRuns(10000000).forAll();
    auto endTime = steady_clock::now();

    EXPECT_GE(duration_cast<util::milliseconds>(endTime - startTime).count(), 2000);
}
