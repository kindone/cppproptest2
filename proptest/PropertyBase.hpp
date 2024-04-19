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
#define PROP_EXPECT_FALSE(cond) PROP_EXPECT_STREAM(cond, cond, " == ", "true")
#define PROP_EXPECT_EQ(a, b) PROP_EXPECT_STREAM((a == b), a, " != ", b)
#define PROP_EXPECT_NE(a, b) PROP_EXPECT_STREAM((a != b), a, " == ", b)
#define PROP_EXPECT_LT(a, b) PROP_EXPECT_STREAM((a < b), a, " >= ", b)
#define PROP_EXPECT_GT(a, b) PROP_EXPECT_STREAM((a > b), a, " <= ", b)
#define PROP_EXPECT_LE(a, b) PROP_EXPECT_STREAM((a <= b), a, " > ", b)
#define PROP_EXPECT_GE(a, b) PROP_EXPECT_STREAM((a >= b), a, " < ", b)
#define PROP_EXPECT_STREQ(a, b, n) \
    PROP_EXPECT_STREAM(memcmp(a, b, n) == 0, ::proptest::Show<char*>(a, n), " not equals ", ::proptest::Show<char*>(b, n))
#define PROP_EXPECT_STREQ2(a, b, n1, n2)                                                                      \
    PROP_EXPECT_STREAM(memcmp(a, b, (n1 <= n2 ? n1 : n2)) == 0, ::proptest::Show<char*>(a, n1), " not equals ", \
                       ::proptest::Show<char*>(b, n2))
#define PROP_EXPECT_STRNE(a, b, n) \
    PROP_EXPECT_STREAM(memcmp(a, b, n) != 0, ::proptest::Show<char*>(a, n), " equals ", ::proptest::Show<char*>(b, n))
#define PROP_EXPECT_STRNE2(a, b, n1, n2)                                                                  \
    PROP_EXPECT_STREAM(memcmp(a, b, (n1 <= n2 ? n1 : n2)) != 0, ::proptest::Show<char*>(a, n1), " equals ", \
                       ::proptest::Show<char*>(b, n2))

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

    bool exampleImpl(const vector<Any>& anyVec);
    bool runForAll(GenVec& curGenVec);
    bool test(const vector<ShrinkableBase>& curShrVec);

    void shrink(Random& savedRand, const GenVec& curGenVec);

    void setSeed(uint64_t s) { seed = s; }
    void setNumRuns(uint32_t runs) { numRuns = runs; }
    void setMaxDurationMs(uint32_t durationMs) { maxDurationMs = durationMs; }
    void setOnStartup(Function<void()> _onStartup) { onStartup = _onStartup; }
    void setOnCleanup(Function<void()> _onCleanup) { onCleanup = _onCleanup; }


    void setCallFunctionWithShr(Function<bool(const vector<ShrinkableBase>&)> _callFunctionWithShr) { callFunctionWithShr = _callFunctionWithShr; }
    void setCallFunction(Function<bool(const vector<Any>&)> _callFunction) { callFunction = _callFunction; }
    void setCallFunctionFromGen(Function<bool(Random&, const vector<AnyGenerator>&)> _callFunctionFromGen) { callFunctionFromGen = _callFunctionFromGen; }
    void setWriteShrs(Function<void(ostream& os, const vector<ShrinkableBase>& shrVec)> _writeShrs) { writeShrs = _writeShrs; }
    void setWriteArgs(Function<void(ostream& os, const vector<Any>& anyVec)> _writeArgs) { writeArgs = _writeArgs; }

    struct ShowShrVec {
        ShowShrVec(const PropertyBase& _property, const vector<ShrinkableBase>& _shrVec) : property(_property), shrVec(_shrVec) {}

        friend ostream& operator<<(ostream& os, const ShowShrVec& show)
        {
            show.property.writeShrs(os, show.shrVec);
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

    static uint32_t defaultNumRuns;
    static uint32_t defaultMaxDurationMs;

    // TODO: configurations
    uint64_t seed;
    uint32_t numRuns;

    uint32_t maxDurationMs; // indefinitely if 0

    Function<void()> onStartup;
    Function<void()> onCleanup;

    Function<bool(const vector<ShrinkableBase>&)> callFunctionWithShr;
    Function<bool(const vector<Any>&)> callFunction;
    Function<bool(Random&, const vector<AnyGenerator>&)> callFunctionFromGen;
public:
    Function<void(ostream& os, const vector<ShrinkableBase>& shrVec)> writeShrs;
    Function<void(ostream& os, const vector<Any>& anyVec)> writeArgs;
protected:
    vector<AnyGenerator> genVec;

    friend struct PropertyContext;
};

}  // namespace proptest
