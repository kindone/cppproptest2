#pragma once

#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/initializer_list.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/functional.hpp"
#include "proptest/std/concepts.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/tuple.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/typefwd.hpp"

namespace proptest {

// forward declaration
// struct AnyFunction;


struct FunctionHolder {
    FunctionHolder() = default;
    // delete copy constructor
    FunctionHolder(const FunctionHolder&) = delete;

    virtual ~FunctionHolder() {
        // proptest::cout << "FunctionHolder destructor (" << this <<  ")" << proptest::endl;
    }

    virtual Any apply(const initializer_list<Any>&) const {
        throw runtime_error(__FILE__, __LINE__, "FunctionHolder::apply const not implemented");
    };

    virtual Any apply(const initializer_list<Any>&) {
        throw runtime_error(__FILE__, __LINE__, "FunctionHolder::apply not implemented");
    };

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) const {
        if constexpr(is_void_v<RET>) {
            apply({Any(arg)...});
            return;
        }
        else
            return apply({Any(arg)...}).template getRef<decay_t<RET>>();
    }

    template <typename RET, typename... ARGS>
    RET call(ARGS... arg) {
        if constexpr(is_void_v<RET>) {
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
        throw invalid_argument(__FILE__, __LINE__, "number of arguments does not match the number of function parameters");
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
        throw invalid_argument(__FILE__, __LINE__, "number of arguments does not match the number of function parameters");
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

/*
template <size_t N>
struct AnyFunctionNHolder : public util::FunctionHolderHelper_t<make_index_sequence<N>>::type {
    virtual ~AnyFunctionNHolder() {}
};

template <typename TARGET_RET, typename F>
  requires (is_function_v<F>)
struct FunctionNHolder;


// abstract
template <typename TARGET_RET, typename RET, typename...ARGS>
struct FunctionNHolder<TARGET_RET, RET(ARGS...) const> : public AnyFunctionNHolder<sizeof...(ARGS)> {
    virtual ~FunctionNHolder() {}

    virtual TARGET_RET operator()(ARGS... arg) const = 0;
    //virtual TARGET_RET operator()(ARGS... arg) = 0;

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) const override {
        if constexpr(is_void_v<TARGET_RET>) {
            operator()(arg.template getRef<ARGS>(true)...);
            return Any();
        }
        else
            return Any(static_cast<TARGET_RET>(operator()(arg.template getRef<ARGS>(true)...)));
    }

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>...) override {
        throw runtime_error(__FILE__, __LINE__, "FunctionNHolder::invoke not implemented");
    }
};

template <typename TARGET_RET, typename RET, typename...ARGS>
struct FunctionNHolder<TARGET_RET, RET(ARGS...)> : public AnyFunctionNHolder<sizeof...(ARGS)> {
    virtual ~FunctionNHolder() {}

    //virtual TARGET_RET operator()(ARGS... arg) = 0;
    virtual TARGET_RET operator()(ARGS... arg) const = 0;

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>...) const override {
        throw runtime_error(__FILE__, __LINE__, "FunctionNHolder::invoke const not implemented");
    }

    Any invoke(conditional_t<is_same_v<ARGS, Any>, Any, Any>... arg) override {
        if constexpr(is_void_v<TARGET_RET>) {
            operator()(arg.template getRef<ARGS>(true)...);
            return Any();
        }
        else
            return Any(static_cast<TARGET_RET>(operator()(arg.template getRef<ARGS>()...)));
    }
};

template <typename Callable, typename TARGET_RET, typename RET, typename...ARGS>
struct FunctionNHolderConst : public FunctionNHolder<TARGET_RET, RET(ARGS...) const> {
    explicit FunctionNHolderConst(const Callable& c) : callable(c) {}
    FunctionNHolderConst(const FunctionNHolderConst&) = delete;
    FunctionNHolderConst(FunctionNHolderConst&&) = delete;

    TARGET_RET operator ()(ARGS... arg) const override {
        return static_cast<TARGET_RET>(callable(arg...));
    }


    //TARGET_RET operator ()(ARGS... arg) override {
    //    return static_cast<TARGET_RET>(callable(arg...));
    //}


    Any apply(const initializer_list<Any>& args) const override {
        if constexpr(same_as<TARGET_RET, void>) {
            util::invokeWithInitializerList<Callable, ARGS...>(util::forward<const Callable>(callable), args);
            return Any();
        }
        else
            return static_cast<TARGET_RET>(util::invokeWithInitializerList<Callable, ARGS...>(util::forward<const Callable>(callable), args));
    }

    Any apply(const initializer_list<Any>& args) override {
        if constexpr(same_as<TARGET_RET, void>) {
            util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args);
            return Any();
        }
        else
            return static_cast<TARGET_RET>(util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args));
    }

    Callable callable;
};

template <typename Callable, typename TARGET_RET, typename RET, typename...ARGS>
struct FunctionNHolderMutable : public FunctionNHolder<TARGET_RET, RET(ARGS...)> {
    explicit FunctionNHolderMutable(const Callable& c) : callable(c) {
        // proptest::cout << "FunctionNHolderMutable constructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() << this << ", sizeof: " << sizeof(FunctionNHolderMutable) << proptest::endl;
    }
    FunctionNHolderMutable(const FunctionNHolderMutable&) = delete;
    FunctionNHolderMutable(FunctionNHolderMutable&&) = delete;

    ~FunctionNHolderMutable() {
        // proptest::cout << "FunctionNHolderMutable destructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() <<  proptest::endl;
    }


    //TARGET_RET operator ()(ARGS... arg) override {
    //    return callable(arg...);
    //}


    TARGET_RET operator ()(ARGS...args) const override {
        return callable(args...);
    }

    Any apply(const initializer_list<Any>& args) override {
        if constexpr(same_as<TARGET_RET, void>) {
            util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args);
            return Any();
        }
        else
            return static_cast<TARGET_RET>(util::invokeWithInitializerListMutable<Callable, ARGS...>(util::forward<Callable>(callable), args));
    }

    mutable Callable callable;
};
*/

template <typename Callable, typename RET, typename...ARGS>
concept isCallableOf = (invocable<Callable, ARGS...> && (is_same_v<RET,void> || is_constructible_v<RET, invoke_result_t<Callable, ARGS...>>));

template <typename F>
//   requires (is_function_v<F>)
struct Function;

template <typename RET, typename...ARGS>
struct PROPTEST_API Function<RET(ARGS...)> {
    using ArgTuple = tuple<ARGS...>;
    using RetType = RET;
    static constexpr size_t Arity = sizeof...(ARGS);

    Function() {}

    //explicit Function(shared_ptr<FunctionHolder> h) : holder(h) {
    //    // proptest::cout << "Function shared_ptr constructor for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" << proptest::endl;
    //}

    Function(const Function& other) : holder(other.holder) {
        // proptest::cout << "Function copy constructor for " << typeid(RET(ARGS...)).name() << "(" << this <<  " <- " << &other << ")" << proptest::endl;
    }

    // Function(const AnyFunction& otherAnyFunction);

    ~Function() {
        // proptest::cout << "Function destructor for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" << proptest::endl;
    }

    template<typename Callable>
        requires (!is_base_of_v<Function, decay_t<Callable>> && is_const_v<Callable> && isCallableOf<Callable, RET, ARGS...>)
    Function(Callable&& c) : holder(util::make_shared<std::function<RET(ARGS...)>>(util::forward<Callable>(c))) {
        static_assert(isCallableOf<Callable, RET, ARGS...>, "Callable does not match function signature");
        // proptest::cout << "Function constructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" <<  proptest::endl;
    }

    template<typename Callable>
        requires (!is_base_of_v<Function, decay_t<Callable>> && !is_const_v<Callable> && isCallableOf<Callable, RET, ARGS...>)
    Function(Callable&& c) : holder(util::make_shared<std::function<RET(ARGS...)>>(util::forward<Callable>(c))) {
        static_assert(isCallableOf<Callable, RET, ARGS...>, "Callable does not match function signature");
        // proptest::cout << "Function constructor: " << typeid(Callable).name() << " for " << typeid(RET(ARGS...)).name() << "(" << this <<  ")" << proptest::endl;
    }

    operator bool() const {
        return static_cast<bool>(holder);
    }

    template <typename...Args>
    RET operator()(Args&&... args) const {
        if(!holder)
            throw runtime_error(__FILE__, __LINE__, "Function not initialized");
        if constexpr(is_void_v<RET>) {
            (*holder)(util::forward<Args>(args)...);
            return;
        }
        else
            return (*holder)(util::forward<Args>(args)...);
    }

    /*
    RET operator()(ARGS... arg) {
        if(!holder)
            throw runtime_error(__FILE__, __LINE__, "Function not initialized");
        if constexpr(is_void_v<RET>) {
            holder->apply({util::make_any<ARGS>(arg)...});
            return;
        }
        else
            return holder->apply({util::make_any<ARGS>(arg)...}).template getRef<RET>();
    }
    */

    mutable shared_ptr<std::function<RET(ARGS...)>> holder;
};

/*
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

*/

namespace util {

/*
template <typename Callable>
decltype(auto) make_function(Callable&& c) {
    using RetType = typename function_traits<Callable>::return_type;
    using HolderType = typename function_traits<Callable>::template template_type_with_self_converted_ret_and_args<FunctionNHolderConst, Callable, RetType>;
    using FunctionType = typename function_traits<Callable>::template function_type_with_signature<Function>;
    return FunctionType{static_pointer_cast<FunctionHolder>(util::make_shared<HolderType>(util::forward<Callable>(c)))};
}

template <typename Callable>
AnyFunction make_anyfunction(Callable&& c) {
    using RetType = typename function_traits<Callable>::return_type;
    using HolderType = typename function_traits<Callable>::template template_type_with_self_converted_ret_and_args<FunctionNHolderConst, Callable, RetType>;
    return AnyFunction{static_pointer_cast<FunctionHolder>(util::make_shared<HolderType>(util::forward<Callable>(c)))};
}
*/

} // namespace util

} // namespace proptest
