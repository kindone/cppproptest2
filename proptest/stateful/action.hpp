#pragma once

#include "proptest/util/function.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/variant.hpp"
#include "proptest/stateful/context.hpp"

/**
 * @file action.hpp
 * @brief Action classes for stateful/concurrency testing
 */

namespace proptest {
namespace stateful {

struct EmptyModel
{
};


template <typename ObjectType>
struct SimpleAction {
    using function_t = Function<void(ObjectType&)>;
    using function_with_context_t = Function<void(ObjectType&, Context&)>;
    explicit SimpleAction(function_t f) : name("Action<?>"), func(f) {}

    SimpleAction(const string& _name, function_t f) : name(_name), func(f) {}

    void operator()(ObjectType& obj) const {
        func(obj);
    }

    friend ostream& operator<<(ostream& os, const SimpleAction& obj) {
        os << obj.name;
        return os;
    }

    string name;
    function_t func;
};

template <typename ObjectType, typename ModelType>
struct Action {
    using function_t = Function<void(ObjectType&, ModelType&)>;
    using function_with_context_t = Function<void(ObjectType&, ModelType&, Context&)>;
    explicit Action(function_t f) : name("Action<?>"), func(f) {}

    Action(const string& _name, function_t f) : name(_name), func(f) {}

    Action(const SimpleAction<ObjectType>& simpleAction) : name(simpleAction.name) {
        func = static_cast<function_t>([simpleAction](ObjectType& obj, ModelType&) {
            return simpleAction(obj);
        });
    }

    void operator()(ObjectType& obj, ModelType& model, Context& context) const {
        if (holds_alternative<function_t>(func)) {
            get<function_t>(func)(obj, model);
        } else {
            get<function_with_context_t>(func)(obj, model, context);
        }
    }

    void operator()(ObjectType& obj, ModelType& model) const {
        if (holds_alternative<function_t>(func)) {
            get<function_t>(func)(obj, model);
        } else {
            Context context;
            get<function_with_context_t>(func)(obj, model, context);
        }
    }

    friend ostream& operator<<(ostream& os, const Action& action) {
        os << action.name;
        return os;
    }

    string name;
    variant<function_t, function_with_context_t> func;
};

} // namespace stateful
} // namespace proptest
