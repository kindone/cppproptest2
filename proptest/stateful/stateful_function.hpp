#pragma once

#include "proptest/util/function_traits.hpp"
#include "proptest/stateful/action.hpp"
#include "proptest/combinator/transform.hpp"
#include "proptest/combinator/oneof.hpp"
#include "proptest/combinator/just.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/Generator.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/std/list.hpp"
#include "proptest/Property.hpp"


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
    using Func = function<bool(ObjectType, list<Action<ObjectType, ModelType>>)>;

public:
    StatefulProperty(InitialGen&& initGen, ModelFactoryFunction mdlFactory, ActionGen<ObjectType, ModelType>& actGen)
        : seed(UINT64_MAX), numRuns(UINT32_MAX), maxDurationMs(UINT32_MAX), initialGen(initGen), modelFactory(mdlFactory), actionGen(actGen)
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

    bool go()
    {
        // TODO add interface to adjust list min max sizes
        auto actionListGen = Arbi<list<Action<ObjectType, ModelType>>>(actionGen);
        vector<AnyGenerator> genVec({initialGen, actionListGen});

        auto func = [modelFactory = this->modelFactory, postCheck = this->postCheck](ObjectType obj,
                                                                         list<Action<ObjectType, ModelType>> actions) {
            auto model = modelFactory(obj);
            for (auto action : actions) {
                action(obj, model);
            }
            if (postCheck)
                postCheck(obj, model);
            return true;
        };

        auto prop = util::make_shared<PropertyType>(func, util::move(genVec));
        if (onStartup)
            prop->setOnStartup(onStartup);
        if (onCleanup)
            prop->setOnStartup(onCleanup);
        if (seed != UINT64_MAX)
            prop->setSeed(seed);
        if (numRuns != UINT32_MAX)
            prop->setNumRuns(numRuns);
        if (maxDurationMs != UINT32_MAX)
            prop->setMaxDurationMs(maxDurationMs);
        return prop->forAll();
    }

private:
    uint64_t seed;
    uint32_t numRuns;
    uint32_t maxDurationMs;
    InitialGen initialGen;
    ModelFactoryFunction modelFactory;
    ActionGen<ObjectType, ModelType> actionGen;

    Function<void(ObjectType&, ModelType&)> postCheck;
    Function<void()> onStartup;
    Function<void()> onCleanup;
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
decltype(auto) statefulProperty(InitialGen&& initialGen, function<ModelType(ObjectType&)> modelFactory,
                                ActionGen<ObjectType, ModelType>& actionGen)
{
    return StatefulProperty<ObjectType, ModelType>(util::forward<InitialGen>(initialGen), modelFactory, actionGen);
}

}  // namespace stateful
}  // namespace proptest
