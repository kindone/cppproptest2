#pragma once
#include "proptest/std/lang.hpp"
#include "proptest/util/typelist.hpp"

namespace proptest {

template <typename F> struct Function;
template <typename RET, typename...ARGS> struct Function<RET(ARGS...)>;

template <template <typename...> typename TEMPLATE, typename RET, typename...ARGS>
TEMPLATE<RET(ARGS...)> TypeListToFunctionType(util::TypeList<ARGS...> list)
{
    return declval<TEMPLATE<RET(ARGS...)>>();
}

template <template <typename...> typename TEMPLATE, typename RET, typename ARG_TYPELIST>
struct TypeListToFunctionTypeHelper
{
    using type = decltype(TypeListToFunctionType<TEMPLATE, RET>(declval<ARG_TYPELIST>()));
};


template <class F>
struct function_traits;

template <class R, class... Args>
struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)>
{
};

template <class R, class... Args>
struct function_traits<R(Args...)>
{
    using return_type = R;
    static constexpr size_t arity = sizeof...(Args);
    using argument_type_list = util::TypeList<Args...>;

    template <template <typename...> typename TEMPLATE, typename NEW_RETURN_TYPE = return_type>
    using function_type_with_signature = typename TypeListToFunctionTypeHelper<TEMPLATE, return_type, argument_type_list>::type;
};

// member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)> : public function_traits<R(C&, Args...)>
{
};

// const member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const> : public function_traits<R(C&, Args...)>
{
};

// member object pointer
template <class C, class R>
struct function_traits<R C::*> : public function_traits<R(C&)>
{
};

// member object pointer
template <class R, class...ARGS>
struct function_traits<Function<R(ARGS...)>> : public function_traits<R(ARGS...)>
{
};


// functor
template <class F>
struct function_traits
{
private:
    using call_type = function_traits<decltype(&F::operator())>;
    using full_argument_type_list = typename call_type::argument_type_list;
public:
    static constexpr size_t arity = call_type::arity - 1;
    using return_type = typename call_type::return_type;
    using argument_type_list = typename full_argument_type_list::tail;
private:
    template <size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename call_type::template argument<N + 1>::type;
    };

    using return_and_argument_type_list = typename argument_type_list::template prepend<return_type>;
    template <typename TARGET_RET>
    using converted_return_and_argument_type_list = typename argument_type_list::template prepend<return_type>::template prepend<TARGET_RET>;

public:
    template <template <typename...> typename TEMPLATE>
    using template_type_with_args = typename TypeListToTemplateTypeHelper<TEMPLATE, argument_type_list>::type;
    template <template <typename...> typename TEMPLATE>
    using template_type_with_ret_and_args = typename TypeListToTemplateTypeHelper<TEMPLATE, return_and_argument_type_list>::type;
    template <template <typename...> typename TEMPLATE, typename NEW_RETURN_TYPE = return_type>
    using function_type_with_signature = typename TypeListToFunctionTypeHelper<TEMPLATE, return_type, argument_type_list>::type;
    template <template <typename...> typename TEMPLATE, typename CALLABLE>
    using template_type_with_self_ret_and_args = typename TypeListToTemplateTypeHelper<TEMPLATE, typename return_and_argument_type_list::template prepend<CALLABLE>>::type;
    template <template <typename...> typename TEMPLATE, typename CALLABLE, typename TARGET_RET>
    using template_type_with_self_converted_ret_and_args = typename TypeListToTemplateTypeHelper<TEMPLATE, typename converted_return_and_argument_type_list<TARGET_RET>::template prepend<CALLABLE>>::type;

    template <size_t N>
    using argument_type = typename argument<N>::type;
};

template <class F>
struct function_traits<F&> : public function_traits<F>
{
};

template <class F>
struct function_traits<F&&> : public function_traits<F>
{
};

}  // namespace proptest
