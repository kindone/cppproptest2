#pragma once

#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/map.hpp"
#include "proptest/std/list.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/set.hpp"

namespace proptest {

enum class StatAssertType { GE, LE, IN_RANGE };

struct StatAssertion
{
    StatAssertion(string key, StatAssertType type, double bound1, const char* file, int line, double bound2 = 0.0)
        : key(util::move(key)), type(type), bound1(bound1), bound2(bound2), filename(file), lineno(line)
    {
    }
    string key;
    StatAssertType type;
    double bound1;
    double bound2;  // for IN_RANGE: min, max
    const char* filename;
    int lineno;
};

struct Tag
{
    Tag(const char* f, int l, const string& v) : filename(f), lineno(l), value(v), count(1) {}
    const char* filename;
    const int lineno;
    string value;
    size_t count;
};

struct Failure
{
    Failure(const char* f, int l, const char* c, const stringstream& s)
        : filename(f), lineno(l), condition(c ? c : ""), str(s.str())
    {
    }
    Failure(const char* f, int l, string c, const stringstream& s)
        : filename(f), lineno(l), condition(util::move(c)), str(s.str())
    {
    }
    const char* filename;
    const int lineno;
    string condition;  // stored copy for lifetime safety
    stringstream str;
};

namespace util {
PROPTEST_API uint64_t getGlobalSeed();
}

ostream& operator<<(ostream&, const Failure&);

struct PROPTEST_API PropertyContext
{
    PropertyContext();
    ~PropertyContext();

    void tag(const char* filename, int lineno, string key, string value);
    void succeed(const char* filename, int lineno, const char* condition, const stringstream& str);
    void fail(const char* filename, int lineno, const char* condition, const stringstream& str);
    void fail(const char* filename, int lineno, string condition, const stringstream& str);
    void tag(string key, string value) { tag("?", -1, key, value); }
    stringstream& getLastStream();
    stringstream flushFailures(int indent = 0);
    void printSummary();
    bool hasFailures() const { return !failures.empty(); }

    void addStatAssertGe(string&& key, double minBound, const char* filename, int lineno);
    void addStatAssertLe(string&& key, double maxBound, const char* filename, int lineno);
    void addStatAssertInRange(string&& key, double minBound, double maxBound, const char* filename, int lineno);
    bool checkStatAssertions(size_t totalRuns);

private:
    // key -> (value -> Tag(count, detail))
    map<string, map<string, Tag> > tags;
    list<Failure> failures;
    vector<StatAssertion> statAssertions;
    set<string> statAssertKeys;  // for deduplication
    bool lastStreamExists;

    PropertyContext* oldContext;
};

}  // namespace proptest
