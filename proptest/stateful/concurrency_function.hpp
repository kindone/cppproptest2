#pragma once

#include "proptest/api.hpp"
#include "proptest/stateful/stateful_function.hpp"
#include "proptest/util/assert.hpp"
#include "proptest/Random.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/std/chrono.hpp"
#include <thread>
#include <atomic>

/**
 * @file concurrency_function.hpp
 * @brief Concurrency testing class based on functional style
 */

namespace proptest {

namespace concurrent {

using stateful::Action;
using stateful::EmptyModel;
using stateful::SimpleAction;
using stateful::SimpleActionGen;
using stateful::ActionGen;

using std::atomic;
using std::atomic_bool;
using std::atomic_int;
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
        log.push_back(FRONT_THREAD_ID);
        counter++;
    }

    void initRear(const vector<string>& rear) {
        rears.emplace_back(rear);
        // logging start/end of action
        for(size_t j = 0; j < rear.size() * 2; j++)
            log.push_back(UNINITIALIZED_THREAD_ID);
    }

    void markActionStart(int threadId) {
        log[counter++] = threadId;
    }

    void markActionEnd(int threadId) {
        log[counter++] = threadId;
    }

    void print(ostream& os) const {
        int count = counter;
        os << "count: " << count << ", order: ";
        int frontItr = 0;
        vector<size_t> rearItrs;
        vector<bool> rearStarted;
        size_t numThreads = rears.size();
        for(size_t i = 0; i < numThreads; i++) {
            rearItrs.push_back(0);
            rearStarted.push_back(false);
        }

        for (int i = 0; i < count; i++) {
            int threadId = log[i];

            if(threadId == UNINITIALIZED_THREAD_ID) {
                os << "(UNINITIALIZED) ";
                break;
            }
            // front
            else if(threadId == FRONT_THREAD_ID) {
                os << front[frontItr] << " -> ";
                ++frontItr;
            }
            // rear
            else {
                if(rearStarted[threadId]) {
                    os << "thr" << threadId << " " << rears[threadId][rearItrs[threadId]] << " end -> ";
                    ++rearItrs[threadId];
                }
                else {
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
};

} // namespace concurrent
} // namespace proptest

namespace proptest {
namespace concurrent {

template <typename ObjectType, typename ModelType>
class PROPTEST_API Concurrency {
public:
    using ActionType = Action<ObjectType,ModelType>;
    using ObjectTypeGen = GenFunction<ObjectType>;
    using ModelTypeGen = Function<ModelType(const ObjectType&)>;
    using ActionList = list<ActionType>;
    using ActionGen = GenFunction<ActionType>;

    static constexpr uint32_t defaultNumRuns = 200;
    static constexpr int defaultNumThreads = 2;

    Concurrency(ObjectTypeGen _initialGen, ActionGen _actionGen)
        : initialGen(_initialGen),
          actionGen(_actionGen),
          seed(util::getGlobalSeed()),
          numRuns(defaultNumRuns),
          numThreads(defaultNumThreads),
          maxDurationMs(0)
    {
    }

    Concurrency(ObjectTypeGen _initialGen, ModelTypeGen _modelFactory,
                ActionGen _actionGen)
        : initialGen(_initialGen),
          modelFactory(_modelFactory),
          actionGen(_actionGen),
          seed(util::getGlobalSeed()),
          numRuns(defaultNumRuns),
          numThreads(defaultNumThreads),
          maxDurationMs(0)
    {
    }

    Concurrency& setOnStartup(Function<void()> _onStartup) {
        onStartup = _onStartup;
        return *this;
    }

    Concurrency& setOnCleanup(Function<void()> _onCleanup) {
        onCleanup = _onCleanup;
        return *this;
    }

    template <typename M = ModelType>
        requires(!is_same_v<M, EmptyModel>)
    Concurrency& setPostCheck(Function<void(ObjectType&, ModelType&)> _postCheck)  {
        postCheck = _postCheck;
        return *this;
    }

    template <typename M = ModelType>
        requires(is_same_v<M, EmptyModel>)
    Concurrency&  setPostCheck(Function<void(ObjectType&)> _postCheck)  {
        postCheck = [_postCheck](ObjectType& sys, ModelType&) { _postCheck(sys); };
        return *this;
    }

    bool go();
    bool invoke(Random& rand);
    void handleShrink(Random& savedRand);

    Concurrency& setSeed(uint64_t s)
    {
        seed = s;
        return *this;
    }

    Concurrency& setNumRuns(uint32_t runs)
    {
        numRuns = runs;
        return *this;
    }

    Concurrency& setMaxConcurrency(uint32_t numThr)
    {
        numThreads = numThr;
        return *this;
    }

    Concurrency& setMaxDurationMs(uint32_t durationMs)
    {
        maxDurationMs = durationMs;
        return *this;
    }

    // Retry/timeout knobs aligned with PropertyBase shrink configuration.
    Concurrency& setShrinkMaxRetries(uint32_t retries)
    {
        shrinkMaxRetries = retries;
        return *this;
    }

    Concurrency& setShrinkTimeoutMs(uint32_t ms)
    {
        shrinkTimeoutMs = ms;
        return *this;
    }

    Concurrency& setShrinkRetryTimeoutMs(uint32_t ms)
    {
        shrinkRetryTimeoutMs = ms;
        return *this;
    }

private:
    ObjectTypeGen initialGen;
    ModelTypeGen modelFactory;
    ActionGen actionGen;
    Function<void()> onStartup;
    Function<void()> onCleanup;
    Function<void(ObjectType&, ModelType&)> postCheck;
    uint64_t seed;
    int numRuns;
    int numThreads;
    uint32_t maxDurationMs;
    uint32_t shrinkMaxRetries = 0;
    uint32_t shrinkTimeoutMs = 0;
    uint32_t shrinkRetryTimeoutMs = 0;
};

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::go()
{
    Random rand(seed);
    Random savedRand(seed);
    cout << "random seed: " << seed << endl;
    PropertyContext ctx;
    int i = 0;
    auto startedTime = steady_clock::now();
    try {
        for (; i < numRuns; i++) {
            if(maxDurationMs != 0) {
                auto currentTime = steady_clock::now();
                if(duration_cast<util::milliseconds>(currentTime - startedTime).count() > maxDurationMs)
                {
                    cout << "Timed out after " << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms, passed " << i << " tests" << endl;
                    return true;
                }
            }
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
                    if(onStartup)
                        onStartup();
                    if(invoke(rand) && onCleanup)
                        onCleanup();

                    pass = true;
                } catch (const Success&) {
                    pass = true;
                } catch (const Discard&) {
                    // silently discard combination
                    pass = false;
                }
            } while (!pass);
        }
    } catch (const PropertyFailedBase& e) {
        cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                  << ")" << endl;
        cerr << "    seed: " << seed << endl;
        // shrink
        handleShrink(savedRand);
        return false;
    } catch (const exception& e) {
        cerr << "Falsifiable, after " << (i + 1) << " tests - exception occurred: " << e.what() << endl;
        cerr << "    seed: " << seed << endl;
        // shrink
        handleShrink(savedRand);
        return false;
    }

    cout << "OK, passed " << numRuns << " tests" << endl;

    return true;
}

template <typename ObjectType, typename ModelType>
struct RearRunner
{
    using ActionType = Action<ObjectType,ModelType>;
    using ActionList = list<ActionType>;


    RearRunner(int _num, ObjectType& _obj, ModelType& _model, const ActionList& _actions, atomic_bool& _thread_ready,
               atomic_bool& _sync_ready, ConcurrentTestDump& _dump)
        : num(_num),
          obj(_obj),
          model(_model),
          actions(_actions),
          thread_ready(_thread_ready),
          sync_ready(_sync_ready),
          dump(_dump)
    {
    }

    void operator()()
    {
        thread_ready = true;
        while (!sync_ready) {}

        stateful::Context context{num};

        for (auto action : actions) {
            dump.markActionStart(num);
            action(obj, model, context);
            dump.markActionEnd(num);
        }
    }

    int num;
    ObjectType& obj;
    ModelType& model;
    const ActionList& actions;
    atomic_bool& thread_ready;
    atomic_bool& sync_ready;
    ConcurrentTestDump& dump;
};

template <typename ObjectType, typename ModelType>
bool Concurrency<ObjectType, ModelType>::invoke(Random& rand)
{
    constexpr int UNINITIALIZED_THREAD_ID = -2;
    constexpr int FRONT_THREAD_ID = -1;
    Shrinkable<ObjectType> initialShr = initialGen(rand);

    auto actionListGen = Arbi<list<Action<ObjectType,ModelType>>>(actionGen);
    Shrinkable<ActionList> frontShr = actionListGen(rand);
    vector<Shrinkable<ActionList>> rearShrs;
    rearShrs.reserve(numThreads);
    for (int i = 0; i < numThreads; i++) {
        rearShrs.push_back(actionListGen(rand));
    }

    ObjectType& obj = initialShr.getMutableRef();
    ModelType model = modelFactory ? modelFactory(obj) : ModelType();
    const auto& front = frontShr.getRef();
    vector<string> frontNames;
    util::transform(front.begin(), front.end(), util::back_inserter(frontNames), [](const ActionType& action) {
        return action.name;
    });

    ConcurrentTestDump dump(frontNames);

    // run front
    stateful::Context context{FRONT_THREAD_ID};
    for (auto action : front) {
        action(obj, model, context);
        dump.appendFront();
    }

    // serial execution
    if (numThreads <= 1) {
        if(postCheck)
            postCheck(obj, model);
        return true;
    }

    // run rear
    thread spawner([&]() {
        atomic_bool sync_ready(false);
        vector<shared_ptr<atomic_bool>> thread_ready;
        vector<thread> rearRunners;
        vector<ActionList> rears;

        for (int i = 0; i < numThreads; i++) {
            thread_ready.emplace_back(new atomic_bool(false));
            const auto& rear = rearShrs[i].getRef();
            vector<string> rearNames;
            util::transform(rear.begin(), rear.end(), util::back_inserter(rearNames), [](const ActionType& action) {
                return action.name;
            });
            rears.emplace_back(rear);

            dump.initRear(rearNames);
        }

        // start threads
        for(int i = 0; i < numThreads; i++) {
            rearRunners.emplace_back(RearRunner<ObjectType, ModelType>(i, obj, model, rearShrs[i].getMutableRef(),
                                                                       *thread_ready[i], sync_ready, dump));
        }

        for (int i = 0; i < numThreads; i++) {
            while (!*thread_ready[i]) {}
        }

        sync_ready = true;

        for (int i = 0; i < numThreads; i++) {
            rearRunners[i].join();
        }

        // dump.print(cout);
    });

    spawner.join();

    if(postCheck)
        postCheck(obj, model);
    return true;
}

template <typename ObjectType, typename ModelType>
void Concurrency<ObjectType, ModelType>::handleShrink(Random&)
{
    auto isShrinkPhaseTimedOut = +[](steady_clock::time_point phaseStart, uint32_t timeoutMs) -> bool {
        if (timeoutMs == 0)
            return false;
        auto elapsed = duration_cast<util::milliseconds>(steady_clock::now() - phaseStart).count();
        return elapsed >= timeoutMs;
    };

    auto actionListGen = Arbi<list<Action<ObjectType, ModelType>>>(actionGen);

    // Re-generate the failing tuple from the saved seed.
    Random rand(seed);
    auto initialShr = initialGen(rand);
    auto frontShr = actionListGen(rand);
    vector<Shrinkable<ActionList>> rearShrs;
    rearShrs.reserve(numThreads);
    for (int i = 0; i < numThreads; i++) {
        rearShrs.push_back(actionListGen(rand));
    }

    vector<ShrinkableBase> shrVec;
    vector<ShrinkableBase::StreamType> shrinksVec;
    shrVec.reserve(2 + rearShrs.size());
    shrinksVec.reserve(2 + rearShrs.size());

    shrVec.push_back(initialShr);
    shrinksVec.push_back(initialShr.getShrinks());
    shrVec.push_back(frontShr);
    shrinksVec.push_back(frontShr.getShrinks());
    for (const auto& rearShr : rearShrs) {
        shrVec.push_back(rearShr);
        shrinksVec.push_back(rearShr.getShrinks());
    }

    const auto writeArgs = +[](ostream& os, const vector<ShrinkableBase>& args) {
        os << "{ initial: " << Show<ShrinkableBase, ObjectType>(args[0]);
        if (args.size() > 1)
            os << ", front: " << Show<ShrinkableBase, ActionList>(args[1]);
        for (size_t i = 2; i < args.size(); i++)
            os << ", rear" << (i - 2) << ": " << Show<ShrinkableBase, ActionList>(args[i]);
        os << " }";
    };

    cout << "  with args: ";
    writeArgs(cout, shrVec);
    cout << endl;

    auto runCandidate = [&](const vector<ShrinkableBase>& args) -> pair<bool, string> {
        try {
            if (onStartup)
                onStartup();

            ObjectType obj = args[0].getAny().template getRef<ObjectType>();
            ModelType model = modelFactory ? modelFactory(obj) : ModelType();
            const auto& front = args[1].getAny().template getRef<ActionList>();

            stateful::Context frontCtx{ConcurrentTestDump::FRONT_THREAD_ID};
            for (auto action : front)
                action(obj, model, frontCtx);

            if (numThreads > 1) {
                atomic_bool sync_ready(false);
                vector<shared_ptr<atomic_bool>> thread_ready;
                vector<thread> rearRunners;
                thread_ready.reserve(numThreads);
                rearRunners.reserve(numThreads);
                vector<ActionList> rearCopies;
                rearCopies.reserve(numThreads);
                ConcurrentTestDump dump;
                for (int i = 0; i < numThreads; i++) {
                    thread_ready.emplace_back(new atomic_bool(false));
                    rearCopies.push_back(args[2 + i].getAny().template getRef<ActionList>());
                    vector<string> rearNames;
                    util::transform(
                        rearCopies.back().begin(), rearCopies.back().end(), util::back_inserter(rearNames),
                        [](const ActionType& action) { return action.name; });
                    dump.initRear(rearNames);
                }
                for (int i = 0; i < numThreads; i++) {
                    rearRunners.emplace_back(RearRunner<ObjectType, ModelType>(
                        i, obj, model, rearCopies[i], *thread_ready[i], sync_ready, dump));
                }
                for (int i = 0; i < numThreads; i++) {
                    while (!*thread_ready[i]) {}
                }
                sync_ready = true;
                for (int i = 0; i < numThreads; i++)
                    rearRunners[i].join();
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
    };

    const bool useRetry = (shrinkMaxRetries > 0);
    int64_t candidateTimeoutMs = 0;
    auto shrinkPhaseStart = steady_clock::now();
    bool anyShrinkFound = false;

    auto assessFailureForRetry = [&](vector<ShrinkableBase>& args) {
        int failCount = 0;
        auto start = steady_clock::now();
        for (int r = 0; r < kShrinkAssessmentRuns; r++) {
            auto [failed, _] = runCandidate(args);
            if (failed)
                failCount++;
        }
        auto elapsedMs = duration_cast<util::milliseconds>(steady_clock::now() - start).count();
        double sec = elapsedMs / 1000.0;
        cout << "  reproduction: " << failCount << "/" << kShrinkAssessmentRuns << " in " << std::fixed
             << std::setprecision(2) << sec << "s" << endl;

        if (failCount <= 0 || shrinkRetryTimeoutMs == 0) {
            candidateTimeoutMs = 0;
        } else {
            candidateTimeoutMs =
                static_cast<int64_t>(elapsedMs / failCount * kShrinkAdaptiveMultiplier);
            if (candidateTimeoutMs > shrinkRetryTimeoutMs)
                candidateTimeoutMs = shrinkRetryTimeoutMs;
        }
    };

    auto shrinkTestCandidate = [&](const vector<ShrinkableBase>& curArgs) -> pair<bool, string> {
        if (!useRetry) {
            return runCandidate(curArgs);
        }
        auto candidateStart = steady_clock::now();
        for (uint32_t retry = 0; retry <= shrinkMaxRetries; retry++) {
            if (isShrinkPhaseTimedOut(shrinkPhaseStart, shrinkTimeoutMs))
                break;
            auto [failed, msg] = runCandidate(curArgs);
            if (failed)
                return {true, msg};
            if (candidateTimeoutMs > 0) {
                auto candidateElapsed =
                    duration_cast<util::milliseconds>(steady_clock::now() - candidateStart).count();
                if (candidateElapsed >= candidateTimeoutMs)
                    break;
            }
        }
        return {false, ""};
    };

    if (useRetry)
        assessFailureForRetry(shrVec);

    for (size_t i = 0; i < shrVec.size(); i++) {
        auto shrinks = shrinksVec[i];
        while (!shrinks.isEmpty()) {
            if (isShrinkPhaseTimedOut(shrinkPhaseStart, shrinkTimeoutMs)) {
                cout << "  shrink phase timeout (" << shrinkTimeoutMs << "ms)" << endl;
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
                cout << "  shrinking found simpler failing arg " << i << ": ";
                writeArgs(cout, shrVec);
                cout << endl;
                if (!failureMsg.empty())
                    cout << "    by failed expectation: " << failureMsg << endl;
                if (useRetry && kReassessOnEachSucessfulShrink)
                    assessFailureForRetry(shrVec);
            } else {
                break;
            }
        }
    }

    if (anyShrinkFound) {
        cout << "  simplest args found by shrinking: ";
        writeArgs(cout, shrVec);
        cout << endl;
    }
}

/* without model */
template <typename ObjectType>
decltype(auto) concurrency(GenFunction<ObjectType> initialGen, SimpleActionGen<ObjectType>& actionGen)
{
    auto actionGen2 = actionGen.template map<Action<ObjectType, EmptyModel>>(
        +[](const SimpleAction<ObjectType>& simpleAction) {
            return Action<ObjectType,EmptyModel>(simpleAction);
        });

    return Concurrency<ObjectType, EmptyModel>(initialGen, actionGen2);
}

/* with model */
template <typename ObjectType, typename ModelType>
decltype(auto) concurrency(GenFunction<ObjectType> initialGen, Function<ModelType(const ObjectType&)> modelFactory, ActionGen<ObjectType, ModelType>& actionGen)
{
    return Concurrency<ObjectType, ModelType>(initialGen, modelFactory, actionGen);
}

}  // namespace concurrent
}  // namespace proptest
