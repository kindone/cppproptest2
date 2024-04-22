#pragma once
#include "proptest/api.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/any.hpp"

namespace proptest {

template <typename RET>
struct Callable1HolderBase {
    virtual ~Callable1HolderBase() {}
    virtual RET operator()(const Any& arg) = 0;
    virtual RET operator()(void* ptr) = 0;
};

template <typename Callable, typename RET, typename ARG>
    requires (!is_void_v<RET>)
struct Callable1Holder : public Callable1HolderBase<RET> {
    explicit Callable1Holder(const Callable& c) : callable(c) {}

    static_assert(isCallableOf<Callable, RET, ARG>, "Callable has incompatible signature for RET(ARG)>");

    RET operator()(const Any& arg) override {
        return callable(util::toCallableArg<ARG>(arg.getRef<ARG>()));
    }

    RET operator()(void* ptr) override {
        return callable(util::toCallableArg<ARG>(*reinterpret_cast<const decay_t<ARG>*>(ptr)));
    }

    decay_t<Callable> callable;
};


template <typename RET>
struct PROPTEST_API Function1
{
    template <typename Callable>
        requires (!is_base_of_v<Function1<RET>, decay_t<Callable>> && is_constructible_v<RET, typename function_traits<Callable>::return_type>)
    Function1(Callable&& c) : holder(util::make_shared<Callable1Holder<Callable, RET, typename function_traits<Callable>::argument_type_list::head>>(util::forward<Callable>(c))) {
    }

    RET operator()(const Any& arg) const {
        return holder->operator()(arg);
    }

    template <typename T>
    RET callDirect(T& arg) const {
        return holder->operator()(static_cast<void*>(&arg));
    }

    shared_ptr<Callable1HolderBase<RET>> holder;
};

template <typename RET, typename ARG>
struct Function1Impl : public Function1<RET>
{
    Function1Impl(const Function1<RET>& f) : Function1<RET>(f) {}

    RET operator()(const ARG& arg) const {
        return this->holder->operator()(util::toCallableArg<ARG>(arg));
    }
};


// template <typename F>
// using Func1 = Function1;

} // namespace proptest
