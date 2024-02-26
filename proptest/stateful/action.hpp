#pragma once

#include "proptest/util/anyfunction.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/io.hpp"


namespace proptest {
namespace stateful {


struct EmptyModel
{
};

template <typename ObjectType>
struct SimpleAction {
    using Function = Function<void(ObjectType&)>;
    explicit SimpleAction(Function f) : name("Action<?>"), func(f) {}

    SimpleAction(const string& _name, Function f) : name(_name), func(f) {}

    void operator()(ObjectType& obj) const {
        func(obj);
    }

    friend ostream& operator<<(ostream& os, const SimpleAction& obj) {
        os << obj.name;
        return os;
    }

    string name;
    Function func;
};

template <typename ObjectType, typename ModelType>
struct Action {
    using Function = Function<void(ObjectType&, ModelType&)>;
    explicit Action(Function f) : name("Action<?>"), func(f) {}

    Action(const string& _name, Function f) : name(_name), func(f) {}

    Action(const SimpleAction<ObjectType>& simpleAction) : name(simpleAction.name) {
        func = [simpleAction](ObjectType& obj, ModelType&) {
            return simpleAction(obj);
        };
    }

    void operator()(ObjectType& obj, ModelType& model) const {
        func(obj, model);
    }

    friend ostream& operator<<(ostream& os, const Action& action) {
        os << action.name;
        return os;
    }

    string name;
    Function func;
};

} // namespace stateful
} // namespace proptest
