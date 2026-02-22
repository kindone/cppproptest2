#include "proptest/PropertyContext.hpp"
#include "proptest/PropertyBase.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

ostream& operator<<(ostream& os, const Failure& f)
{
    auto detail = f.str.str();
    if (detail.empty())
        os << f.condition << " (" << f.filename << ":" << f.lineno << ")";
    else
        os << f.condition << " (" << f.filename << ":" << f.lineno << ") with " << f.str.str();
    return os;
}

PropertyContext::PropertyContext() : lastStreamExists(false), oldContext(PropertyBase::getContext())
{
    PropertyBase::setContext(this);
}

PropertyContext::~PropertyContext()
{
    PropertyBase::setContext(oldContext);
}

void PropertyContext::tag(const char* file, int lineno, string key, string value)
{
    auto itr = tags.find(key);
    // key already exists
    if (itr != tags.end()) {
        auto& valueMap = itr->second;
        auto valueItr = valueMap.find(value);
        // value already exists
        if (valueItr != valueMap.end()) {
            auto& tag = valueItr->second;
            tag.count++;
        } else {
            valueMap.insert(pair<string, Tag>(value, Tag(file, lineno, value)));
        }
    } else {
        map<string, Tag> valueMap;
        valueMap.insert(pair<string, Tag>(value, Tag(file, lineno, value)));
        tags.insert(pair<string, map<string, Tag>>(key, valueMap));
    }
}

void PropertyContext::succeed(const char*, int, const char*, const stringstream&)
{
    // DO NOTHING
    lastStreamExists = false;
}

void PropertyContext::fail(const char* filename, int lineno, const char* condition, const stringstream& str)
{
    failures.push_back(Failure(filename, lineno, condition, str));
    lastStreamExists = true;
}

void PropertyContext::fail(const char* filename, int lineno, string condition, const stringstream& str)
{
    failures.push_back(Failure(filename, lineno, util::move(condition), str));
    lastStreamExists = true;
}

stringstream& PropertyContext::getLastStream()
{
    static stringstream defaultStr;
    if (failures.empty() || !lastStreamExists)
        return defaultStr;

    return failures.back().str;
}

stringstream PropertyContext::flushFailures(int indent)
{
    const auto doIndent = +[](stringstream& str, int indent) {
        for (int i = 0; i < indent; i++)
            str << " ";
    };

    stringstream allFailures;
    auto itr = failures.begin();
    if (itr != failures.end()) {
        // doIndent(allFailures, indent);
        allFailures << *itr++;
    }
    for (; itr != failures.end(); ++itr) {
        allFailures << "," << endl;
        doIndent(allFailures, indent);
        allFailures << *itr;
    }
    failures.clear();
    return allFailures;
}

void PropertyContext::addStatAssertGe(string&& key, double minBound, const char* filename, int lineno)
{
    string dedupKey = "GE:" + key + ":" + to_string(minBound);
    if (statAssertKeys.find(dedupKey) != statAssertKeys.end())
        return;
    statAssertKeys.insert(dedupKey);
    statAssertions.push_back(StatAssertion(util::move(key), StatAssertType::GE, minBound, filename, lineno));
}

void PropertyContext::addStatAssertLe(string&& key, double maxBound, const char* filename, int lineno)
{
    string dedupKey = "LE:" + key + ":" + to_string(maxBound);
    if (statAssertKeys.find(dedupKey) != statAssertKeys.end())
        return;
    statAssertKeys.insert(dedupKey);
    statAssertions.push_back(StatAssertion(util::move(key), StatAssertType::LE, maxBound, filename, lineno));
}

void PropertyContext::addStatAssertInRange(string&& key, double minBound, double maxBound, const char* filename, int lineno)
{
    string dedupKey = "IN_RANGE:" + key + ":" + to_string(minBound) + ":" + to_string(maxBound);
    if (statAssertKeys.find(dedupKey) != statAssertKeys.end())
        return;
    statAssertKeys.insert(dedupKey);
    statAssertions.push_back(StatAssertion(util::move(key), StatAssertType::IN_RANGE, minBound, filename, lineno, maxBound));
}

bool PropertyContext::checkStatAssertions(size_t totalRuns)
{
    if (totalRuns == 0)
        return true;
    static const string kTrueValue("true");  // PROP_STAT records bool as "true"/"false"; we assert on "true" ratio
    bool allPassed = true;
    for (const auto& a : statAssertions) {
        size_t count = 0;
        auto keyItr = tags.find(a.key);
        if (keyItr != tags.end()) {
            auto valueItr = keyItr->second.find(kTrueValue);
            if (valueItr != keyItr->second.end())
                count = valueItr->second.count;
        }
        double ratio = static_cast<double>(count) / totalRuns;
        bool pass = false;
        stringstream ss;
        switch (a.type) {
            case StatAssertType::GE:
                pass = ratio >= a.bound1;
                if (!pass)
                    ss << "PROP_STAT_ASSERT_GE(" << a.key << ", " << a.bound1 << ") failed: ratio " << ratio
                       << " < " << a.bound1 << " (" << count << "/" << totalRuns << ")";
                break;
            case StatAssertType::LE:
                pass = ratio <= a.bound1;
                if (!pass)
                    ss << "PROP_STAT_ASSERT_LE(" << a.key << ", " << a.bound1 << ") failed: ratio " << ratio
                       << " > " << a.bound1 << " (" << count << "/" << totalRuns << ")";
                break;
            case StatAssertType::IN_RANGE:
                pass = ratio >= a.bound1 && ratio <= a.bound2;
                if (!pass)
                    ss << "PROP_STAT_ASSERT_IN_RANGE(" << a.key << ", " << a.bound1 << ", " << a.bound2
                       << ") failed: ratio " << ratio << " not in [" << a.bound1 << ", " << a.bound2 << "] ("
                       << count << "/" << totalRuns << ")";
                break;
        }
        if (!pass) {
            allPassed = false;
            stringstream empty;
            fail(a.filename, a.lineno, ss.str(), empty);
        }
    }
    return allPassed;
}

void PropertyContext::printSummary()
{
    for (const auto& tagKV : tags) {
        auto& key = tagKV.first;
        auto& valueMap = tagKV.second;
        cout << "  " << key << ": " << endl;
        size_t total = 0;
        for (const auto& valueKV : valueMap) {
            auto tag = valueKV.second;
            total += tag.count;
        }

        for (const auto& valueKV : valueMap) {
            auto value = valueKV.first;
            auto tag = valueKV.second;
            cout << "    " << value << ": " << tag.count << "/" << total << " ("
                      << static_cast<double>(tag.count) / total * 100 << "%)" << endl;
        }
    }
}

}  // namespace proptest
