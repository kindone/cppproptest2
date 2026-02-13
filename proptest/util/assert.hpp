#pragma once

#include "proptest/api.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/io.hpp"

/**
 * @file assert.hpp
 * @brief Assertion helpers for property-based testing
 */

namespace proptest {

struct PROPTEST_API AssertFailed : public logic_error
{
    AssertFailed(const char* fname, int line, const error_code& /*error*/, const char* condition,
                 const void* /*caller*/)
        : logic_error(fname, line, condition), filename(fname), lineno(line)
    {
    }

    AssertFailed(const AssertFailed&) = default;

    virtual ~AssertFailed();

    const char* filename;
    int lineno;
};

struct PROPTEST_API PropertyFailedBase : public logic_error
{
    PropertyFailedBase(const AssertFailed& e) : logic_error(e), filename(e.filename), lineno(e.lineno) {}
    virtual ~PropertyFailedBase();

    const char* filename;
    int lineno;
};

struct PROPTEST_API PropertyFailed : public PropertyFailedBase
{
    PropertyFailed(const AssertFailed& e) : PropertyFailedBase(e) {}
    virtual ~PropertyFailed();
};

struct PROPTEST_API Discard : public logic_error
{
    Discard(const char* fname, int line, const error_code& /*error*/, const char* /*condition*/,
            const void* caller)
        : logic_error(fname, line, "Discard", caller)
    {
    }
    virtual ~Discard();
};

struct PROPTEST_API Success : public logic_error
{
    Success(const char* fname, int line, const error_code& /*error*/, const char* /*condition*/,
            const void* caller)
        : logic_error(fname, line, "Discard", caller)
    {
    }
    virtual ~Success();
};

namespace util {
ostream& errorOrEmpty(bool condition);
}

}  // namespace proptest

#define PROP_ASSERT_VARGS(condition, code)                                                                 \
    do {                                                                                                   \
        if (!(condition)) {                                                                                \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, code, #condition, nullptr); \
            throw __proptest_except_obj;                                                                   \
        }                                                                                                  \
    } while (false)

#define PROP_ASSERT_STREAM(condition, a, sign, b)                                                                  \
    do {                                                                                                           \
        if (!(condition)) {                                                                                        \
            stringstream __prop_assert_stream_str;                                                                 \
            __prop_assert_stream_str << #condition << " with " << a << sign << b;                                  \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {}, __prop_assert_stream_str.str(), \
                                                           nullptr);                                               \
            throw __proptest_except_obj;                                                                           \
        }                                                                                                          \
    } while (false)

#define PROP_ASSERT(condition) PROP_ASSERT_VARGS(condition, {})
#define PROP_ASSERT_TRUE(condition) PROP_ASSERT_VARGS(condition, {})
#define PROP_ASSERT_FALSE(condition) PROP_ASSERT_VARGS(!condition, {})

#define PROP_ASSERT_EQ(a, b)                                                                                \
    do {                                                                                                   \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        PROP_ASSERT_STREAM(__prop_a == __prop_b, __prop_a, " != ", __prop_b);                                \
    } while (false)
#define PROP_ASSERT_NE(a, b)                                                                                \
    do {                                                                                                   \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        PROP_ASSERT_STREAM(__prop_a != __prop_b, __prop_a, " == ", __prop_b);                                \
    } while (false)
#define PROP_ASSERT_LT(a, b)                                                                                \
    do {                                                                                                   \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        PROP_ASSERT_STREAM(__prop_a < __prop_b, __prop_a, " >= ", __prop_b);                                  \
    } while (false)
#define PROP_ASSERT_GT(a, b)                                                                                \
    do {                                                                                                   \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        PROP_ASSERT_STREAM(__prop_a > __prop_b, __prop_a, " <= ", __prop_b);                                  \
    } while (false)
#define PROP_ASSERT_LE(a, b)                                                                                \
    do {                                                                                                   \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        PROP_ASSERT_STREAM(__prop_a <= __prop_b, __prop_a, " > ", __prop_b);                                  \
    } while (false)
#define PROP_ASSERT_GE(a, b)                                                                                \
    do {                                                                                                   \
        auto __prop_a = (a);                                                                                \
        auto __prop_b = (b);                                                                                \
        PROP_ASSERT_STREAM(__prop_a >= __prop_b, __prop_a, " < ", __prop_b);                                  \
    } while (false)

#define PROP_ASSERT_STREQ(a, b, n)                                                                            \
    do {                                                                                                      \
        auto __prop_a = (a);                                                                                   \
        auto __prop_b = (b);                                                                                   \
        auto __prop_n = (n);                                                                                   \
        if (!(memcmp(__prop_a, __prop_b, __prop_n) == 0)) {                                                    \
            stringstream __prop_assert_stream_str;                                                            \
            __prop_assert_stream_str << #a << " not equals " << #b << " with " << proptest::Show<char*>(__prop_a, __prop_n) \
                                     << " not equals " << proptest::Show<char*>(__prop_b, __prop_n);                        \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                            \
                                                           __prop_assert_stream_str.str().c_str(), nullptr);  \
            throw __proptest_except_obj;                                                                      \
        }                                                                                                     \
    } while (false)

#define PROP_ASSERT_STREQ2(a, b, n1, n2)                                                                       \
    do {                                                                                                       \
        auto __prop_a = (a);                                                                                    \
        auto __prop_b = (b);                                                                                    \
        auto __prop_n1 = (n1);                                                                                  \
        auto __prop_n2 = (n2);                                                                                  \
        if (!(memcmp(__prop_a, __prop_b, (__prop_n1 <= __prop_n2 ? __prop_n1 : __prop_n2)) == 0)) {             \
            stringstream __prop_assert_stream_str;                                                             \
            __prop_assert_stream_str << #a << " not equals " << #b << " with " << proptest::Show<char*>(__prop_a, __prop_n1) \
                                     << " not equals " << proptest::Show<char*>(__prop_b, __prop_n2);                      \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                             \
                                                           __prop_assert_stream_str.str().c_str(), nullptr);   \
            throw __proptest_except_obj;                                                                       \
        }                                                                                                      \
    } while (false)

#define PROP_ASSERT_STRNE(a, b, n)                                                                           \
    do {                                                                                                     \
        auto __prop_a = (a);                                                                                   \
        auto __prop_b = (b);                                                                                   \
        auto __prop_n = (n);                                                                                   \
        if (!(memcmp(__prop_a, __prop_b, __prop_n) != 0)) {                                                   \
            stringstream __prop_assert_stream_str;                                                           \
            __prop_assert_stream_str << #a << " equals " << #b << " with " << proptest::Show<char*>(__prop_a, __prop_n)    \
                                     << " equals " << proptest::Show<char*>(__prop_b, __prop_n);                           \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                           \
                                                           __prop_assert_stream_str.str().c_str(), nullptr); \
            throw __proptest_except_obj;                                                                     \
        }                                                                                                    \
    } while (false)

#define PROP_ASSERT_STRNE2(a, b, n1, n2)                                                                     \
    do {                                                                                                     \
        auto __prop_a = (a);                                                                                   \
        auto __prop_b = (b);                                                                                   \
        auto __prop_n1 = (n1);                                                                                  \
        auto __prop_n2 = (n2);                                                                                  \
        if (!(memcmp(__prop_a, __prop_b, (__prop_n1 <= __prop_n2 ? __prop_n1 : __prop_n2)) != 0)) {            \
            stringstream __prop_assert_stream_str;                                                           \
            __prop_assert_stream_str << #a << " equals " << #b << " with " << proptest::Show<char*>(__prop_a, __prop_n1)   \
                                     << " equals " << proptest::Show<char*>(__prop_b, __prop_n2);                          \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                           \
                                                           __prop_assert_stream_str.str().c_str(), nullptr); \
            throw __proptest_except_obj;                                                                     \
        }                                                                                                    \
    } while (false)

#define PROP_DISCARD()                                                  \
    do {                                                                \
        throw ::proptest::Discard(__FILE__, __LINE__, {}, "", nullptr); \
    } while (false)

#define PROP_SUCCESS()                                                  \
    do {                                                                \
        throw ::proptest::Success(__FILE__, __LINE__, {}, "", nullptr); \
    } while (false)
