#pragma once

#include "proptest/std/lang.hpp"
#include "proptest/std/initializer_list.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/functional.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/std/io.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/typefwd.hpp"

namespace proptest {

// forward declaration
struct AnyFunction;


struct FunctionHolder {
    FunctionHolder() = default;
    // delete copy constructor
    FunctionHolder(const FunctionHolder&) = delete;

    virtual ~FunctionHolder() {
        // proptest::cout << "FunctionHolder destructor (" << this <<  ")" << proptest::endl;
    }

    virtual Any apply(const initializer_list<Any>&) const {
        throw runtime_error("FunctionHolder::apply const not implemented");
    };

    virtual Any apply(const initializer_list<Any>&) {
        throw runtime_error("FunctionHolder::apply not implemented");
    };

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) const {
        if constexpr(same_as<RET,void>) {
            apply({Any(arg)...});
            return;
        }
        else
            return apply({Any(arg)...}).template getRef<decay_t<RET>>();
    }

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) {
        if constexpr(same_as<RET,void>) {
            apply({Any(arg)...});
            return;
        }
        else
            return apply({Any(arg)...}).template getRef<decay_t<RET>>();
    }
};

namespace util {

template<typename... Args>
concept AllAny = (is_same_v<Args, Any> && ...);


template <typename Callable, typename... ARGS, size_t... Is>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithInitializerListImplConst(const Callable&& callable, const initializer_list<Any>& list, index_sequence<Is...>) {
    if (list.size() != sizeof...(ARGS)) {
        throw std::invalid_argument("number of arguments does not match the number of function parameters");
    }
    auto it = list.begin();
    return callable((*(it + Is)).getRef<ARGS>()...);
}

template<typename Callable, typename... ARGS>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithInitializerList(const Callable&& callable, const initializer_list<Any>& list) {
    return invokeWithInitializerListImplConst<Callable, ARGS...>(util::forward<const Callable>(callable), list, std::index_sequence_for<ARGS...>{});
}

template <typename Callable, typename... ARGS, size_t... Is>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithInitializerListImplMutable(Callable&& callable, const initializer_list<Any>& list, index_sequence<Is...>) {
    if (list.size() != sizeof...(ARGS)) {
        throw std::invalid_argument("number of arguments does not match the number of function parameters");
    }
    auto it = list.begin();
    return callable((*(it + Is)).getRef<ARGS>()...);
}

template<typename Callable, typename... ARGS>
    requires (invocable<Callable, ARGS...>)
decltype(auto) invokeWithInitializerListMutable(Callable&& callable, const initializer_list<Any>& list) {
    return invokeWithInitializerListImplMutable<Callable, ARGS...>(util::forward<Callable>(callable), list, std::index_sequence_for<ARGS...>{});
}

// abstract
template<typename... ARGS>
        requires (AllAny<ARGS...>)
struct FunctionHolderHelper : public FunctionHolder {
    static constexpr int N = sizeof...(ARGS);
    virtual Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const = 0;
    virtual Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) = 0;
};

template <typename T>
struct FunctionHolderHelper_t;

template <size_t... Is>
struct FunctionHolderHelper_t<index_sequence<Is...>> {
    using type = FunctionHolderHelper<decltype((void)Is, Any{})...>;
};

} // namespace util

template <size_t N>
struct AnyFunctionNHolder : public util::FunctionHolderHelper_t<make_index_sequence<N>>::type {
    virtual ~AnyFunctionNHolder() {}
};

template <typename F>
  requires (is_function_v<F>)
struct FunctionNHolder;


// abstract
template <typename RET, typename...ARGS>
struct FunctionNHolder<RET(ARGS...) const> : public AnyFunctionNHolder<sizeof...(ARGS)> {
    virtual ~FunctionNHolder() {}

    virtual RET operator()(ARGS... arg) const = 0;
    virtual RET operator()(ARGS... arg) = 0;

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const override {
        if constexpr(same_as<RET, void>)
            return Any();
        else
            return Any(operator()(arg.template getRef<ARGS>()...));
    }

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) override {
        throw runtime_error("FunctionNHolder::invoke not implemented");
    }
};

template <typename RET, typename...ARGS>
struct FunctionNHolder<RET(ARGS...)> : public AnyFunctionNHolder<sizeof...(ARGS)> {
    virtual ~FunctionNHolder() {}

    virtual RET operator()(ARGS... arg) = 0;
    virtual RET operator()(ARGS... arg) const = 0;

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const override {
        throw runtime_error("FunctionNHolder::invoke const not implemented");
    }

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) override {
        if constexpr(same_as<RET, void>)
            return Any();
        else
            return Any(operator()(arg.template getRef<ARGS>()...));
    }
};

template <typename Callable, typename RET, typename...ARGS>
struct FunctionNHolderImplConst : public FunctionNHolder<RET(ARGS...) const> {
    explicit FunctionNHolderImplConst(const Callable& c) : callable(c) {}
    FunctionNHolderImplConst(const FunctionNHolderImplConst&) = delete;
    FunctionNHolderImplConst(FunctionNHolderImplConst&&) = delete;

    RET operator ()(ARGS... arg) const override {
        return callable(arg...);
    }

    RET operator ()(ARGS... arg) override {
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

    Any apply(const initializer_list<Any>& args) override {
        if constexpr(same_as<RET, void>) {
            util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args);
            return Any();
        }
        else
            return util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args);
    }

    Callable callable;
};

template <typename Callable, typename RET, typename...ARGS>
struct FunctionNHolderImplMutable : public FunctionNHolder<RET(ARGS...)> {
    explicit FunctionNHolderImplMutable(const Callable& c) : callable(c) {
        // proptest::cout << "FunctionNHolderImplMutable constructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() << this << ", sizeof: " << sizeof(FunctionNHolderImplMutable) << proptest::endl;
    }
    // FunctionNHolderImplMutable(const FunctionNHolderImplMutable&) = delete;
    // FunctionNHolderImplMutable(FunctionNHolderImplMutable&&) = delete;

    ~FunctionNHolderImplMutable() {
        // proptest::cout << "FunctionNHolderImplMutable destructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() <<  proptest::endl;
    }

    RET operator ()(ARGS... arg) override {
        return callable(arg...);
    }

    RET operator ()(ARGS... arg) const override {
        throw runtime_error("FunctionNHolderImplMutable::operator() const not implemented");
    }

    Any apply(const initializer_list<Any>& args) override {
        if constexpr(same_as<RET, void>) {
            util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args);
            return Any();
        }
        else
            return util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args);
    }

    Callable callable;
};

template <typename F>
//   requires (is_function_v<F>)
struct Function;

template <typename RET, typename...ARGS>
struct Function<RET(ARGS...)> {
    Function() = delete;
    explicit Function(shared_ptr<FunctionHolder> h) : holder(h) {
        // proptest::cout << "Function shared_ptr constructor for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" << proptest::endl;
    }
    Function(const Function& other) : holder(other.holder) {
        // proptest::cout << "Function copy constructor for " << typeid(RET(ARGS...)).name() << "(" << this <<  " <- " << &other << ")" << proptest::endl;
    }
    Function(const AnyFunction& otherAnyFunction);

    ~Function() {
        // proptest::cout << "Function destructor for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" << proptest::endl;
    }

    template<typename Callable>
        requires (invocable<Callable, ARGS...> && !is_base_of_v<Function, decay_t<Callable>> && is_const_v<Callable>)
    Function(Callable&& c) : holder(util::make_shared<FunctionNHolderImplConst<decay_t<Callable>, RET, ARGS...>>(util::forward<Callable>(c))) {
        // proptest::cout << "Function constructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" <<  proptest::endl;
    }

    template<typename Callable>
        requires (invocable<Callable, ARGS...> && !is_base_of_v<Function, decay_t<Callable>> && !is_const_v<Callable>)
    Function(Callable&& c) : holder(util::make_shared<FunctionNHolderImplMutable<decay_t<Callable>, RET, ARGS...>>(util::forward<Callable>(c))) {
        // proptest::cout << "Function constructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" << proptest::endl;
    }

    RET operator()(ARGS... arg) const {
        if constexpr(same_as<RET, void>) {
            holder->apply({util::make_any<ARGS>(arg)...});
            return;
        }
        else
            return holder->apply({util::make_any<ARGS>(arg)...}).template getRef<RET>();
    }

     RET operator()(ARGS... arg) {
        if constexpr(same_as<RET, void>) {
            holder->apply({util::make_any<ARGS>(arg)...});
            return;
        }
        else
            return holder->apply({util::make_any<ARGS>(arg)...}).template getRef<RET>();
    }

    shared_ptr<FunctionHolder> holder;
};

struct AnyFunction {
    AnyFunction() = delete;
    explicit AnyFunction(shared_ptr<FunctionHolder> h) : holder(h) {}

    template <typename RET, typename... ARGS>
    AnyFunction(const Function<RET(ARGS...)>& f) : holder(f.holder) {}

    Any apply(const initializer_list<Any>& args) {
        return holder->apply(args);
    }

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) const {
        return holder->apply({util::make_any<ARGS>(arg)...}).template getRef<RET>();
    }

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) {
        return holder->apply({util::make_any<ARGS>(arg)...}).template getRef<RET>();
    }

    shared_ptr<FunctionHolder> holder;
};

template <typename RET, typename...ARGS>
Function<RET(ARGS...)>::Function(const AnyFunction& otherAnyFunction) : holder(otherAnyFunction.holder) {}

namespace util {

template <typename Callable>
decltype(auto) make_function(Callable&& c) {
    using HolderType = typename function_traits<Callable>::template template_type_with_self_ret_and_args<FunctionNHolderImplConst, Callable>;
    using FunctionType = typename function_traits<Callable>::template function_type_with_signature<Function>;
    return FunctionType{static_pointer_cast<FunctionHolder>(util::make_shared<HolderType>(util::forward<Callable>(c)))};
}

template <typename Callable>
AnyFunction make_anyfunction(Callable&& c) {
    using HolderType = typename function_traits<Callable>::template template_type_with_self_ret_and_args<FunctionNHolderImplConst, Callable>;
    return AnyFunction{static_pointer_cast<FunctionHolder>(util::make_shared<HolderType>(util::forward<Callable>(c)))};
}

} // namespace util

} // namespace proptest