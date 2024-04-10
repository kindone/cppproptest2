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

namespace util {

/* const T& should be turned into compatible type T accordingly*/
template <typename T>
T toCallableArg(const decay_t<T>& value) {
	if constexpr (std::is_reference_v<T>) {
		if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
			return value;
		} else {
			return const_cast<decay_t<T>&>(value);
		}
	} else {
		return value;
	}
}

} // namespace util

template <typename Callable, typename RET, typename...ARGS>
concept isCallableOf = (invocable<Callable, invoke_result_t<decltype(util::toCallableArg<ARGS>), ARGS>...> && (is_same_v<RET,void> || is_constructible_v<RET, invoke_result_t<Callable, invoke_result_t<decltype(util::toCallableArg<ARGS>), ARGS>...>>));

template <typename RET, typename...ARGS>
struct CallableHolderBase {
    virtual ~CallableHolderBase() {}
    virtual RET operator()(const decay_t<ARGS>&... args) = 0;
};

template <typename Callable, typename RET, typename...ARGS>
struct CallableHolder : public CallableHolderBase<RET, ARGS...> {
    explicit CallableHolder(const Callable& c) : callable(c) {}

    static_assert(isCallableOf<Callable, RET, ARGS...>, "Callable has incompatible signature for RET(ARGS...)>");

    RET operator()(const decay_t<ARGS>&... args) override {
        if constexpr(is_void_v<RET>) {
            callable(util::toCallableArg<ARGS>(args)...);
            return;
        }
        else
            return callable(util::toCallableArg<ARGS>(args)...);
    }

    decay_t<Callable> callable;
};

template <typename F> struct Function;

template <typename RET, typename...ARGS>
struct PROPTEST_API Function<RET(ARGS...)> {
    using ArgTuple = tuple<ARGS...>;
    using RetType = RET;
    static constexpr size_t Arity = sizeof...(ARGS);

    Function() = default;

    Function(const Function& other) : holder(other.holder) {}

    template <typename Callable>
        requires (!is_base_of_v<Function, decay_t<Callable>>)
    Function(Callable&& c) : holder(util::make_shared<CallableHolder<Callable, RET, ARGS...>>(util::forward<Callable>(c))) {
    }

    operator bool() const {
        return static_cast<bool>(holder);
    }

    RET operator()(const decay_t<ARGS>&... args) const {
        if(!holder)
            throw runtime_error(__FILE__, __LINE__, "Function not initialized");
        if constexpr(is_void_v<RET>) {
            holder->operator()(args...);
            return;
        }
        else
            return holder->operator()(args...);
    }

    mutable shared_ptr<CallableHolderBase<RET, ARGS...>> holder;
};


} // namespace proptest
