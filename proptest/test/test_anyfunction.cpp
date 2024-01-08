#include "proptest/util/anyfunction.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "gtest/gtest.h"

using namespace proptest;

TEST(FunctionNHolderImpl, basic)
{
    auto lambda0 = []() { return 1; };
    FunctionNHolderImpl<decltype(lambda0),int> holder0(util::forward<decltype(lambda0)>(lambda0));
    EXPECT_EQ(holder0(), 1);
    auto lambda = [](int a, int b) { return a+b; };
    FunctionNHolderImpl<decltype(lambda),int,int,int> holder(util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(holder(1,2), 3);
    EXPECT_EQ(holder.apply({Any(1),Any(2)}).getRef<int>(), 3);
}

TEST(FunctionNHolder, ptr)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<FunctionNHolder<int(int,int)>> lambda0 = util::make_unique<FunctionNHolderImpl<decltype(lambda),int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ((*lambda0)(1,2), 3);
    EXPECT_EQ(lambda0->invoke(Any(1),Any(2)).getRef<int>(), 3);
}

TEST(Function, construct)
{
    auto lambda0 = []() { return 1; };
    Function<int()> function0(lambda0);
    EXPECT_EQ(function0(), 1);
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function(lambda);
    EXPECT_EQ(function(1,2), 3);

    Function<int(int,int)> function2([](int a, int b) { return a+b; });
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, make_function)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = make_function(lambda);
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, assign_functor)
{
    struct Functor {
        int operator()(int a, int b) { return a+b; }
    };

    Functor functor;
    Function<int(int,int)> function = functor;
    EXPECT_EQ(function(1,2), 3);
}

int func(int a, int b) { return a+b; }

TEST(Function, assign_function)
{
    Function<int(int,int)> function = func;
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, assign_function_pointer)
{
    int (*fptr)(int a, int b) = func;
    Function<int(int,int)> function = fptr;
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, assign_FunctionNHolderImpl)
{
    auto lambda = [](int a, int b) { return a+b; };
    FunctionNHolderImpl<decltype(lambda),int,int,int> functionNHolderImpl(util::forward<decltype(lambda)>(lambda));
    Function<int(int,int)> function = functionNHolderImpl;
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, copy_and_reset_original)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = make_function(lambda);
    Function<int(int,int)> function2(function);
    EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(int,int)>> function3 = util::make_shared<Function<int(int,int)>>(make_function(lambda));
    Function<int(int,int)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(1,2), 3);
}

TEST(Function, Any_as_parameter)
{
    auto lambda = [](Any a, Any b) -> int { return a.getRef<int>() + b.getRef<int>(); };
    Function<int(Any,Any)> function = make_function(lambda);
    Function<int(Any,Any)> function2(function);
    EXPECT_EQ(function2(1,2), 3);
    // shared_ptr<Function<int(Any,Any)>> function3 = util::make_shared<Function<int(Any,Any)>>(make_function(lambda));
    // Function<int(Any,Any)> function4(*function3);
    // function3.reset();
    // EXPECT_EQ(function4(Any(1),Any(2)), 3);
}

TEST(AnyFunctionHolderHelper, basic)
{
    struct F1 : public util::AnyFunctionHolderHelper_t<make_index_sequence<1>>::type {
        Any invoke(Any arg) const override {
            return Any(arg.getRef<int>()+1);
        }
    };

    F1 f1;
    ASSERT_EQ(F1::N, 1);
    EXPECT_EQ(f1.invoke(Any(1)).getRef<int>(), 2);

    struct F2 : public util::AnyFunctionHolderHelper_t<make_index_sequence<2>>::type {
        Any invoke(Any arg1, Any arg2) const override {
            return Any(arg1.getRef<int>()+arg2.getRef<int>());
        }
    };

    F2 f2;
    ASSERT_EQ(F2::N, 2);
    // EXPECT_EQ(f2(1,3), 4); -- compile error
    EXPECT_EQ(f2.invoke(Any(1), Any(3)).getRef<int>(), 4);
}

TEST(AnyFunctionNHolder, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderImpl<decltype(lambda),int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    // EXPECT_EQ((*lambda0)(1,2), 3); -- compile error
    EXPECT_EQ(anyFunctionN->invoke(Any(1),Any(2)).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->apply(vector<Any>{Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->call<int>(1,2), 3);
}

TEST(AnyFunctionHolder, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<FunctionNHolder<int(int, int)>> functionN = util::make_unique<FunctionNHolderImpl<decltype(lambda),int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    unique_ptr<AnyFunctionHolder> anyFunction = util::move(functionN);
    // EXPECT_EQ((*lambda0)(1,2), 3); -- compile error
    // EXPECT_EQ((*anyFunction)(Any(1),Any(2)).getRef<int>(), 3); -- compile error
    EXPECT_EQ(anyFunction->apply(vector<Any>{Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction->call<int>(1,2), 3);
}

TEST(AnyFunction, from_Function)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = make_function(lambda);
    AnyFunction anyFunction = function;
    EXPECT_EQ(anyFunction.apply(vector<Any>{Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction.call<int>(1,2), 3);
}

TEST(AnyFunction, make_any_function)
{
    auto lambda = [](int a, int b) { return a+b; };
    AnyFunction anyFunction = make_any_function(lambda);
    EXPECT_EQ(anyFunction.apply(vector<Any>{Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction.call<int>(1,2), 3);
}

TEST(AnyFunction, copy)
{
    auto lambda = [](int a, int b) { return a+b; };
    AnyFunction anyFunction = make_any_function(lambda);
    AnyFunction anyFunction2 = anyFunction;
    EXPECT_EQ(anyFunction2.apply(vector<Any>{Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction2.call<int>(1,2), 3);
}
