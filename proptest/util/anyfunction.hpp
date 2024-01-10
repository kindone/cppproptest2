#pragma once

#include "proptest/std/lang.hpp"
#include "proptest/std/initializer_list.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/functional.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/typefwd.hpp"

namespace proptest {

// forward declaration
struct AnyFunction;


struct AnyFunctionHolder {
    AnyFunctionHolder() = default;
    // delete copy constructor
    AnyFunctionHolder(const AnyFunctionHolder&) = delete;

    virtual ~AnyFunctionHolder() {}

    virtual Any apply(const initializer_list<Any>&) const {
        throw runtime_error("Not implemented");
    };

    template <typename RET, typename... ARGS>
        requires(!same_as<RET, void>)
    RET call(ARGS... arg) const {
        if constexpr(same_as<RET,void>) {
            apply({Any(arg)...});
            return;
        }
        else
            return apply({Any(arg)...}).template getRef<decay_t<RET>>();
    }

    template <typename RET, typename... ARGS>
        requires(same_as<RET, void>)
    void call(ARGS... arg) const {
        apply({Any(arg)...});
        return;
    }
};

namespace util {

template<typename... Args>
concept AllAny = (is_same_v<Args, Any> && ...);


template <typename Callable, typename... ARGS, size_t... Is>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithInitializerListImpl(const Callable&& callable, const initializer_list<Any>& list, index_sequence<Is...>) {
    if (list.size() != sizeof...(ARGS)) {
        throw std::invalid_argument("number of arguments does not match the number of function parameters");
    }
    auto it = list.begin();
    return callable((*(it + Is)).getRef<ARGS>()...);
}

template<typename Callable, typename... ARGS>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithInitializerList(const Callable&& callable, const initializer_list<Any>& list) {
    return invokeWithInitializerListImpl<Callable, ARGS...>(util::forward<const Callable>(callable), list, std::index_sequence_for<ARGS...>{});
}

// abstract
template<typename... ARGS>
        requires (AllAny<ARGS...>)
struct AnyFunctionHolderHelper : public AnyFunctionHolder {
    static constexpr int N = sizeof...(ARGS);
    virtual Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const = 0;
};

template <typename T>
struct AnyFunctionHolderHelper_t;

template <size_t... Is>
struct AnyFunctionHolderHelper_t<index_sequence<Is...>> {
    using type = AnyFunctionHolderHelper<decltype((void)Is, Any{})...>;
};

} // namespace util

template <size_t N>
struct AnyFunctionNHolder : public util::AnyFunctionHolderHelper_t<make_index_sequence<N>>::type {
    virtual ~AnyFunctionNHolder() {}
};

template <typename F>
  requires (is_function_v<F>)
struct FunctionNHolder;


// abstract
template <typename RET, typename...ARGS>
struct FunctionNHolder<RET(ARGS...)> : public AnyFunctionNHolder<sizeof...(ARGS)> {
    virtual ~FunctionNHolder() {}

    virtual RET operator()(ARGS... arg) const = 0;

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const override {
        if constexpr(same_as<RET, void>)
            return Any();
        else
            return Any(operator()(arg.template getRef<ARGS>()...));
    }
};

template <typename Callable, typename RET, typename...ARGS>
struct FunctionNHolderImpl : public FunctionNHolder<RET(ARGS...)> {
    explicit FunctionNHolderImpl(const Callable& c) : callable(c) {}

    RET operator ()(ARGS... arg) const override {
        return callable(arg...);
    }

    Any apply(const initializer_list<Any>& args) const override {
        if constexpr(same_as<RET, void>) {
            util::invokeWithInitializerList<Callable, ARGS...>(util::forward<const Callable>(callable), args);
            return Any();
        }
        else
            return util::invokeWithInitializerList<Callable, ARGS...>(util::forward<const Callable>(callable), args);
    }

    Callable callable;
};

template <typename F>
  requires (is_function_v<F>)
struct Function;

template <typename RET, typename...ARGS>
struct Function<RET(ARGS...)> {
    Function() = delete;
    explicit Function(shared_ptr<AnyFunctionHolder> h) : holder(h) {}
    Function(const Function& other) : holder(other.holder) {}
    Function(const AnyFunction& otherAnyFunction);

    template<typename Callable>
        requires (invocable<Callable, ARGS...> && !same_as<decay_t<Callable>, Function>)
    Function(Callable&& c) : holder(util::make_shared<FunctionNHolderImpl<Callable, RET, ARGS...>>(util::forward<Callable>(c))) {}

    RET operator()(ARGS... arg) const {
        if constexpr(same_as<RET, void>) {
            holder->apply({util::make_any<ARGS>(arg)...});
            return;
        }
        else
            return holder->apply({util::make_any<ARGS>(arg)...}).template getRef<RET>();
    }

    shared_ptr<AnyFunctionHolder> holder;
};

struct AnyFunction {
    AnyFunction() = delete;
    explicit AnyFunction(shared_ptr<AnyFunctionHolder> h) : holder(h) {}

    template <typename RET, typename... ARGS>
    AnyFunction(const Function<RET(ARGS...)>& f) : holder(f.holder) {}

    Any apply(const initializer_list<Any>& args) {
        return holder->apply(args);
    }

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) const {
        return holder->apply({Any(arg)...}).template getRef<RET>();
    }

    shared_ptr<AnyFunctionHolder> holder;
};

template <typename RET, typename...ARGS>
Function<RET(ARGS...)>::Function(const AnyFunction& otherAnyFunction) : holder(otherAnyFunction.holder) {}


template <typename Callable>
decltype(auto) make_function(Callable&& c) {
    using HolderType = typename function_traits<Callable>::template template_type_with_self_ret_and_args<FunctionNHolderImpl, Callable>;
    using FunctionType = typename function_traits<Callable>::template function_type_with_signature<Function>;
    return FunctionType{static_pointer_cast<AnyFunctionHolder>(util::make_shared<HolderType>(util::forward<Callable>(c)))};
}

template <typename Callable>
AnyFunction make_any_function(Callable&& c) {
    using HolderType = typename function_traits<Callable>::template template_type_with_self_ret_and_args<FunctionNHolderImpl, Callable>;
    return AnyFunction{static_pointer_cast<AnyFunctionHolder>(util::make_shared<HolderType>(util::forward<Callable>(c)))};
}

} // namespace proptest