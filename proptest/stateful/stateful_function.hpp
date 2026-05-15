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

                        auto actionShrinkables = make_shrinkable<vector<ShrinkableBase>>();
                        auto& actionShrinkableVec = actionShrinkables.getMutableRef();
                        actionShrinkableVec.reserve(numActions);

                        for (size_t i = 0; i < numActions; ++i) {
                            auto nextActionGen = actionGenFactory(obj, model);
                            auto actionShr = nextActionGen(rand);
                            actionShrinkableVec.push_back(actionShr);
                            actionShr.getRef()(obj, model);
                        }

                        auto actionListShr =
                            shrinkListLike<list, Action<ObjectType, ModelType>>(actionShrinkables, actionListMinSize);
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
