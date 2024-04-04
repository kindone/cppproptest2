#include "proptest/api.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/any.hpp"

namespace proptest {

struct Callable1HolderBase {
    virtual ~Callable1HolderBase() {}
    virtual Any operator()(const Any& arg) = 0;
};

template <typename Callable, typename RET, typename ARG>
struct Callable1Holder : public Callable1HolderBase {
    explicit Callable1Holder(const Callable& c) : callable(c) {}

    static_assert(isCallableOf<Callable, RET, ARG>, "Callable has incompatible signature for RET(ARG)>");

    Any operator()(const Any& arg) override {
        if constexpr(is_void_v<RET>) {
            callable(util::toCallableArg<ARG>(arg.getRef<ARG>()));
            return Any();
        }
        else
            return Any(callable(util::toCallableArg<ARG>(arg.getRef<ARG>())));
    }

    decay_t<Callable> callable;
};


struct PROPTEST_API Function1
{
    template <typename Callable>
        requires (!is_base_of_v<Function1, decay_t<Callable>>)
    Function1(Callable&& c) : holder(util::make_shared<Callable1Holder<Callable, typename function_traits<Callable>::return_type, typename function_traits<Callable>::argument_type_list::head>>(util::forward<Callable>(c))) {
    }

    Any operator()(const Any& arg) const {
        return holder->operator()(arg);
    }

    shared_ptr<Callable1HolderBase> holder;
};

template <typename RET, typename ARG>
struct Function1Impl : Function1
{
    Function1Impl(const Function1& f) : Function1(f) {}

    RET operator()(const ARG& arg) const {
        if constexpr(is_void_v<RET>) {
            holder->operator()(util::toCallableArg<ARG>(arg));
            return;
        }
        else
            return holder->operator()(util::toCallableArg<ARG>(arg)).template getRef<RET>();
    }
};


template <typename F>
using Func1 = Function1;

} // namespace proptest
