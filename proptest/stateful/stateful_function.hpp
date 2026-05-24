#pragma once

#include "proptest/util/function_traits.hpp"
#include "proptest/stateful/action.hpp"
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
#include "proptest/std/chrono.hpp"
#include "proptest/std/optional.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/concepts.hpp"

/**
 * @file stateful_function.hpp
 * @brief Stateful testing class based on functional style
 */

namespace proptest {

namespace stateful {

// template <typename ObjectType, typename ModelType>
// using ActionListGen = GenFunction<list<Action<ObjectType, ModelType>>>;
template <typename ObjectType>
using SimpleActionGen = Generator<SimpleAction<ObjectType>>;

template <typename ObjectType, typename ModelType>
using ActionGen = Generator<Action<ObjectType, ModelType>>;

template <typename ObjectType>
using SimpleActionGenFactory = Function<SimpleActionGen<ObjectType>(ObjectType&)>;

template <typename ObjectType, typename ModelType>
using ActionGenFactory = Function<ActionGen<ObjectType, ModelType>(ObjectType&, ModelType&)>;

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
    using ModelFactoryFunction = Function<ModelType(ObjectType&)>;
    using ArgsType = StatefulArgs<ObjectType, ModelType>;
    using PropertyType = Property<ArgsType>;
    using Func = Function<bool(ArgsType)>;

public:
    static constexpr size_t defaultActionListMinSize = 0;
    static constexpr size_t defaultActionListMaxSize = 20;

    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGen<ObjectType, ModelType>& actGen)
        : initialGen(initGen), modelFactory(mdlFactory), actionGen(actGen)
    {
    }

    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGenFactory<ObjectType, ModelType> actGenFactory)
        : initialGen(initGen), modelFactory(mdlFactory), actionGenFactory(util::move(actGenFactory))
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

    bool go()
    {
        Generator<ArgsType> argsGen = actionGenFactory ? makeStateDependentArgsGen() : makeStaticArgsGen();
        vector<AnyGenerator> genVec({argsGen});

        auto func = [modelFactory = this->modelFactory, postCheck = this->postCheck,
                     onActionStart = this->onActionStart, onActionEnd = this->onActionEnd](ArgsType args) {
            ObjectType obj = args.initial;
            auto model = modelFactory(obj);
            for (auto action : args.actions) {
                if (onActionStart)
                    onActionStart(obj, model);
                action(obj, model);
                if (onActionEnd)
                    onActionEnd(obj, model);
            }
            if (postCheck)
                postCheck(obj, model);
            return true;
        };

        auto prop = util::make_shared<PropertyType>(func, util::move(genVec));
        if (onStartup)
            prop->setOnStartup(onStartup);
        if (onCleanup)
            prop->setOnCleanup(onCleanup);
        if (seed.has_value())
            prop->setSeed(*seed);
        if (numRuns.has_value())
            prop->setNumRuns(*numRuns);
        if (maxDurationMs.has_value())
            prop->setMaxDurationMs(*maxDurationMs);
        if (shrinkMaxRetries.has_value())
            prop->setShrinkMaxRetries(*shrinkMaxRetries);
        if (shrinkTimeoutMs.has_value())
            prop->setShrinkTimeoutMs(*shrinkTimeoutMs);
        if (shrinkRetryTimeoutMs.has_value())
            prop->setShrinkRetryTimeoutMs(*shrinkRetryTimeoutMs);
        if (onReproductionStats)
            prop->setOnReproductionStats(onReproductionStats);
        if (onFailureReproduction)
            prop->setOnFailureReproduction(onFailureReproduction);
        if (outputStream)
            prop->setOutputStream(*outputStream);
        if (errorStream)
            prop->setErrorStream(*errorStream);
        auto resultProp = prop->forAll();
        lastReproductionStats = resultProp.getLastReproductionStats();
        return static_cast<bool>(resultProp);
    }

private:
    optional<uint64_t> seed = nullopt;
    optional<uint32_t> numRuns = nullopt;
    optional<uint32_t> maxDurationMs = nullopt;
    optional<uint32_t> shrinkMaxRetries = nullopt;
    optional<uint32_t> shrinkTimeoutMs = nullopt;
    optional<uint32_t> shrinkRetryTimeoutMs = nullopt;
    InitialGen initialGen;
    ModelFactoryFunction modelFactory;
    optional<ActionGen<ObjectType, ModelType>> actionGen = nullopt;
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
    ostream* outputStream = nullptr;
    ostream* errorStream = nullptr;

    optional<ReproductionStats> lastReproductionStats;

    Generator<ArgsType> makeStaticArgsGen() const
    {
        auto actionListGen =
            Arbi<list<Action<ObjectType, ModelType>>>(*actionGen, actionListMinSize, actionListMaxSize);
        // Preserve action-list-first shrinking by constructing stateful args
        // with constructor order (actions, initial).
        return gen::construct<ArgsType, list<Action<ObjectType, ModelType>>, ObjectType>(actionListGen, initialGen);
    }

    // ── Helpers for makeStateDependentArgsGen ────────────────────────────────

    // Step 1: Call the factory per slot with live (obj, model), execute each
    // action immediately so the next factory call sees the updated state.
    // Saves a Random bookmark before each generation so shrinking can later
    // regenerate the same action slot from the same random seed with a
    // (prefix-replayed) state.  Elements are Shrinkable<pair<Random,Action>>
    // with the action's full shrink tree transferred via map<PairType>.
    static Shrinkable<vector<ShrinkableBase>> genActionShrinkables(
        Random& rand, ObjectType& obj, ModelType& model,
        const ActionGenFactory<ObjectType, ModelType>& factory, size_t numActions)
    {
        using ActionType = Action<ObjectType, ModelType>;
        using PairType = pair<Random, ActionType>;

        auto actionShrinkables = make_shrinkable<vector<ShrinkableBase>>();
        auto& vec = actionShrinkables.getMutableRef();
        vec.reserve(numActions);
        for (size_t i = 0; i < numActions; ++i) {
            Random bookmark = rand;  // snapshot before generation
            auto nextActionGen = factory(obj, model);
            auto actionShr = nextActionGen(rand);  // advances rand
            // Map action → pair<bookmark,action>, preserving entire shrink tree:
            // every shrunken action becomes pair<same-bookmark, shrunken-action>.
            auto pairShr = actionShr.template map<PairType>(
                [bookmark](const ActionType& action) -> PairType {
                    return PairType(bookmark, action);
                });
            vec.push_back(ShrinkableBase(pairShr));
            actionShr.getRef()(obj, model);  // execute to advance state
        }
        return actionShrinkables;
    }

    // Step 2: Wrap a Shrinkable<vector<ShrinkableBase>> (elements: Shrinkable<pair<Random,Action>>)
    // with four shrink phases and extract the final list<Action>:
    //   Phase 1  — prefix-length-first (shorter sequences before element simplification)
    //   Phase 2  — element-wise via stored pair shrink trees (fast; uses the shrink tree baked
    //              in at generation time, which may be stale for state-dependent factories)
    //   Phase 2b — state-aware bookmark shrinking: for each non-last position replay the
    //              prefix, regenerate the action from its bookmark with the correct state,
    //              and yield shrinks of that fresh generation
    //   Phase 3  — last-action parameter shrinking via the last element's stored shrink tree
    static Shrinkable<list<Action<ObjectType, ModelType>>> applyStatefulShrinkTree(
        Shrinkable<vector<ShrinkableBase>> actionShrinkables,
        size_t minSize,
        const ObjectType& initial,
        const ModelFactoryFunction& modelFactory,
        const ActionGenFactory<ObjectType, ModelType>& actionGenFactory)
    {
        using ActionType = Action<ObjectType, ModelType>;
        using PairType = pair<Random, ActionType>;

        // Phase 1: prefix-length-first shrinking
        auto withPhase1 = shrinkVectorLength(actionShrinkables, minSize);

        // Phase 2: element-wise shrinking via stored pair shrink trees
        auto withPhase2 = shrinkAnyVector(withPhase1, minSize, true, false);

        // Phase 2b: state-aware bookmark-based shrinking for each non-last position.
        // Replays the prefix to reconstruct the live state at position i, regenerates
        // the action from its stored bookmark, and yields shrinks of that fresh tree.
        auto withPhase2b = withPhase2.concat(
            [initial, modelFactory, actionGenFactory, minSize](ShrinkableBase& nodeShr) -> ShrinkableBase::StreamType {
                const auto& vec = nodeShr.getRef<vector<ShrinkableBase>>();
                // Only meaningful when there is at least one non-last element
                if (vec.size() <= 1)
                    return ShrinkableBase::StreamType::empty();

                Stream result = Stream::empty();
                for (size_t i = 0; i + 1 < vec.size(); i++) {
                    // Replay prefix [0, i) to obtain the correct state at position i
                    ObjectType simObj = initial;
                    ModelType simModel = modelFactory(simObj);
                    bool replayFailed = false;
                    for (size_t j = 0; j < i && !replayFailed; j++) {
                        try {
                            vec[j].getAny().getRef<PairType>().second(simObj, simModel);
                        } catch (...) {
                            replayFailed = true;
                        }
                    }
                    if (replayFailed)
                        break;  // prefix diverged; skip remaining positions

                    // Regenerate action i from its bookmark using the replayed state
                    const PairType& pairI = vec[i].getAny().getRef<PairType>();
                    Random bookmark = pairI.first;    // copy for capture
                    Random randForGen = pairI.first;  // copy to consume during generation
                    auto freshActionShr = actionGenFactory(simObj, simModel)(randForGen);

                    // Each shrink of the fresh action becomes a candidate vector
                    auto candidates = freshActionShr.getShrinks()
                        .template transform<ShrinkableBase, ShrinkableBase>(
                            [vec, i, minSize, bookmark](const ShrinkableBase& shrunkAction) -> ShrinkableBase {
                                // Map shrunkAction → pair, preserving its further shrink tree
                                Shrinkable<ActionType> shrunkActionShr(shrunkAction);
                                auto newPairShr = shrunkActionShr.template map<PairType>(
                                    [bookmark](const ActionType& action) -> PairType {
                                        return PairType(bookmark, action);
                                    });
                                auto newVec = vec;
                                newVec[i] = ShrinkableBase(newPairShr);
                                auto newVecShr = make_shrinkable<vector<ShrinkableBase>>(newVec);
                                auto newLengthShr = shrinkVectorLength(newVecShr, minSize);
                                return ShrinkableBase(shrinkAnyVector(newLengthShr, minSize, true, false));
                            });

                    result = result.concat(candidates);
                }
                return result;
            });

        // Phase 3: last-action parameter shrinking via the last element's stored shrink tree
        auto withPhase3 = withPhase2b.concat(
            [minSize](ShrinkableBase& nodeShr) -> ShrinkableBase::StreamType {
                const auto& vec = nodeShr.getRef<vector<ShrinkableBase>>();
                if (vec.empty())
                    return ShrinkableBase::StreamType::empty();
                return vec.back().getShrinks().template transform<ShrinkableBase, ShrinkableBase>(
                    [vec, minSize](const ShrinkableBase& shrunkLast) -> ShrinkableBase {
                        auto newVec = vec;
                        newVec.back() = shrunkLast;
                        auto newVecShr = make_shrinkable<vector<ShrinkableBase>>(newVec);
                        auto newLengthShr = shrinkVectorLength(newVecShr, minSize);
                        return ShrinkableBase(shrinkAnyVector(newLengthShr, minSize, true, false));
                    });
            });

        // Final extraction: pull the action out of each pair<Random,Action> element
        return withPhase3.template flatMap<list<ActionType>>(
            +[](const vector<ShrinkableBase>& vec) {
                auto resultPtr = util::make_unique<list<ActionType>>();
                for (const auto& shr : vec)
                    resultPtr->push_back(shr.getAny().getRef<PairType>().second);
                return Shrinkable<list<ActionType>>(
                    util::make_any<list<ActionType>>(util::move(resultPtr)));
            });
    }

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

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, SimpleActionGen<ObjectType>& simpleActionGen)
{
    static EmptyModel emptyModel;
    auto actionGen = simpleActionGen.template map<Action<ObjectType, EmptyModel>>(
        [](const SimpleAction<ObjectType>& simpleAction) { return Action<ObjectType, EmptyModel>(simpleAction); });

    auto modelFactory = +[](ObjectType&) { return emptyModel; };
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

    auto modelFactory = +[](ObjectType&) { return emptyModel; };
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

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(ObjectType&)> modelFactory,
                                ActionGen<ObjectType, ModelType>& actionGen)
{
    return StatefulProperty<ObjectType, ModelType>(util::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(ObjectType&)> modelFactory,
                                ActionGenFactory<ObjectType, ModelType> actionGenFactory)
{
    return StatefulProperty<ObjectType, ModelType>(
        util::forward<InitialGen>(initialGen), modelFactory, util::move(actionGenFactory));
}

template <typename ObjectType, typename ModelType, typename InitialGen, typename Factory>
    requires std::invocable<Factory, ObjectType&, ModelType&> &&
             std::constructible_from<ActionGen<ObjectType, ModelType>,
                                     std::invoke_result_t<Factory, ObjectType&, ModelType&>>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(ObjectType&)> modelFactory,
                                Factory&& actionGenFactory)
{
    return statefulProperty<ObjectType, ModelType>(
        util::forward<InitialGen>(initialGen), modelFactory,
        ActionGenFactory<ObjectType, ModelType>(util::forward<Factory>(actionGenFactory)));
}

}  // namespace stateful
}  // namespace proptest
