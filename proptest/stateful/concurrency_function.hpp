#pragma once

#include "proptest/api.hpp"
#include "proptest/stateful/stateful_function.hpp"
#include "proptest/Random.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/Generator.hpp"
#include <thread>
#include <atomic>

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
    ConcurrentTestDump(const vector<string>& _front) : front(_front) {}

    static constexpr int UNINITIALIZED_THREAD_ID = -2;
    static constexpr int FRONT_THREAD_ID = -1;

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
            // front
            if(threadId == FRONT_THREAD_ID) {
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
                    cout << "Timed out after " << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms , passed " << i << " tests" << endl;
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
               atomic_bool& _sync_ready, vector<int>& _log, atomic_int& _counter)
        : num(_num),
          obj(_obj),
          model(_model),
          actions(_actions),
          thread_ready(_thread_ready),
          sync_ready(_sync_ready),
          log(_log),
          counter(_counter)
    {
    }

    void operator()()
    {
        thread_ready = true;
        while (!sync_ready) {}

        for (auto action : actions) {
            log[counter++] = num; // start
            action(obj, model);
            // cout << "rear2" << endl;
            log[counter++] = num; // end
        }
    }

    int num;
    ObjectType& obj;
    ModelType model;
    const ActionList& actions;
    atomic_bool& thread_ready;
    atomic_bool& sync_ready;
    vector<int>& log;
    atomic_int& counter;
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

    ObjectType obj = initialShr.get();
    ModelType model = modelFactory ? modelFactory(obj) : ModelType();
    auto& front = frontShr.getRef();
    vector<string> frontNames;
    transform(front.begin(), front.end(), util::back_inserter(frontNames), [](const ActionType& action) {
        return action.name;
    });

    atomic<int> counter{0};
    vector<int> log;

    // run front
    for (auto action : front) {
        action(obj, model);
        log.push_back(FRONT_THREAD_ID);
        counter++;
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
            auto& rear = rearShrs[i].getRef();
            rears.emplace_back(rear);

            // logging start/end of action
            for(size_t j = 0; j < rear.size() * 2; j++)
                log.push_back(UNINITIALIZED_THREAD_ID);
        }

        // start threads
        for(int i = 0; i < numThreads; i++) {
            rearRunners.emplace_back(RearRunner<ObjectType, ModelType>(i, obj, model, rearShrs[i].getMutableRef(),
                                                                       *thread_ready[i], sync_ready, log, counter));
        }

        for (int i = 0; i < numThreads; i++) {
            while (!*thread_ready[i]) {}
        }

        sync_ready = true;

        for (int i = 0; i < numThreads; i++) {
            rearRunners[i].join();
        }

        cout << "count: " << counter << ", order: ";
        auto frontItr = front.begin();
        vector<typename ActionList::iterator> rearItrs;
        vector<bool> rearStarted;
        rearItrs.reserve(numThreads);
        rearStarted.reserve(numThreads);

        for(int i = 0; i < numThreads; i++) {
            rearItrs.push_back(rears[i].begin());
            rearStarted.push_back(false);
        }

        for (int i = 0; i < counter; i++) {
            int threadId = log[i];
            // front
            if(threadId == FRONT_THREAD_ID) {
                cout << (*frontItr) << " -> ";
                ++frontItr;
            }
            // rear
            else {
                if(rearStarted[threadId]) {
                    cout << "thr" << threadId << " " << (*rearItrs[threadId]) << " end -> ";
                    ++rearItrs[threadId];
                }
                else {
                    cout << "thr" << threadId << " " << (*rearItrs[threadId]) << " start -> ";
                }
                rearStarted[threadId] = rearStarted[threadId] ? false : true;
            }
        }

        cout << "onCleanup" << endl;
    });

    spawner.join();

    if(postCheck)
        postCheck(obj, model);
    return true;
}

template <typename ObjectType, typename ModelType>
void Concurrency<ObjectType, ModelType>::handleShrink(Random&)
{
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
