#include "proptest/Arbitrary.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/floating.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/generator/vector.hpp"
#include "proptest/generator/set.hpp"
#include "proptest/generator/map.hpp"
#include "proptest/generator/pair.hpp"
#include "proptest/generator/tuple.hpp"
#include "proptest/generator/string.hpp"
#include "proptest/generator/utf8string.hpp"
#include "proptest/generator/utf16string.hpp"
#include "proptest/generator/cesu8string.hpp"
#include "proptest/generator/shared_ptr.hpp"
#include "proptest/Random.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/printing.hpp"

using namespace proptest;

TEST(Arbitrary, bool)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<bool>();
    for(int i = 0; i < 5; i++)
        cout << arbi(rand).getRef() << endl;
    // TODO: validate
}

/*
TEST(Arbitrary, toGenerator)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<bool>();
    Generator<bool> gen = arbi;
    for(int i = 0; i < 5; i++)
        cout << gen(rand).getRef() << endl;

    // rvalue
    Generator<bool> gen2 = Arbi<bool>();
}
*/

TEST(Arbitrary, toGenFunction)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<bool>();
    GenFunction<bool> gen = arbi;
    for(int i = 0; i < 5; i++)
        cout << gen(rand).getRef() << endl;

    // rvalue
    GenFunction<bool> gen2 = Arbi<bool>();
}

TEST(Arbitrary, int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto val = arbi(rand).getRef();
    for(int i = 0; i < 5; i++)
        cout << arbi(rand).getRef() << endl;
    EXPECT_TRUE(val >= numeric_limits<int>::min());
    EXPECT_TRUE(val <= numeric_limits<int>::max());
}

TEST(Arbitrary, double)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<double>();
    for(int i = 0; i < 5; i++)
        cout << arbi(rand).getRef() << endl;
}

TEST(Arbitrary, list_int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<list<int>>();
    for(int i = 0; i < 2; i++) {
        auto shr = arbi(rand);
        show(cout, shr.getRef());
        cout << endl;
        EXPECT_LE(shr.getRef().size(), Arbi<list<int>>::defaultMaxSize);
    }

    auto arbi2 = Arbi<list<int>>(Arbi<int>());
    for(int i = 0; i < 2; i++) {
        auto shr = arbi2(rand);
        EXPECT_LE(shr.getRef().size(), Arbi<list<int>>::defaultMaxSize);
    }

    auto arbi3 = Arbi<list<int>>(gen::interval<int>(0, 9));
    for(int i = 0; i < 2; i++) {
        auto shr = arbi3(rand);
        const auto& l = shr.getRef();
        EXPECT_LE(l.size(), Arbi<list<int>>::defaultMaxSize);
        for(auto val : l) {
            EXPECT_TRUE(0<= val && val <= 9);
        }
    }

    // Config-based constructor
    auto arbi4 = Arbi<list<int>>({.elemGen = gen::interval<int>(0, 5), .minSize = 1, .maxSize = 4});
    for(int i = 0; i < 5; i++) {
        auto shr = arbi4(rand);
        const auto& l = shr.getRef();
        EXPECT_GE(l.size(), 1u);
        EXPECT_LE(l.size(), 4u);
        for(auto val : l) {
            EXPECT_TRUE(0 <= val && val <= 5);
        }
    }
}

TEST(Arbitrary, vector_int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<vector<int>>();
    for(int i = 0; i < 2; i++) {
        auto shr = arbi(rand);
        show(cout, shr.getRef());
        cout << endl;
        EXPECT_LE(shr.getRef().size(), Arbi<vector<int>>::defaultMaxSize);
    }

    auto arbi2 = Arbi<vector<int>>(Arbi<int>());
    for(int i = 0; i < 2; i++) {
        auto shr = arbi2(rand);
        EXPECT_LE(shr.getRef().size(), Arbi<vector<int>>::defaultMaxSize);
    }

    auto arbi3 = Arbi<vector<int>>(gen::interval<int>(0, 9));
    for(int i = 0; i < 2; i++) {
        auto shr = arbi3(rand);
        const auto& l = shr.getRef();
        EXPECT_LE(l.size(), Arbi<vector<int>>::defaultMaxSize);
        for(auto val : l) {
            EXPECT_TRUE(0<= val && val <= 9);
        }
    }

    // Config-based constructor (named parameters)
    auto arbi4 = Arbi<vector<int>>({.minSize = 5, .maxSize = 10});
    for(int i = 0; i < 5; i++) {
        auto shr = arbi4(rand);
        const auto& v = shr.getRef();
        EXPECT_GE(v.size(), 5u);
        EXPECT_LE(v.size(), 10u);
    }
    auto arbi5 = Arbi<vector<int>>({.elemGen = gen::interval<int>(1, 100), .minSize = 2, .maxSize = 5});
    for(int i = 0; i < 5; i++) {
        auto shr = arbi5(rand);
        const auto& v = shr.getRef();
        EXPECT_GE(v.size(), 2u);
        EXPECT_LE(v.size(), 5u);
        for(auto val : v) {
            EXPECT_TRUE(1 <= val && val <= 100);
        }
    }
}

TEST(Arbitrary, set_int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<set<int>>();
    for(int i = 0; i < 5; i++) {
        auto shr = arbi(rand);
        show(cout, shr.getRef());
        cout << endl;
        EXPECT_LE(shr.getRef().size(), (Arbi<set<int   >>::defaultMaxSize));
    }

    // Config-based constructor
    auto arbi2 = Arbi<set<int>>({.minSize = 3, .maxSize = 8});
    for(int i = 0; i < 5; i++) {
        auto shr = arbi2(rand);
        EXPECT_GE(shr.getRef().size(), 3u);
        EXPECT_LE(shr.getRef().size(), 8u);
    }
}

TEST(Arbitrary, map_int_int)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<map<int,int>>();
    for(int i = 0; i < 2; i++) {
        auto shr = arbi(rand);
        show(cout, shr.getRef());
        cout << endl;
        EXPECT_LE(shr.getRef().size(), (Arbi<map<int,int>>::defaultMaxSize));
    }

    // Config-based constructor
    auto arbi2 = Arbi<map<int,int>>({.minSize = 2, .maxSize = 6});
    for(int i = 0; i < 5; i++) {
        auto shr = arbi2(rand);
        EXPECT_GE(shr.getRef().size(), 2u);
        EXPECT_LE(shr.getRef().size(), 6u);
    }
    auto arbi3 = Arbi<map<int,int>>({.keyGen = gen::interval<int>(0, 9), .valueGen = gen::interval<int>(1, 100), .minSize = 1, .maxSize = 4});
    for(int i = 0; i < 5; i++) {
        auto shr = arbi3(rand);
        const auto& m = shr.getRef();
        EXPECT_GE(m.size(), 1u);
        EXPECT_LE(m.size(), 4u);
        for(const auto& [k, v] : m) {
            EXPECT_TRUE(0 <= k && k <= 9);
            EXPECT_TRUE(1 <= v && v <= 100);
        }
    }
}

TEST(Arbitrary, pair_int_string)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<pair<int,string>>(gen::interval(0,10), gen::interval(0,10).map<string>([](int n) { return to_string(n); }));
    for(int i = 0; i < 10; i++) {
        auto shr = arbi(rand);
        show(cout, shr.getRef());
        cout << endl;
        const auto& thePair = shr.getRef();
        EXPECT_TRUE(0 <= thePair.first && thePair.first <= 10);
        EXPECT_TRUE(1 <= thePair.second.size() && thePair.second.size() <= 2);
    }
}

TEST(Arbitrary, tuple_int_string)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<tuple<int,string>>(gen::interval(0,10), gen::interval(0,10).map<string>([](int n) { return to_string(n); }));
    for(int i = 0; i < 10; i++) {
        auto shr = arbi(rand);
        show(cout, shr.getRef());
        cout << endl;
        const auto& tup = shr.getRef();
        EXPECT_TRUE(0 <= get<0>(tup) && get<0>(tup) <= 10);
        EXPECT_TRUE(1 <= get<1>(tup).size() && get<1>(tup).size() <= 2);
    }
}

TEST(Arbitrary, string_default)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<string>();
    for(int i = 0; i < 100; i++) {
        auto str = arbi(rand).getRef();
        EXPECT_LE(str.size(), Arbi<string>::defaultMaxSize);
    }

    auto arbi2 = Arbi<string>(Arbi<char>());
    for(int i = 0; i < 100; i++) {
        auto str = arbi2(rand).getRef();
        EXPECT_LE(str.size(), Arbi<string>::defaultMaxSize);
    }
}

TEST(Arbitrary, string_customchars)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<string>(gen::interval<char>('0','9'));
    for(int i = 0; i < 100; i++) {
        auto str = arbi(rand).getRef();
        for(auto c : str) {
            EXPECT_TRUE(c >= '0' && c<= '9');
        }
    }

    // Config-based constructor
    auto arbi2 = Arbi<string>({.minSize = 5, .maxSize = 15});
    for(int i = 0; i < 10; i++) {
        auto str = arbi2(rand).getRef();
        EXPECT_GE(str.size(), 5u);
        EXPECT_LE(str.size(), 15u);
    }
    auto arbi3 = Arbi<string>({.elemGen = gen::interval<char>('A', 'Z'), .minSize = 2, .maxSize = 8});
    for(int i = 0; i < 10; i++) {
        auto str = arbi3(rand).getRef();
        EXPECT_GE(str.size(), 2u);
        EXPECT_LE(str.size(), 8u);
        for(auto c : str) {
            EXPECT_TRUE(c >= 'A' && c <= 'Z');
        }
    }
}

template <typename T, typename...ARGS>
bool isStringValid(ARGS&&...args) {
    if constexpr(is_same_v<T, UTF8String>)
        return util::isValidUTF8(util::forward<ARGS>(args)...);
    else if(is_same_v<T, CESU8String>)
        return util::isValidCESU8(util::forward<ARGS>(args)...);
    else if(is_same_v<T, UTF16BEString>)
        return util::isValidUTF16BE(util::forward<ARGS>(args)...);
    else if(is_same_v<T, UTF16LEString>)
        return util::isValidUTF16LE(util::forward<ARGS>(args)...);
    else {
        throw runtime_error(__FILE__, __LINE__, "unsupported string type");
    }
}

TYPED_TEST(StringLikeTest, Arbitrary)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<TypeParam>();
    for(int i = 0; i < 10; i++) {
        auto str = arbi(rand).getRef();
        EXPECT_LE(str.charsize(), Arbi<TypeParam>::defaultMaxSize);

        vector<uint8_t> vec(str.begin(), str.end());
        size_t length = 0;
        EXPECT_TRUE(isStringValid<TypeParam>(vec, length));
        EXPECT_EQ(length, str.charsize());
        EXPECT_GE(vec.size(), str.charsize()); // byte count can be greater then the char size
    }
}

TYPED_TEST(StringLikeTest, Arbitrary_customchars)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<TypeParam>(gen::interval<uint32_t>('0','9'));
    for(int i = 0; i < 10; i++) {
        auto str = arbi(rand).getRef();
        for(auto c : str) {
            if constexpr(is_same_v<TypeParam, UTF8String> || is_same_v<TypeParam, CESU8String>) {
                EXPECT_TRUE(c >= '0' && c<= '9');
            }
        }
    }

    // Config-based constructor for UTF-8/CESU-8 strings
    if constexpr(is_same_v<TypeParam, UTF8String> || is_same_v<TypeParam, CESU8String>) {
        auto arbi2 = Arbi<TypeParam>({.minSize = 1, .maxSize = 5});
        for(int i = 0; i < 5; i++) {
            auto str = arbi2(rand).getRef();
            EXPECT_GE(str.charsize(), 1u);
            EXPECT_LE(str.charsize(), 5u);
        }
    }
}

TEST(Arbitrary, basic)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto val = arbi(rand).getRef();
    for(int i = 0; i < 10; i++)
        cout << arbi(rand).getRef() << endl;
    EXPECT_TRUE(val >= numeric_limits<int>::min());
    EXPECT_TRUE(val <= numeric_limits<int>::max());
}

TEST(Arbitrary, monadic)
{
    Random rand(getCurrentTime());
    auto arbi = Arbi<int>();
    auto gen = arbi.map<int>([](const int& val) { return val * 2; });
    auto gen2 = gen.filter([](const int& val) { return val > 0; });

    for(int i = 0; i < 10; i++) {
        auto val = gen2(rand).getRef();
        cout << val << endl;
        EXPECT_TRUE(val % 2 == 0 && val > 0);
    }
}
