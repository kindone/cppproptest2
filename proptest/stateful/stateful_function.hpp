#pragma once

#include "proptest/util/function_traits.hpp"
#include "proptest/stateful/action.hpp"
#include "proptest/Generator.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/just.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/std/list.hpp"
#include "proptest/Property.hpp"
#include "proptest/std/chrono.hpp"
#include "proptest/std/optional.hpp"

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

template <typename ObjectType, typename ModelType>
class StatefulProperty {
    using InitialGen = GenFunction<ObjectType>;
    using ModelFactoryFunction = Function<ModelType(ObjectType&)>;
    using PropertyType = Property<ObjectType, list<Action<ObjectType, ModelType>>>;
    using Func = Function<bool(ObjectType, list<Action<ObjectType, ModelType>>)>;

public:
    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGen<ObjectType, ModelType>& actGen)
        : initialGen(initGen), modelFactory(mdlFactory), actionGen(actGen)
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

    bool go()
    {
        // TODO add interface to adjust list min max sizes
        auto actionListGen = Arbi<list<Action<ObjectType, ModelType>>>(actionGen);
        vector<AnyGenerator> genVec({initialGen, actionListGen});

        auto func = [modelFactory = this->modelFactory, postCheck = this->postCheck,
                     onActionStart = this->onActionStart, onActionEnd = this->onActionEnd](
                        ObjectType obj, list<Action<ObjectType, ModelType>> actions) {
            auto model = modelFactory(obj);
            for (auto action : actions) {
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
    ActionGen<ObjectType, ModelType> actionGen;

    Function<void(ObjectType&, ModelType&)> postCheck;
    Function<void(ObjectType&, ModelType&)> onActionStart;
    Function<void(ObjectType&, ModelType&)> onActionEnd;
    Function<void()> onStartup;
    Function<void()> onCleanup;
    Function<void(ReproductionStats)> onReproductionStats;
    Function<void(int, const vector<Any>&, const string&)> onFailureReproduction;

    optional<ReproductionStats> lastReproductionStats;
};

template <typename ObjectType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, SimpleActionGen<ObjectType>& actionGen)
{
    static EmptyModel emptyModel;
    auto actionGen2 = actionGen.template map<Action<ObjectType, EmptyModel>>(
        [](const SimpleAction<ObjectType>& simpleAction) { return Action<ObjectType, EmptyModel>(simpleAction); });

    auto modelFactory = +[](ObjectType&) { return emptyModel; };
    return StatefulProperty<ObjectType, EmptyModel>(util::forward<InitialGen>(initialGen), modelFactory, actionGen2);
}

template <typename ObjectType, typename ModelType, typename InitialGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, Function<ModelType(ObjectType&)> modelFactory,
                                ActionGen<ObjectType, ModelType>& actionGen)
{
    return StatefulProperty<ObjectType, ModelType>(util::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

}  // namespace stateful
}  // namespace proptest
