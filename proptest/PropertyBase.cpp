#include "proptest/PropertyBase.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/std/chrono.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/Random.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/Shrinkable.hpp"

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
    const uint64_t effectiveSeed = seed.value_or(util::getGlobalSeed());
    const uint32_t effectiveNumRuns = numRuns.value_or(defaultNumRuns);
    const uint32_t effectiveMaxDurationMs = maxDurationMs.value_or(defaultMaxDurationMs);

    Random rand(effectiveSeed);
    Random savedRand(effectiveSeed);
    cout << "random seed: " << effectiveSeed << endl;
    PropertyContext ctx;
    auto startedTime = steady_clock::now();

    size_t i = 0;
    try {
        for (; i < effectiveNumRuns; i++) {
            if (effectiveMaxDurationMs != 0) {
                auto currentTime = steady_clock::now();
                if (duration_cast<util::milliseconds>(currentTime - startedTime).count() > effectiveMaxDurationMs)
                {
                    cout << "Timed out after "
                         << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms , passed "
                         << i << " tests" << endl;
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

    cout << "OK, passed " << effectiveNumRuns << " tests" << endl;
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

bool PropertyBase::isShrinkPhaseTimedOut(steady_clock::time_point phaseStart, uint32_t timeoutMs) const
{
    if (timeoutMs == 0)
        return false;
    auto elapsed = duration_cast<util::milliseconds>(steady_clock::now() - phaseStart).count();
    return elapsed >= timeoutMs;
}

void PropertyBase::assessFailureForRetry(vector<ShrinkableBase>& shrVec,
    int64_t& candidateTimeoutMs, int assessmentIndex)
{
    int failCount = 0;
    auto start = steady_clock::now();

    vector<Any> argsVec;
    argsVec.reserve(shrVec.size());
    for (const auto& s : shrVec)
        argsVec.push_back(s.getAny());

    for (int r = 0; r < kShrinkAssessmentRuns; r++) {
        PropertyContext ctx;
        bool failed = !test(shrVec) || ctx.hasFailures();
        if (failed) {
            failCount++;
            if (onFailureReproduction) {
                string errMsg = ctx.flushFailures().str();
                onFailureReproduction(assessmentIndex, argsVec, errMsg);
            }
        }
    }

    auto elapsedMs = duration_cast<util::milliseconds>(steady_clock::now() - start).count();
    double sec = elapsedMs / 1000.0;

    stringstream argsSs;
    writeArgs(argsSs, shrVec);
    ReproductionStats stats{failCount, kShrinkAssessmentRuns, sec, argsSs.str()};
    lastReproductionStats = stats;

    if (onReproductionStats)
        onReproductionStats(stats);

    cout << "  reproduction: " << failCount << "/" << kShrinkAssessmentRuns << " in " << std::fixed
         << std::setprecision(2) << sec << "s" << endl;

    const uint32_t effectiveShrinkRetryTimeoutMs = shrinkRetryTimeoutMs.value_or(0);
    if (failCount <= 0 || effectiveShrinkRetryTimeoutMs == 0)
        candidateTimeoutMs = 0;
    else {
        candidateTimeoutMs = static_cast<int64_t>(elapsedMs / failCount * kShrinkAdaptiveMultiplier);
        if (candidateTimeoutMs > effectiveShrinkRetryTimeoutMs)
            candidateTimeoutMs = effectiveShrinkRetryTimeoutMs;
    }
}

pair<bool, string> PropertyBase::shrinkTestCandidate(const vector<ShrinkableBase>& curShrVec, bool useRetry,
    uint32_t maxRetries, uint32_t phaseTimeoutMs, int64_t candidateTimeoutMs,
    steady_clock::time_point phaseStart) const
{
    auto* self = const_cast<PropertyBase*>(this);
    if (!useRetry || maxRetries == 0) {
        // Deterministic: single run per candidate
        PropertyContext ctx;
        if (!self->test(curShrVec) || ctx.hasFailures())
            return {true, ctx.flushFailures(4).str()};
        return {false, ""};
    }
    // Retry mode: run until failure or limits
    auto candidateStart = steady_clock::now();
    for (uint32_t retry = 0; retry <= maxRetries; retry++) {
        if (isShrinkPhaseTimedOut(phaseStart, phaseTimeoutMs))
            break;
        PropertyContext retryCtx;
        if (!self->test(curShrVec) || retryCtx.hasFailures())
            return {true, retryCtx.flushFailures(4).str()};
        if (candidateTimeoutMs > 0) {
            auto candidateElapsed =
                duration_cast<util::milliseconds>(steady_clock::now() - candidateStart).count();
            if (candidateElapsed >= candidateTimeoutMs)
                break;
        }
    }
    return {false, ""};
}

void PropertyBase::shrink(Random& savedRand, const GenVec& curGenVec)
{
    // Regenerate failed value tuple from saved seed
    const size_t Arity = curGenVec.size();
    vector<ShrinkableBase> shrVec;
    vector<ShrinkableBase::StreamType> shrinksVec;
    shrVec.reserve(Arity);
    shrinksVec.reserve(Arity);
    for (size_t i = 0; i < Arity; i++) {
        auto shr = curGenVec[i](savedRand);
        shrVec.push_back(shr);
        shrinksVec.push_back(shr.getShrinks());
    }

    cout << "  with args: " << ShowShrVec{*this, shrVec} << endl;

    const uint32_t effectiveShrinkMaxRetries = shrinkMaxRetries.value_or(0);
    const uint32_t effectiveShrinkTimeoutMs = shrinkTimeoutMs.value_or(0);
    const bool useRetry = (effectiveShrinkMaxRetries > 0);
    int64_t candidateTimeoutMs = 0;
    auto shrinkPhaseStart = steady_clock::now();
    int assessmentIndex = 0;

    // Initial assessment: measure reproduction rate for adaptive retry budget
    if (useRetry)
        assessFailureForRetry(shrVec, candidateTimeoutMs, assessmentIndex++);

    // Try to shrink each argument in turn
    for (size_t i = 0; i < Arity; i++) {
        auto shrinks = shrinksVec[i];
        while (!shrinks.isEmpty()) {
            if (isShrinkPhaseTimedOut(shrinkPhaseStart, effectiveShrinkTimeoutMs)) {
                cout << "  shrink phase timeout (" << effectiveShrinkTimeoutMs << "ms)" << endl;
                break;
            }

            auto iter = shrinks.iterator<ShrinkableBase::StreamElementType>();
            bool shrinkFound = false;
            string failureMsg;
            while (iter.hasNext()) {
                auto next = iter.next();
                vector<ShrinkableBase> curShrVec = shrVec;
                curShrVec[i] = next;

                auto [failed, msg] = shrinkTestCandidate(curShrVec, useRetry, effectiveShrinkMaxRetries,
                    effectiveShrinkTimeoutMs, candidateTimeoutMs, shrinkPhaseStart);
                if (failed)
                    failureMsg = msg;

                if (failed) {
                    shrinks = next.getShrinks();
                    shrVec[i] = next;
                    shrinkFound = true;
                    break;
                }
            }
            if (shrinkFound) {
                cout << "  shrinking found simpler failing arg " << i << ": " << ShowShrVec{*this, shrVec} << endl;
                if (!failureMsg.empty())
                    cout << "    by failed expectation: " << failureMsg << endl;
                if (useRetry && kReassessOnEachSucessfulShrink)
                    assessFailureForRetry(shrVec, candidateTimeoutMs, assessmentIndex++);
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
