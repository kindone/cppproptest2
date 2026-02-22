#include "proptest/PropertyBase.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/std/chrono.hpp"
#include "proptest/Random.hpp"

namespace proptest {

namespace util {

uint64_t getGlobalSeed()
{
    static const char* env_seed = std::getenv("PROPTEST_SEED");
    if (env_seed) {
        return atoll(env_seed);
    } else {
        static uint64_t time = getCurrentTime();
        return time;
    }
}

}  // namespace util

PropertyContext* PropertyBase::context = nullptr;
uint32_t PropertyBase::defaultNumRuns = 1000;
uint32_t PropertyBase::defaultMaxDurationMs = 0;

void PropertyBase::setContext(PropertyContext* ctx)
{
    context = ctx;
}

void PropertyBase::tag(const char* file, int lineno, string key, string value)
{
    if (!context)
        throw runtime_error(__FILE__, __LINE__, "context is not set");

    context->tag(file, lineno, key, value);
}

void PropertyBase::addStatAssertGe(string&& key, double bound, const char* filename, int lineno)
{
    if (context)
        context->addStatAssertGe(util::move(key), bound, filename, lineno);
}

void PropertyBase::addStatAssertLe(string&& key, double bound, const char* filename, int lineno)
{
    if (context)
        context->addStatAssertLe(util::move(key), bound, filename, lineno);
}

void PropertyBase::addStatAssertInRange(string&& key, double minBound, double maxBound, const char* filename, int lineno)
{
    if (context)
        context->addStatAssertInRange(util::move(key), minBound, maxBound, filename, lineno);
}

void PropertyBase::succeed(const char* file, int lineno, const char* condition, const stringstream& str)
{
    if (!context)
        throw runtime_error(__FILE__, __LINE__, "context is not set");

    context->succeed(file, lineno, condition, str);
}

void PropertyBase::fail(const char* file, int lineno, const char* condition, const stringstream& str)
{
    if (!context)
        throw runtime_error(__FILE__, __LINE__, "context is not set");

    context->fail(file, lineno, condition, str);
}

bool PropertyBase::invoke(Random&)
{
    return true;
}

bool PropertyBase::exampleImpl(const vector<Any>& values)
{
    try {
        try {
            try {
                if (onStartup)
                    onStartup();
                bool result = callFunction(values);
                if (onCleanup)
                    onCleanup();
                return result;
            } catch (const AssertFailed& e) {
                throw PropertyFailed(e);
            }
        } catch (const Success&) {
            return true;
        } catch (const Discard&) {
            // silently discard combination
            cerr << "Discard is not supported for single run" << endl;
        }
    } catch (const PropertyFailedBase& e) {
        cerr << "example failed: " << e.what() << " (" << e.filename << ":" << e.lineno << ")" << endl;
        cerr << "  with args: " << ShowAnyVec{*this, values} << endl;
        return false;
    } catch (const exception& e) {
        // skip shrinking?
        cerr << "example failed by exception: " << e.what() << endl;
        cerr << "  with args: " << ShowAnyVec{*this, values} << endl;
        return false;
    }
    return false;
}

bool PropertyBase::runForAll(const GenVec& curGenVec)
{
    Random rand(seed);
    Random savedRand(seed);
    cout << "random seed: " << seed << endl;
    PropertyContext ctx;
    auto startedTime = steady_clock::now();

    size_t i = 0;
    try {
        for (; i < numRuns; i++) {
            if(maxDurationMs != 0) {
                auto currentTime = steady_clock::now();
                if(duration_cast<util::milliseconds>(currentTime - startedTime).count() > maxDurationMs)
                {
                    cout << "Timed out after " << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms , passed " << i << " tests" << endl;
                    if (!ctx.checkStatAssertions(i)) {
                        stringstream failures = ctx.flushFailures();
                        cerr << "Stat assertion failed: " << failures.str() << endl;
                        ctx.printSummary();
                        return false;
                    }
                    ctx.printSummary();
                    return true;
                }
            }
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
                    if (onStartup)
                        onStartup();
                    // generate values
                    bool result = callFunctionFromGen(rand, curGenVec);

                    if (onCleanup)
                        onCleanup();
                    stringstream failures = ctx.flushFailures();
                    // failed expectations
                    if (failures.rdbuf()->in_avail()) {
                        cerr << "Falsifiable, after " << (i + 1) << " tests: ";
                        cerr << failures.str();
                        shrink(savedRand, curGenVec);
                        return false;
                    } else if (!result) {
                        cerr << "Falsifiable, after " << (i + 1) << " tests" << endl;
                        shrink(savedRand, curGenVec);
                        return false;
                    }
                    pass = true;
                } catch (const Success&) {
                    pass = true;
                } catch (const Discard&) {
                    // silently discard combination
                    pass = false;
                }
            } while (!pass);
        }
    } catch (const AssertFailed& e) {
        cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                << ")" << endl;
        // shrink
        shrink(savedRand, curGenVec);
        return false;
    } catch (const PropertyFailedBase& e) {
        cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                << ")" << endl;
        // shrink
        shrink(savedRand, curGenVec);
        return false;
    } catch (const exception& e) {
        cerr << "Falsifiable, after " << (i + 1) << " tests - unhandled exception thrown: " << e.what() << endl;
        // shrink
        shrink(savedRand, curGenVec);
        return false;
    }

    cout << "OK, passed " << numRuns << " tests" << endl;
    if (!ctx.checkStatAssertions(i)) {
        stringstream failures = ctx.flushFailures();
        cerr << "Stat assertion failed: " << failures.str() << endl;
        ctx.printSummary();
        return false;
    }
    ctx.printSummary();
    return true;
}

bool PropertyBase::test(const vector<ShrinkableBase>& curShrVec)
{
    bool result = false;
    try {
        if (onStartup)
            onStartup();

        result = callFunction(curShrVec);

        if (onCleanup)
            onCleanup();
    } catch (const AssertFailed&) {
        result = false;
        // cerr << "    assertion failed: " << e.what() << " (" << e.filename << ":"
        //           << e.lineno << ")" << endl;
    } catch (const exception&) {
        result = false;
    }
    return result;
}

void PropertyBase::shrink(Random& savedRand, const GenVec& curGenVec)
{
    // regenerate failed value tuple
    const size_t Arity = curGenVec.size();
    vector<ShrinkableBase> shrVec;
    vector<ShrinkableBase::StreamType> shrinksVec;
    shrVec.reserve(Arity);
    shrinksVec.reserve(Arity);
    for(size_t i = 0; i < Arity; i++) {
        auto shr = curGenVec[i](savedRand);
        shrVec.push_back(shr);
        shrinksVec.push_back(shr.getShrinks());
    };

    cout << "  with args: " << ShowShrVec{*this, shrVec} << endl;

    for(size_t i = 0; i < Arity; i++) {
        auto shrinks = shrinksVec[i];
        while (!shrinks.isEmpty()) {
            auto iter = shrinks.iterator<ShrinkableBase::StreamElementType>();
            bool shrinkFound = false;
            PropertyContext context;
            // keep trying until failure is reproduced
            while (iter.hasNext()) {
                // get shrinkable
                auto next = iter.next();
                vector<ShrinkableBase> curShrVec = shrVec;
                curShrVec[i] = next;
                if (!test(curShrVec) || context.hasFailures()) {
                    shrinks = next.getShrinks();
                    shrVec[i] = next;
                    shrinkFound = true;
                    break;
                }
            }
            if (shrinkFound) {
                cout << "  shrinking found simpler failing arg " << i << ": " << ShowShrVec{*this, shrVec} << endl;
                if (context.hasFailures())
                    cout << "    by failed expectation: " << context.flushFailures(4).str() << endl;
            } else {
                break;
            }
        }
    }

    cout << "  simplest args found by shrinking: " << ShowShrVec{*this, shrVec} << endl;
}

stringstream& PropertyBase::getLastStream()
{
    if (!context)
        throw runtime_error(__FILE__, __LINE__, "context is not set");

    return context->getLastStream();
}

}  // namespace proptest
