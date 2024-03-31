#include "proptest/util/function.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

/*
TEST(AnyFunctionHolderHelper, basic)
{
    struct F1 : public util::FunctionHolderHelper_t<make_index_sequence<1>>::type {
        Any invoke(Any arg) const override {
            return Any(arg.getRef<int>()+1);
        }
        Any invoke(Any arg) override {
            return Any(arg.getRef<int>()+1);
        }
    };

    F1 f1;
    ASSERT_EQ(F1::N, 1);
    EXPECT_EQ(f1.invoke(Any(1)).getRef<int>(), 2);

    struct F2 : public util::FunctionHolderHelper_t<make_index_sequence<2>>::type {
        Any invoke(Any arg1, Any arg2) const override {
            return Any(arg1.getRef<int>()+arg2.getRef<int>());
        }

        Any invoke(Any arg1, Any arg2) override {
            return Any(arg1.getRef<int>()+arg2.getRef<int>());
        }
    };

    F2 f2;
    ASSERT_EQ(F2::N, 2);
    EXPECT_EQ(f2.invoke(Any(1), Any(3)).getRef<int>(), 4);
}

TEST(AnyFunctionNHolder, fptr)
{
    auto lambda = +[](int a, int b) { return a+b; };
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(anyFunctionN->invoke(Any(1),Any(2)).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->call<int>(1,2), 3);
}

TEST(AnyFunctionNHolder, functor)
{
    struct Functor {
        int operator()(int a, int b) { return a+b; }
    };

    Functor functor;
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderMutable<Functor,int,int,int,int>>(functor);
    EXPECT_EQ(anyFunctionN->invoke(Any(1),Any(2)).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->call<int>(1,2), 3);
}

TEST(AnyFunctionNHolder, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(anyFunctionN->invoke(Any(1),Any(2)).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->call<int>(1,2), 3);
}

TEST(FunctionHolder, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<FunctionNHolder<int,int(int, int)>> functionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    unique_ptr<FunctionHolder> anyFunction = util::move(functionN);
    EXPECT_EQ(anyFunction->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction->call<int>(1,2), 3);
}

TEST(FunctionHolder, void)
{
    auto lambda = [](int, int) { return; };
    unique_ptr<FunctionNHolder<void,void(int, int)>> functionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),void,void,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    unique_ptr<FunctionHolder> anyFunction = util::move(functionN);
    anyFunction->apply({Any(1),Any(2)});
    anyFunction->call<void>(1,2);
}

TEST(AnyFunction, from_Function)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = util::make_function(lambda);
    AnyFunction anyFunction = function;
    EXPECT_EQ(anyFunction.apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction.call<int>(1,2), 3);
}

TEST(AnyFunction, make_anyfunction_primitives)
{
    auto lambda = [](int a, int b) { return a+b; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    EXPECT_EQ(anyFunction.apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction.call<int>(1,2), 3);
}

TEST(AnyFunction, make_anyfunction_ptr)
{
    auto lambda = [](int *a, int *b) { return *a+*b; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ(anyFunction.call<int>(&x,&y), 3);
    EXPECT_EQ(anyFunction.apply({Any(&x),Any(&y)}).getRef<int>(), 3);
}

TEST(AnyFunction, make_anyfunction_ptr_mutable)
{
    auto lambda = [](int *a, int *b) { return *a = *a+*b; return *a; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ(anyFunction.call<int>(&x,&y), 3);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(anyFunction.apply({Any(&x),Any(&y)}).getRef<int>(), 5);
    EXPECT_EQ(x, 5);
}

TEST(AnyFunction, make_anyfunction_reference_mutable)
{
    auto lambda = [](int &a, int &b) { return a = a+b; return a; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ((anyFunction.call<int, int&, int&>(x,y)), 3); // need & in the call type params
    EXPECT_EQ(x, 3);
    EXPECT_EQ(anyFunction.apply({util::make_any<int&>(x), util::make_any<int&>(y)}).getRef<int>(), 5);
    EXPECT_EQ(x, 5);
}

TEST(AnyFunction, copy)
{
    auto lambda = [](int a, int b) { return a+b; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    AnyFunction anyFunction2 = anyFunction;
    EXPECT_EQ(anyFunction2.apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction2.call<int>(1,2), 3);
}
*/
