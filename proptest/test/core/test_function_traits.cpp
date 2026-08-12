#include "proptest/util/function_traits.hpp"
#include "proptest/util/function.hpp"
#include "proptest/std/functional.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;


int foo(int a, int b)
{
    return a+b;
}

struct Bar
{
    int operator()(int a, int b)
    {
        return a+b;
    }
};

TEST(FunctionTraits, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    using TheOtherType = function_traits<decltype(lambda)>::function_type_with_signature<Function>;
    static_assert(is_same_v<TheOtherType, Function<int(int, int)>>, "TheOtherType should be Function<int(int, int)>");
    [[maybe_unused]] TheOtherType otherType;
    using TheOtherType2 = function_traits<TheOtherType>::function_type_with_signature<Function>;
    static_assert(is_same_v<TheOtherType, TheOtherType2>, "TheOtherType and TheOtherType2 should be the same");
    using TheOtherType3 = function_traits<decltype(foo)>::function_type_with_signature<Function>;
    static_assert(is_same_v<TheOtherType, TheOtherType3>, "TheOtherType and TheOtherType3 should be the same");

    using TheOtherType4 = function_traits<Bar>::function_type_with_signature<Function>;
    static_assert(is_same_v<TheOtherType, TheOtherType4>, "TheOtherType and TheOtherType4 should be the same");

    std::function<int(int, int)> stdFunc = foo;

    using TheOtherType5 = function_traits<decltype(stdFunc)>::function_type_with_signature<Function>;
    static_assert(is_same_v<TheOtherType, TheOtherType5>, "TheOtherType and TheOtherType5 should be the same");
}
