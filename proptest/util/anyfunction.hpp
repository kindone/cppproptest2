#pragma once

#include "proptest/std/lang.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/tuple.hpp"
#include "proptest/std/functional.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/tupleorvector.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/typefwd.hpp"

namespace proptest {

template<typename... Args>
concept AllAny = (is_same_v<Args, Any> && ...);

struct AnyFunctionHolder {
    AnyFunctionHolder() = default;
    // delete copy constructor
    AnyFunctionHolder(const AnyFunctionHolder&) = delete;

    virtual ~AnyFunctionHolder() {}

    virtual Any apply(const vector<Any>&) const {
        throw runtime_error("Not implemented");
    };

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) const {
        return apply({Any(arg)...}).template getRef<decay_t<RET>>();
    }
};


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

template <size_t N>
struct AnyFunctionNHolder : public AnyFunctionHolderHelper_t<make_index_sequence<N>>::type {
    virtual ~AnyFunctionNHolder() {}
};

template <typename F>
  requires (is_function_v<F>)
struct FunctionNHolder;

template <typename RET, typename...ARGS>
struct FunctionNHolder<RET(ARGS...)> : public AnyFunctionNHolder<sizeof...(ARGS)> {
    virtual ~FunctionNHolder() {}

    virtual RET operator()(ARGS... arg) const = 0;

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const override {
        return Any(operator()(arg.template getRef<decay_t<ARGS>>()...));
    }
};

template <typename Callable, typename RET, typename...ARGS>
struct FunctionNHolderImpl : public FunctionNHolder<RET(ARGS...)> {
    explicit FunctionNHolderImpl(const Callable& c) : callable(c) {}

    RET operator ()(ARGS... arg) const override {
        return callable(arg...);
    }

    Any apply(const vector<Any>& args) const override {
        return util::invokeWithVector<Callable, ARGS...>(util::forward<const Callable>(callable), args);
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

    template<typename Callable>
        requires (invocable<Callable, ARGS...>)
    Function(Callable&& c) : holder(util::make_shared<FunctionNHolderImpl<Callable, RET, ARGS...>>(util::forward<Callable>(c))) {}

    RET operator()(ARGS... arg) const {
        return holder->apply({Any(arg)...}).template getRef<RET>();
    }

    shared_ptr<AnyFunctionHolder> holder;
};

struct AnyFunction {
    AnyFunction() = delete;
    explicit AnyFunction(shared_ptr<AnyFunctionHolder> h) : holder(h) {}

    template <typename RET, typename... ARGS>
    AnyFunction(const Function<RET(ARGS...)>& f) : holder(f.holder) {}

    Any apply(const vector<Any>& args) {
        return holder->apply(args);
    };

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) const {
        return holder->apply({Any(arg)...}).template getRef<RET>();
    }

    shared_ptr<AnyFunctionHolder> holder;
};


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