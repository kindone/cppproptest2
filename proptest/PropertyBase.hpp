#pragma once

#include "proptest/api.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/optional.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Generator.hpp"

#define PROP_EXPECT_STREAM(condition, a, sign, b)                                            \
    ([&]() -> stringstream& {                                                           \
        if (!(condition)) {                                                                  \
            stringstream __prop_expect_stream_str;                                      \
            __prop_expect_stream_str << (a) << (sign) << (b);                                      \
            ::proptest::PropertyBase::fail(__FILE__, __LINE__, #condition, __prop_expect_stream_str);    \
        } else {                                                                             \
            stringstream __prop_expect_stream_str;                                      \
            ::proptest::PropertyBase::succeed(__FILE__, __LINE__, #condition, __prop_expect_stream_str); \
        }                                                                                    \
        return ::proptest::PropertyBase::getLastStream();                                                \
    })()

#define PROP_EXPECT(cond) PROP_EXPECT_STREAM(cond, "", "", "")
#define PROP_EXPECT_TRUE(cond) PROP_EXPECT_STREAM(cond, "", "", "")
#define PROP_EXPECT_FALSE(cond)                                                                             \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_cond = (cond);                                                                          \
        return PROP_EXPECT_STREAM(__prop_cond, __prop_cond, " == ", "true");                                   \
    })()
#define PROP_EXPECT_EQ(a, b)                                                                                \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        return PROP_EXPECT_STREAM((__prop_a == __prop_b), __prop_a, " != ", __prop_b);                        \
    })()
#define PROP_EXPECT_NE(a, b)                                                                                \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        return PROP_EXPECT_STREAM((__prop_a != __prop_b), __prop_a, " == ", __prop_b);                        \
    })()
#define PROP_EXPECT_LT(a, b)                                                                                \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        return PROP_EXPECT_STREAM((__prop_a < __prop_b), __prop_a, " >= ", __prop_b);                         \
    })()
#define PROP_EXPECT_GT(a, b)                                                                                \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        return PROP_EXPECT_STREAM((__prop_a > __prop_b), __prop_a, " <= ", __prop_b);                         \
    })()
#define PROP_EXPECT_LE(a, b)                                                                                \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        return PROP_EXPECT_STREAM((__prop_a <= __prop_b), __prop_a, " > ", __prop_b);                         \
    })()
#define PROP_EXPECT_GE(a, b)                                                                                \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        return PROP_EXPECT_STREAM((__prop_a >= __prop_b), __prop_a, " < ", __prop_b);                         \
    })()
#define PROP_EXPECT_STREQ(a, b, n)                                                                          \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        auto __prop_n = (n);                                                                                \
        return PROP_EXPECT_STREAM(memcmp(__prop_a, __prop_b, __prop_n) == 0,                                 \
                                 ::proptest::Show<char*>(__prop_a, __prop_n), " not equals ",                \
                                 ::proptest::Show<char*>(__prop_b, __prop_n));                              \
    })()
#define PROP_EXPECT_STREQ2(a, b, n1, n2)                                                                    \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        auto __prop_n1 = (n1);                                                                              \
        auto __prop_n2 = (n2);                                                                              \
        return PROP_EXPECT_STREAM(memcmp(__prop_a, __prop_b, (__prop_n1 <= __prop_n2 ? __prop_n1 : __prop_n2)) == 0, \
                                 ::proptest::Show<char*>(__prop_a, __prop_n1), " not equals ",                 \
                                 ::proptest::Show<char*>(__prop_b, __prop_n2));                              \
    })()
#define PROP_EXPECT_STRNE(a, b, n)                                                                          \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        auto __prop_n = (n);                                                                                \
        return PROP_EXPECT_STREAM(memcmp(__prop_a, __prop_b, __prop_n) != 0,                                \
                                 ::proptest::Show<char*>(__prop_a, __prop_n), " equals ",                     \
                                 ::proptest::Show<char*>(__prop_b, __prop_n));                               \
    })()
#define PROP_EXPECT_STRNE2(a, b, n1, n2)                                                                    \
    ([&]() -> stringstream& {                                                                               \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        auto __prop_n1 = (n1);                                                                              \
        auto __prop_n2 = (n2);                                                                              \
        return PROP_EXPECT_STREAM(memcmp(__prop_a, __prop_b, (__prop_n1 <= __prop_n2 ? __prop_n1 : __prop_n2)) != 0, \
                                 ::proptest::Show<char*>(__prop_a, __prop_n1), " equals ",                     \
                                 ::proptest::Show<char*>(__prop_b, __prop_n2));                               \
    })()

#define PROP_STAT(VALUE)                                                                       \
    do {                                                                                       \
        stringstream __prop_stat_key;                                                     \
        __prop_stat_key << (#VALUE);                                                           \
        stringstream __prop_stat_value;                                                   \
        __prop_stat_value << boolalpha;                                                   \
        __prop_stat_value << (VALUE);                                                          \
        ::proptest::PropertyBase::tag(__FILE__, __LINE__, __prop_stat_key.str(), __prop_stat_value.str()); \
    } while (false)

#define PROP_TAG(KEY, VALUE)                                                                   \
    do {                                                                                       \
        stringstream __prop_stat_key;                                                     \
        __prop_stat_key << (KEY);                                                              \
        stringstream __prop_stat_value;                                                   \
        __prop_stat_value << boolalpha;                                                   \
        __prop_stat_value << (VALUE);                                                          \
        ::proptest::PropertyBase::tag(__FILE__, __LINE__, __prop_stat_key.str(), __prop_stat_value.str()); \
    } while (false)

#define PROP_CLASSIFY(condition, KEY, VALUE)                   \
    do {                                                       \
        if (condition) {                                       \
            ::proptest::PropertyBase::tag(__FILE__, __LINE__, KEY, VALUE); \
        }                                                      \
    } while (false)

namespace proptest {

class Random;
struct ShrinkableBase;
struct AnyGenerator;

class PROPTEST_API PropertyBase {
public:
    using GenVec = vector<AnyGenerator>;

    PropertyBase(vector<AnyGenerator>&& gens) : seed(util::getGlobalSeed()), numRuns(defaultNumRuns), maxDurationMs(defaultMaxDurationMs), genVec(util::move(gens)) {}
    virtual ~PropertyBase() {}

    static void setDefaultNumRuns(uint32_t);
    static void tag(const char* filename, int lineno, string key, string value);
    static void succeed(const char* filename, int lineno, const char* condition, const stringstream& str);
    static void fail(const char* filename, int lineno, const char* condition, const stringstream& str);
    static stringstream& getLastStream();

    virtual void writeArgs(ostream& os, const vector<ShrinkableBase>& shrVec) const = 0;
    virtual void writeArgs(ostream& os, const vector<Any>& anyVec) const = 0;

    bool exampleImpl(const vector<Any>& anyVec);
    bool runForAll(const GenVec& curGenVec);
    bool test(const vector<ShrinkableBase>& curShrVec);

    void shrink(Random& savedRand, const GenVec& curGenVec);

    struct ShowShrVec {
        ShowShrVec(const PropertyBase& _property, const vector<ShrinkableBase>& _shrVec) : property(_property), shrVec(_shrVec) {}

        friend ostream& operator<<(ostream& os, const ShowShrVec& show)
        {
            show.property.writeArgs(os, show.shrVec);
            return os;
        }
        const PropertyBase& property;
        const vector<ShrinkableBase>& shrVec;
    };

    struct ShowAnyVec {
        ShowAnyVec(const PropertyBase& _property, const vector<Any>& _anyVec) : property(_property), anyVec(_anyVec) {}

        friend ostream& operator<<(ostream& os, const ShowAnyVec& show)
        {
            show.property.writeArgs(os, show.anyVec);
            return os;
        }
        const PropertyBase& property;
        const vector<Any>& anyVec;
    };

protected:
    static void setContext(PropertyContext* context);
    static PropertyContext* getContext() { return context; }
    static PropertyContext* context;

protected:
    bool invoke(Random& rand);
    virtual bool callFunction(const vector<Any>& anyVec) = 0;
    virtual bool callFunction(const vector<ShrinkableBase>& shrVec) = 0;
    virtual bool callFunctionFromGen(Random& rand, const vector<AnyGenerator>& genVec) = 0;

    static uint32_t defaultNumRuns;
    static uint32_t defaultMaxDurationMs;

    // TODO: configurations
    uint64_t seed;
    uint32_t numRuns;

    uint32_t maxDurationMs; // indefinitely if 0

    Function<void()> onStartup;
    Function<void()> onCleanup;

    vector<AnyGenerator> genVec;

    /// Last test run result (true = success, false = failure). Used for operator bool() and chainable API.
    bool lastRunOk = true;

    friend struct PropertyContext;
};

}  // namespace proptest
