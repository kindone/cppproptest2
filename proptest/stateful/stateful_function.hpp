#pragma once

#include "proptest/util/function_traits.hpp"
#include "proptest/stateful/action_gen.hpp"
#include "proptest/stateful/shrink_pipeline.hpp"
#include "proptest/Generator.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/just.hpp"
#include "proptest/combinator/construct.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/std/list.hpp"
#include "proptest/Property.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/std/chrono.hpp"
#include "proptest/std/optional.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/concepts.hpp"
#include <atomic>
#include <exception>
#include <mutex>
#include <thread>

/**
 * @file stateful_function.hpp
 * @brief Stateful testing class based on functional style
 */

namespace proptest {

namespace stateful {

// SimpleActionGen, ActionGen, SimpleActionGenFactory, ActionGenFactory are in action_gen.hpp.

using std::atomic_bool;
using std::atomic;
using std::exception_ptr;
using std::lock_guard;
using std::mutex;
using std::current_exception;
using std::thread;

struct PROPTEST_API ConcurrentTestDump {
    ConcurrentTestDump() {}
    ConcurrentTestDump(const vector<string>& _front) : front(_front) {}

    static constexpr int UNINITIALIZED_THREAD_ID = -2;
    static constexpr int FRONT_THREAD_ID = -1;

    void setFront(const vector<string>& _front) {
        front = _front;
    }

    void appendFront() {
        lock_guard<mutex> guard(mtx);
        log.push_back(FRONT_THREAD_ID);
        counter++;
    }

    void initRear(const vector<string>& rear) {
        lock_guard<mutex> guard(mtx);
        rears.emplace_back(rear);
        for (size_t j = 0; j < rear.size() * 2; j++)
            log.push_back(UNINITIALIZED_THREAD_ID);
    }

    void markActionStart(int threadId) {
        lock_guard<mutex> guard(mtx);
        log[counter++] = threadId;
    }

    void markActionEnd(int threadId) {
        lock_guard<mutex> guard(mtx);
        log[counter++] = threadId;
    }

    void print(ostream& os) const {
        lock_guard<mutex> guard(mtx);
        int count = counter;
        os << "count: " << count << ", order: ";
        int frontItr = 0;
        vector<size_t> rearItrs;
        vector<bool> rearStarted;
        size_t numThreads = rears.size();
        for (size_t i = 0; i < numThreads; i++) {
            rearItrs.push_back(0);
            rearStarted.push_back(false);
        }

        for (int i = 0; i < count; i++) {
            int threadId = log[i];

            if (threadId == UNINITIALIZED_THREAD_ID) {
                os << "(UNINITIALIZED) ";
                break;
            } else if (threadId == FRONT_THREAD_ID) {
                os << front[frontItr] << " -> ";
                ++frontItr;
            } else {
                if (rearStarted[threadId]) {
                    os << "thr" << threadId << " " << rears[threadId][rearItrs[threadId]] << " end -> ";
                    ++rearItrs[threadId];
                } else {
                    os << "thr" << threadId << " " << rears[threadId][rearItrs[threadId]] << " start -> ";
                }
                rearStarted[threadId] = rearStarted[threadId] ? false : true;
            }
        }

        os << "onCleanup" << endl;
    }

    friend ostream& operator<<(ostream& os, const ConcurrentTestDump& dump) {
        dump.print(os);
        return os;
    }

    atomic<int> counter{0};
    vector<int> log;
    vector<string> front;
    vector<vector<string>> rears;
    mutable mutex mtx;
};

template <typename ObjectType, typename ModelType>
struct StatefulArgs {
    StatefulArgs() = default;
    StatefulArgs(list<Action<ObjectType, ModelType>> actions, ObjectType initial)
        : actions(util::move(actions)), initial(util::move(initial))
    {
    }

    list<Action<ObjectType, ModelType>> actions;
    ObjectType initial;
};

template <typename ObjectType, typename ModelType>
inline ostream& show(ostream& os, const StatefulArgs<ObjectType, ModelType>& args)
{
    os << "{ initial: " << Show<ObjectType>(args.initial)
       << ", actions: " << Show<list<Action<ObjectType, ModelType>>>(args.actions) << " }";
    return os;
}

template <typename ObjectType, typename ModelType>
class StatefulProperty {
    using InitialGen = GenFunction<ObjectType>;
    using ModelFactoryFunction = Function<ModelType(const ObjectType&)>;
    using ArgsType = StatefulArgs<ObjectType, ModelType>;
    using ActionType = Action<ObjectType, ModelType>;
    using ActionList = list<ActionType>;

public:
    static constexpr uint32_t defaultNumRuns = 1000;
    static constexpr size_t defaultActionListMinSize = 0;
    static constexpr size_t defaultActionListMaxSize = 20;

    StatefulProperty(InitialGen initGen, ModelFactoryFunction mdlFactory, ActionGen<ObjectType, ModelType>& actGen)
        : initialGen(util::move(initGen)), modelFactory(mdlFactory),
          actionGenFactory([actGen](ObjectType&, ModelType&) { return actGen; })
    {
    }

    StatefulProperty(InitialGen initGen, ModelFactoryFunction mdlFactory, ActionGenFactory<ObjectType, ModelType> actGenFactory)
        : initialGen(util::move(initGen)), modelFactory(mdlFactory), actionGenFactory(util::move(actGenFactory))
    {
    }

    StatefulProperty& setSeed(uint64_t s)
    {
        seed = s;
        return *this;
    }

    StatefulProperty& setNumRuns(uint32_t runs)
    {
        numRuns = runs;
        return *this;
    }

    StatefulProperty& setMaxConcurrency(uint32_t numThreads)
    {
        maxConcurrency = numThreads;
        return *this;
    }

    StatefulProperty& setMaxDurationMs(uint32_t durationMs)
    {
        maxDurationMs = durationMs;
        return *this;
    }

    StatefulProperty& setShrinkMaxRetries(uint32_t retries)
    {
        shrinkMaxRetries = retries;
        return *this;
    }

    StatefulProperty& setShrinkTimeoutMs(uint32_t ms)
    {
        shrinkTimeoutMs = ms;
        return *this;
    }

    StatefulProperty& setShrinkRetryTimeoutMs(uint32_t ms)
    {
        shrinkRetryTimeoutMs = ms;
        return *this;
    }

    StatefulProperty& setOnReproductionStats(Function<void(ReproductionStats)> f)
    {
        onReproductionStats = util::move(f);
        return *this;
    }

    StatefulProperty& setOnFailureReproduction(Function<void(int, const vector<Any>&, const string&)> f)
    {
        onFailureReproduction = util::move(f);
        return *this;
    }

    optional<ReproductionStats> getLastReproductionStats() const { return lastReproductionStats; }

    StatefulProperty& setOnStartup(Function<void()> _onStartup)
    {
        onStartup = _onStartup;
        return *this;
    }

    StatefulProperty& setOnCleanup(Function<void()> _onCleanup)
    {
        onCleanup = _onCleanup;
        return *this;
    }


    template <typename M = ModelType>
        requires(!is_same_v<M, EmptyModel>)
    StatefulProperty& setPostCheck(Function<void(ObjectType&, ModelType&)> _postCheck)
    {
        postCheck = _postCheck;
        return *this;
    }

    template <typename M = ModelType>
        requires(is_same_v<M, EmptyModel>)
    StatefulProperty& setPostCheck(Function<void(ObjectType&)> _postCheck)
    {
        postCheck = [_postCheck](ObjectType& sys, ModelType&) {
            _postCheck(sys);
        };
        return *this;
    }

    StatefulProperty& setOnActionStart(Function<void(ObjectType&, ModelType&)> _onActionStart)
    {
        onActionStart = _onActionStart;
        return *this;
    }

    StatefulProperty& setOnActionEnd(Function<void(ObjectType&, ModelType&)> _onActionEnd)
    {
        onActionEnd = _onActionEnd;
        return *this;
    }

    StatefulProperty& setOutputStream(ostream& os)
    {
        outputStream = &os;
        return *this;
    }

    StatefulProperty& setErrorStream(ostream& os)
    {
        errorStream = &os;
        return *this;
    }

    StatefulProperty& setOutputStreams(ostream& out, ostream& err)
    {
        outputStream = &out;
        errorStream = &err;
        return *this;
    }

    StatefulProperty& setActionListMinSize(size_t minSize)
    {
        actionListMinSize = minSize;
        if (actionListMaxSize < actionListMinSize)
            actionListMaxSize = actionListMinSize;
        return *this;
    }

    StatefulProperty& setActionListMaxSize(size_t maxSize)
    {
        actionListMaxSize = maxSize;
        if (actionListMaxSize < actionListMinSize)
            actionListMinSize = actionListMaxSize;
        return *this;
    }

    StatefulProperty& setActionListSize(size_t size)
    {
        actionListMinSize = size;
        actionListMaxSize = size;
        return *this;
    }

    bool go();

private:
    optional<uint64_t> seed = nullopt;
    optional<uint32_t> numRuns = nullopt;
    optional<uint32_t> maxDurationMs = nullopt;
    optional<uint32_t> shrinkMaxRetries = nullopt;
    optional<uint32_t> shrinkTimeoutMs = nullopt;
    optional<uint32_t> shrinkRetryTimeoutMs = nullopt;
    uint32_t maxConcurrency = 0;
    InitialGen initialGen;
    ModelFactoryFunction modelFactory;
    ActionGenFactory<ObjectType, ModelType> actionGenFactory;
    size_t actionListMinSize = defaultActionListMinSize;
    size_t actionListMaxSize = defaultActionListMaxSize;

    Function<void(ObjectType&, ModelType&)> postCheck;
    Function<void(ObjectType&, ModelType&)> onActionStart;
    Function<void(ObjectType&, ModelType&)> onActionEnd;
    Function<void()> onStartup;
    Function<void()> onCleanup;
    Function<void(ReproductionStats)> onReproductionStats;
    Function<void(int, const vector<Any>&, const string&)> onFailureReproduction;
    ostream* outputStream = &cout;
    ostream* errorStream = &cerr;

    optional<ReproductionStats> lastReproductionStats;

    bool invoke(Random& rand);
    void handleShrink(Random& savedRand);
    void writeArgs(ostream& os, const vector<ShrinkableBase>& args) const;
    pair<bool, string> runCandidate(const vector<ShrinkableBase>& args) const;
    void assessFailureForRetry(vector<ShrinkableBase>& args, int64_t& candidateTimeoutMs, int assessmentIndex);

    Generator<ArgsType> makeStaticArgsGen() const
    {
        auto actionGen = actionGenFactory;
        auto actionListGen = Arbi<list<Action<ObjectType, ModelType>>>(
            Generator<Action<ObjectType, ModelType>>([actionGen](Random& rand) mutable {
                ObjectType obj{};
                ModelType model{};
                return actionGen(obj, model)(rand);
            }),
            actionListMinSize, actionListMaxSize);
        return gen::construct<ArgsType, list<Action<ObjectType, ModelType>>, ObjectType>(actionListGen, initialGen);
    }

    // ── Helpers for makeStateDependentArgsGen ────────────────────────────────
    // genActionShrinkables and applyStatefulShrinkTree are free function templates
    // in shrink_pipeline.hpp (shared with the concurrency pipeline).

    // Step 3 (outer): for a given initial object, build the full args generator.
    // Captures are passed by value so the resulting Generator<ArgsType> outlives
    // the StatefulProperty that created it.
    Generator<ArgsType> makeStateDependentArgsGen() const
    {
        return Generator<ObjectType>(initialGen).template flatMap<ArgsType>(
            [modelFactory = this->modelFactory, actionGenFactory = this->actionGenFactory,
             actionListMinSize = this->actionListMinSize, actionListMaxSize = this->actionListMaxSize](
                ObjectType& initial) -> Generator<ArgsType> {
                return Generator<ArgsType>(Function<Shrinkable<ArgsType>(Random&)>(
                    [initial, modelFactory, actionGenFactory, actionListMinSize, actionListMaxSize](
                        Random& rand) mutable -> Shrinkable<ArgsType> {
                        ObjectType obj = initial;
                        auto model = modelFactory(obj);
                        const size_t numActions = rand.getRandomSize(actionListMinSize, actionListMaxSize + 1);
                        auto actionShrinkables =
                            genActionShrinkables(rand, obj, model, actionGenFactory, numActions);
                        auto actionListShr = applyStatefulShrinkTree(
                            actionShrinkables, actionListMinSize,
                            initial, modelFactory, actionGenFactory);
                        return actionListShr.template map<ArgsType>(
                            [initial](list<Action<ObjectType, ModelType>>& actions) {
                                return ArgsType(actions, initial);
                            });
                    }));
            });
    }
};

template <typename ObjectType, typename ModelType>
struct StatefulRearRunner
{
    using ActionType = Action<ObjectType, ModelType>;
    using ActionList = list<ActionType>;

    StatefulRearRunner(int _num, ObjectType& _obj, ModelType& _model, const ActionList& _actions,
                       atomic_bool& _thread_ready, atomic_bool& _sync_ready, ConcurrentTestDump& _dump,
                       Function<void(ObjectType&, ModelType&)> _onActionStart,
                       Function<void(ObjectType&, ModelType&)> _onActionEnd,
                       shared_ptr<exception_ptr> _firstException,
                       shared_ptr<mutex> _exceptionMutex)
        : num(_num),
          obj(_obj),
          model(_model),
          actions(_actions),
          thread_ready(_thread_ready),
          sync_ready(_sync_ready),
          dump(_dump),
          onActionStart(_onActionStart),
          onActionEnd(_onActionEnd),
          firstException(_firstException),
          exceptionMutex(_exceptionMutex)
    {
    }

    void operator()()
    {
        thread_ready = true;
        while (!sync_ready) {}

        try {
            Context context{num};
            for (auto action : actions) {
                dump.markActionStart(num);
                if (onActionStart)
                    onActionStart(obj, model);
                action(obj, model, context);
                if (onActionEnd)
                    onActionEnd(obj, model);
                dump.markActionEnd(num);
            }
        } catch (...) {
            lock_guard<mutex> guard(*exceptionMutex);
            if (!*firstException)
                *firstException = current_exception();
        }
    }

    int num;
    ObjectType& obj;
    ModelType& model;
    const ActionList& actions;
    atomic_bool& thread_ready;
    atomic_bool& sync_ready;
    ConcurrentTestDump& dump;
    Function<void(ObjectType&, ModelType&)> onActionStart;
    Function<void(ObjectType&, ModelType&)> onActionEnd;
    shared_ptr<exception_ptr> firstException;
    shared_ptr<mutex> exceptionMutex;
};

template <typename ObjectType, typename ModelType>
bool StatefulProperty<ObjectType, ModelType>::go()
{
    const uint64_t effectiveSeed = seed.value_or(util::getGlobalSeed());
    const uint32_t effectiveNumRuns = numRuns.value_or(defaultNumRuns);
    const uint32_t effectiveMaxDurationMs = maxDurationMs.value_or(0);

    Random rand(effectiveSeed);
    Random savedRand(effectiveSeed);
    *outputStream << "random seed: " << effectiveSeed << endl;
    auto startedTime = steady_clock::now();

    uint32_t i = 0;
    try {
        for (; i < effectiveNumRuns; i++) {
            if (effectiveMaxDurationMs != 0) {
                auto currentTime = steady_clock::now();
                auto elapsed = duration_cast<util::milliseconds>(currentTime - startedTime).count();
                if (elapsed > effectiveMaxDurationMs) {
                    *outputStream << "Timed out after " << elapsed << "ms, passed " << i << " tests" << endl;
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
                    if (invoke(rand) && onCleanup)
                        onCleanup();
                } catch (const Success&) {
                    pass = true;
                } catch (const Discard&) {
                    pass = false;
                }
            } while (!pass);
        }
    } catch (const PropertyFailedBase& e) {
        *errorStream << "Falsifiable, after " << (i + 1) << " tests: " << e.what()
                     << " (" << e.filename << ":" << e.lineno << ")" << endl;
        *errorStream << "    seed: " << effectiveSeed << endl;
        handleShrink(savedRand);
        return false;
    } catch (const exception& e) {
        *errorStream << "Falsifiable, after " << (i + 1)
                     << " tests - exception occurred: " << e.what() << endl;
        *errorStream << "    seed: " << effectiveSeed << endl;
        handleShrink(savedRand);
        return false;
    }

    *outputStream << "OK, passed " << effectiveNumRuns << " tests" << endl;
    return true;
}

template <typename ObjectType, typename ModelType>
bool StatefulProperty<ObjectType, ModelType>::invoke(Random& rand)
{
    Shrinkable<ObjectType> initialShr = initialGen(rand);
    ObjectType& obj = initialShr.getMutableRef();
    ModelType model = modelFactory(obj);

    ConcurrentTestDump dump;
    vector<ActionList> rearLists;
    rearLists.reserve(maxConcurrency);

    const size_t frontSize = rand.getRandomSize(actionListMinSize, actionListMaxSize + 1);
    vector<string> frontNames;
    frontNames.reserve(frontSize);
    Context frontCtx{ConcurrentTestDump::FRONT_THREAD_ID};
    for (size_t i = 0; i < frontSize; i++) {
        auto nextActionGen = actionGenFactory(obj, model);
        auto actionShr = nextActionGen(rand);
        const auto& action = actionShr.getRef();
        frontNames.push_back(action.name);
        if (onActionStart)
            onActionStart(obj, model);
        action(obj, model, frontCtx);
        if (onActionEnd)
            onActionEnd(obj, model);
        dump.appendFront();
    }
    dump.setFront(frontNames);

    for (uint32_t t = 0; t < maxConcurrency; t++) {
        ObjectType simObj = obj;
        ModelType simModel = model;
        const size_t rearSize = rand.getRandomSize(actionListMinSize, actionListMaxSize + 1);
        ActionList rearList;
        Context dummyCtx{-99};
        bool simFailed = false;
        for (size_t i = 0; i < rearSize && !simFailed; i++) {
            auto nextActionGen = actionGenFactory(simObj, simModel);
            auto actionShr = nextActionGen(rand);
            const auto& action = actionShr.getRef();
            rearList.push_back(action);
            try {
                action(simObj, simModel, dummyCtx);
            } catch (...) {
                simFailed = true;
            }
        }
        rearLists.push_back(util::move(rearList));
    }

    if (maxConcurrency <= 1) {
        if (postCheck)
            postCheck(obj, model);
        return true;
    }

    atomic_bool sync_ready(false);
    vector<shared_ptr<atomic_bool>> thread_ready;
    vector<thread> rearRunners;
    auto firstException = util::make_shared<exception_ptr>();
    auto exceptionMutex = util::make_shared<mutex>();
    thread_ready.reserve(maxConcurrency);
    rearRunners.reserve(maxConcurrency);

    for (uint32_t i = 0; i < maxConcurrency; i++) {
        thread_ready.emplace_back(new atomic_bool(false));
        vector<string> rearNames;
        util::transform(rearLists[i].begin(), rearLists[i].end(), util::back_inserter(rearNames),
                        [](const ActionType& action) { return action.name; });
        dump.initRear(rearNames);
    }

    for (uint32_t i = 0; i < maxConcurrency; i++) {
        rearRunners.emplace_back(StatefulRearRunner<ObjectType, ModelType>(
            static_cast<int>(i), obj, model, rearLists[i], *thread_ready[i], sync_ready, dump,
            onActionStart, onActionEnd, firstException, exceptionMutex));
    }

    for (uint32_t i = 0; i < maxConcurrency; i++)
        while (!*thread_ready[i]) {}
    sync_ready = true;
    for (uint32_t i = 0; i < maxConcurrency; i++)
        rearRunners[i].join();
    if (*firstException)
        std::rethrow_exception(*firstException);

    if (postCheck)
        postCheck(obj, model);
    return true;
}

template <typename ObjectType, typename ModelType>
void StatefulProperty<ObjectType, ModelType>::writeArgs(ostream& os, const vector<ShrinkableBase>& args) const
{
    os << "{ initial: " << Show<ShrinkableBase, ObjectType>(args[0]);
    if (args.size() > 1) {
        if (maxConcurrency == 0)
            os << ", actions: " << Show<ShrinkableBase, ActionList>(args[1]);
        else
            os << ", front: " << Show<ShrinkableBase, ActionList>(args[1]);
    }
    for (size_t i = 2; i < args.size(); i++)
        os << ", rear" << (i - 2) << ": " << Show<ShrinkableBase, ActionList>(args[i]);
    os << " }";
}

template <typename ObjectType, typename ModelType>
pair<bool, string> StatefulProperty<ObjectType, ModelType>::runCandidate(const vector<ShrinkableBase>& args) const
{
    try {
        if (onStartup)
            onStartup();

        ObjectType obj = args[0].getAny().template getRef<ObjectType>();
        ModelType model = modelFactory(obj);
        const auto& front = args[1].getAny().template getRef<ActionList>();

        Context frontCtx{ConcurrentTestDump::FRONT_THREAD_ID};
        for (auto action : front) {
            if (onActionStart)
                onActionStart(obj, model);
            action(obj, model, frontCtx);
            if (onActionEnd)
                onActionEnd(obj, model);
        }

        const int effectiveThreads = static_cast<int>(args.size()) - 2;
        if (effectiveThreads > 0) {
            atomic_bool sync_ready(false);
            vector<shared_ptr<atomic_bool>> thread_ready;
            vector<thread> rearRunners;
            vector<ActionList> rearCopies;
            ConcurrentTestDump dump;
            auto firstException = util::make_shared<exception_ptr>();
            auto exceptionMutex = util::make_shared<mutex>();
            thread_ready.reserve(effectiveThreads);
            rearRunners.reserve(effectiveThreads);
            rearCopies.reserve(effectiveThreads);

            for (int i = 0; i < effectiveThreads; i++) {
                thread_ready.emplace_back(new atomic_bool(false));
                rearCopies.push_back(args[2 + i].getAny().template getRef<ActionList>());
                vector<string> rearNames;
                util::transform(rearCopies.back().begin(), rearCopies.back().end(),
                                util::back_inserter(rearNames),
                                [](const ActionType& action) { return action.name; });
                dump.initRear(rearNames);
            }
            for (int i = 0; i < effectiveThreads; i++) {
                rearRunners.emplace_back(StatefulRearRunner<ObjectType, ModelType>(
                    i, obj, model, rearCopies[i], *thread_ready[i], sync_ready, dump,
                    onActionStart, onActionEnd, firstException, exceptionMutex));
            }
            for (int i = 0; i < effectiveThreads; i++)
                while (!*thread_ready[i]) {}
            sync_ready = true;
            for (int i = 0; i < effectiveThreads; i++)
                rearRunners[i].join();
            if (*firstException)
                std::rethrow_exception(*firstException);
        }

        if (postCheck)
            postCheck(obj, model);
        if (onCleanup)
            onCleanup();
        return {false, ""};
    } catch (const AssertFailed& e) {
        return {true, string(e.what()) + " (" + e.filename + ":" + to_string(e.lineno) + ")"};
    } catch (const PropertyFailedBase& e) {
        return {true, string(e.what()) + " (" + e.filename + ":" + to_string(e.lineno) + ")"};
    } catch (const exception& e) {
        return {true, string("exception: ") + e.what()};
    }
}

template <typename ObjectType, typename ModelType>
void StatefulProperty<ObjectType, ModelType>::assessFailureForRetry(
    vector<ShrinkableBase>& args, int64_t& candidateTimeoutMs, int assessmentIndex)
{
    int failCount = 0;
    auto start = steady_clock::now();

    vector<Any> argsVec;
    argsVec.reserve(args.size());
    for (const auto& arg : args)
        argsVec.push_back(arg.getAny());

    for (int r = 0; r < kShrinkAssessmentRuns; r++) {
        auto [failed, msg] = runCandidate(args);
        if (failed) {
            failCount++;
            if (onFailureReproduction)
                onFailureReproduction(assessmentIndex, argsVec, msg);
        }
    }

    auto elapsedMs = duration_cast<util::milliseconds>(steady_clock::now() - start).count();
    double sec = elapsedMs / 1000.0;

    stringstream argsSs;
    writeArgs(argsSs, args);
    ReproductionStats stats{failCount, kShrinkAssessmentRuns, sec, argsSs.str()};
    lastReproductionStats = stats;
    if (onReproductionStats)
        onReproductionStats(stats);

    *outputStream << "  reproduction: " << failCount << "/" << kShrinkAssessmentRuns
                  << " in " << std::fixed << std::setprecision(2) << sec << "s" << endl;

    const uint32_t effectiveShrinkRetryTimeoutMs = shrinkRetryTimeoutMs.value_or(0);
    if (failCount <= 0 || effectiveShrinkRetryTimeoutMs == 0) {
        candidateTimeoutMs = 0;
    } else {
        candidateTimeoutMs = static_cast<int64_t>(elapsedMs / failCount * kShrinkAdaptiveMultiplier);
        if (candidateTimeoutMs > effectiveShrinkRetryTimeoutMs)
            candidateTimeoutMs = effectiveShrinkRetryTimeoutMs;
    }
}

template <typename ObjectType, typename ModelType>
void StatefulProperty<ObjectType, ModelType>::handleShrink(Random& savedRand)
{
    auto isShrinkPhaseTimedOut = +[](steady_clock::time_point phaseStart, uint32_t timeoutMs) -> bool {
        if (timeoutMs == 0)
            return false;
        auto elapsed = duration_cast<util::milliseconds>(steady_clock::now() - phaseStart).count();
        return elapsed >= timeoutMs;
    };

    Random rand(savedRand);
    auto initialShr = initialGen(rand);

    ObjectType initialObj = initialShr.getRef();
    ModelType initialModel = modelFactory(initialObj);

    ObjectType frontObj = initialObj;
    ModelType frontModel = initialModel;
    const size_t frontSize = rand.getRandomSize(actionListMinSize, actionListMaxSize + 1);
    auto frontActionShrinkables = genActionShrinkables(
        rand, frontObj, frontModel, actionGenFactory, frontSize);

    auto wrappedFront = applyStatefulShrinkTree(
        frontActionShrinkables, actionListMinSize,
        initialObj, modelFactory, actionGenFactory);

    struct RearShrinkables {
        Shrinkable<ActionList> wrapped;
    };
    vector<RearShrinkables> rearShrinkablesList;
    rearShrinkablesList.reserve(maxConcurrency);

    for (uint32_t t = 0; t < maxConcurrency; t++) {
        ObjectType rearObj = frontObj;
        ModelType rearModel = frontModel;
        const size_t rearSize = rand.getRandomSize(actionListMinSize, actionListMaxSize + 1);
        auto rearActionShrinkables = genActionShrinkables(
            rand, rearObj, rearModel, actionGenFactory, rearSize);

        ObjectType postFrontObjCapture = frontObj;
        ModelType postFrontModelCapture = frontModel;
        Function<ModelType(const ObjectType&)> rearModelFactory =
            [postFrontModelCapture](const ObjectType&) { return postFrontModelCapture; };

        auto wrappedRear = applyStatefulShrinkTree(
            rearActionShrinkables, actionListMinSize,
            postFrontObjCapture, rearModelFactory, actionGenFactory);

        rearShrinkablesList.push_back({util::move(wrappedRear)});
    }

    vector<ShrinkableBase> shrVec;
    vector<ShrinkableBase::StreamType> shrinksVec;
    shrVec.reserve(2 + rearShrinkablesList.size());
    shrinksVec.reserve(2 + rearShrinkablesList.size());

    shrVec.push_back(initialShr);
    shrinksVec.push_back(initialShr.getShrinks());
    shrVec.push_back(wrappedFront);
    shrinksVec.push_back(wrappedFront.getShrinks());

    for (auto& rs : rearShrinkablesList) {
        shrVec.push_back(rs.wrapped);
        shrinksVec.push_back(rs.wrapped.getShrinks());
    }

    *outputStream << "  with args: ";
    writeArgs(*outputStream, shrVec);
    *outputStream << endl;

    const bool useRetry = shrinkMaxRetries.value_or(0) > 0;
    int64_t candidateTimeoutMs = 0;
    auto shrinkPhaseStart = steady_clock::now();
    bool anyShrinkFound = false;
    int assessmentIndex = 0;

    auto shrinkTestCandidate = [&](const vector<ShrinkableBase>& curArgs) -> pair<bool, string> {
        if (!useRetry) {
            return runCandidate(curArgs);
        }
        auto candidateStart = steady_clock::now();
        for (uint32_t retry = 0; retry <= shrinkMaxRetries.value_or(0); retry++) {
            if (isShrinkPhaseTimedOut(shrinkPhaseStart, shrinkTimeoutMs.value_or(0)))
                break;
            auto [failed, msg] = runCandidate(curArgs);
            if (failed)
                return {true, msg};
            if (candidateTimeoutMs > 0) {
                auto elapsed = duration_cast<util::milliseconds>(steady_clock::now() - candidateStart).count();
                if (elapsed >= candidateTimeoutMs)
                    break;
            }
        }
        return {false, ""};
    };

    if (useRetry)
        assessFailureForRetry(shrVec, candidateTimeoutMs, assessmentIndex++);

    if (maxConcurrency > 0) {
        const int originalNumRears = static_cast<int>(shrVec.size()) - 2;
        for (int tryRears = 0; tryRears < originalNumRears; tryRears++) {
            if (isShrinkPhaseTimedOut(shrinkPhaseStart, shrinkTimeoutMs.value_or(0))) {
                *outputStream << "  shrink phase timeout (" << shrinkTimeoutMs.value_or(0) << "ms)" << endl;
                break;
            }
            vector<ShrinkableBase> reduced(shrVec.begin(), shrVec.begin() + 2 + tryRears);
            auto [failed, msg] = shrinkTestCandidate(reduced);
            if (failed) {
                while (shrinksVec.size() > reduced.size())
                    shrinksVec.pop_back();
                shrVec = reduced;
                anyShrinkFound = true;
                *outputStream << "  shrinking found simpler (fewer threads, " << tryRears << " rear(s)): ";
                writeArgs(*outputStream, shrVec);
                *outputStream << endl;
                if (!msg.empty())
                    *outputStream << "    by failed expectation: " << msg << endl;
                if (useRetry && kReassessOnEachSucessfulShrink)
                    assessFailureForRetry(shrVec, candidateTimeoutMs, assessmentIndex++);
                break;
            }
        }
    }

    for (size_t i = 0; i < shrVec.size(); i++) {
        auto shrinks = shrinksVec[i];
        while (!shrinks.isEmpty()) {
            if (isShrinkPhaseTimedOut(shrinkPhaseStart, shrinkTimeoutMs.value_or(0))) {
                *outputStream << "  shrink phase timeout (" << shrinkTimeoutMs.value_or(0) << "ms)" << endl;
                break;
            }

            auto iter = shrinks.iterator<ShrinkableBase::StreamElementType>();
            bool shrinkFound = false;
            string failureMsg;
            while (iter.hasNext()) {
                auto next = iter.next();
                auto cur = shrVec;
                cur[i] = next;
                auto [failed, msg] = shrinkTestCandidate(cur);
                if (failed)
                    failureMsg = msg;
                if (failed) {
                    shrVec[i] = next;
                    shrinks = next.getShrinks();
                    shrinkFound = true;
                    break;
                }
            }

            if (shrinkFound) {
                anyShrinkFound = true;
                *outputStream << "  shrinking found simpler failing arg " << i << ": ";
                writeArgs(*outputStream, shrVec);
                *outputStream << endl;
                if (!failureMsg.empty())
                    *outputStream << "    by failed expectation: " << failureMsg << endl;
                if (useRetry && kReassessOnEachSucessfulShrink)
                    assessFailureForRetry(shrVec, candidateTimeoutMs, assessmentIndex++);
            } else {
                break;
            }
        }
    }

    if (anyShrinkFound) {
        *outputStream << "  simplest args found by shrinking: ";
        writeArgs(*outputStream, shrVec);
        *outputStream << endl;
    }
}

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, SimpleActionGen<ObjectType>& simpleActionGen)
{
    static EmptyModel emptyModel;
    auto actionGen = simpleActionGen.template map<Action<ObjectType, EmptyModel>>(
        [](const SimpleAction<ObjectType>& simpleAction) { return Action<ObjectType, EmptyModel>(simpleAction); });

    auto modelFactory = +[](const ObjectType&) { return emptyModel; };
    return StatefulProperty<ObjectType, EmptyModel>(util::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, SimpleActionGenFactory<ObjectType> simpleActionGenFactory)
{
    static EmptyModel emptyModel;
    ActionGenFactory<ObjectType, EmptyModel> actionGenFactory =
        [simpleActionGenFactory](ObjectType& obj, EmptyModel&) {
            return simpleActionGenFactory(obj).template map<Action<ObjectType, EmptyModel>>(
                [](const SimpleAction<ObjectType>& simpleAction) {
                    return Action<ObjectType, EmptyModel>(simpleAction);
                });
        };

    auto modelFactory = +[](const ObjectType&) { return emptyModel; };
    return StatefulProperty<ObjectType, EmptyModel>(
        util::forward<InitialGen>(initialGen), modelFactory, util::move(actionGenFactory));
}

template <typename ObjectType, typename InitialGen, typename Factory>
    requires std::invocable<Factory, ObjectType&> &&
             std::constructible_from<SimpleActionGen<ObjectType>, std::invoke_result_t<Factory, ObjectType&>>
decltype(auto) statefulProperty(InitialGen&& initialGen, Factory&& simpleActionGenFactory)
{
    return statefulProperty<ObjectType>(
        util::forward<InitialGen>(initialGen),
        SimpleActionGenFactory<ObjectType>(util::forward<Factory>(simpleActionGenFactory)));
}

template <typename ObjectType, typename InitialGen, typename Factory>
    requires std::invocable<Factory, ObjectType&, EmptyModel&> &&
             std::constructible_from<ActionGen<ObjectType, EmptyModel>,
                                     std::invoke_result_t<Factory, ObjectType&, EmptyModel&>>
decltype(auto) statefulProperty(InitialGen&& initialGen, Factory&& actionGenFactory)
{
    return statefulProperty<ObjectType, EmptyModel>(
        util::forward<InitialGen>(initialGen),
        Function<EmptyModel(const ObjectType&)>([](const ObjectType&) { return EmptyModel{}; }),
        ActionGenFactory<ObjectType, EmptyModel>(util::forward<Factory>(actionGenFactory)));
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(const ObjectType&)> modelFactory,
                                ActionGen<ObjectType, ModelType>& actionGen)
{
    return StatefulProperty<ObjectType, ModelType>(util::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(const ObjectType&)> modelFactory,
                                ActionGenFactory<ObjectType, ModelType> actionGenFactory)
{
    return StatefulProperty<ObjectType, ModelType>(
        util::forward<InitialGen>(initialGen), modelFactory, util::move(actionGenFactory));
}

template <typename ObjectType, typename ModelType, typename InitialGen, typename Factory>
    requires std::invocable<Factory, ObjectType&, ModelType&> &&
             std::constructible_from<ActionGen<ObjectType, ModelType>,
                                     std::invoke_result_t<Factory, ObjectType&, ModelType&>>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(const ObjectType&)> modelFactory,
                                Factory&& actionGenFactory)
{
    return statefulProperty<ObjectType, ModelType>(
        util::forward<InitialGen>(initialGen), modelFactory,
        ActionGenFactory<ObjectType, ModelType>(util::forward<Factory>(actionGenFactory)));
}

}  // namespace stateful
}  // namespace proptest
