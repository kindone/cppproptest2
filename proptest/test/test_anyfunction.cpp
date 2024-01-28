#include "proptest/util/anyfunction.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(FunctionNHolderConst, fptr)
{
    auto lambda0 = +[]() { return 1; };
    FunctionNHolderConst<decltype(lambda0),int> holder0(util::forward<decltype(lambda0)>(lambda0));
    EXPECT_EQ(holder0(), 1);
    auto lambda = +[](int a, int b) { return a+b; };
    FunctionNHolderConst<decltype(lambda),int,int,int> holder(util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(holder(1,2), 3);
    EXPECT_EQ(holder.apply({Any(1),Any(2)}).getRef<int>(), 3);
}

TEST(FunctionNHolderConst, fptr_void)
{
    auto lambda0 = +[]() { return; };
    FunctionNHolderConst<decltype(lambda0),void> holder0(util::forward<decltype(lambda0)>(lambda0));
    holder0();
    auto lambda = +[](int a, int b) { return; };
    FunctionNHolderConst<decltype(lambda),void,int,int> holder(util::forward<decltype(lambda)>(lambda));
    holder(1,2);
    holder.apply({Any(1),Any(2)});
}

TEST(FunctionNHolderConst, functor)
{
    struct Functor {
        int operator()(int a, int b) { return a+b; }
    };

    Functor functor;
    FunctionNHolderMutable<Functor,int,int,int> holder(functor);
    EXPECT_EQ(holder(1,2), 3);
    EXPECT_EQ(holder.apply({Any(1),Any(2)}).getRef<int>(), 3);
}

TEST(FunctionNHolderConst, lambda)
{
    auto lambda0 = []() { return 1; };
    FunctionNHolderConst<decltype(lambda0),int> holder0(util::forward<decltype(lambda0)>(lambda0));
    EXPECT_EQ(holder0(), 1);
    auto lambda = [](int a, int b) { return a+b; };
    FunctionNHolderConst<decltype(lambda),int,int,int> holder(util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(holder(1,2), 3);
    EXPECT_EQ(holder.apply({Any(1),Any(2)}).getRef<int>(), 3);
}

TEST(FunctionNHolderConst, lambda_with_capture)
{
    auto factory = [](int x) {
        auto lambda = [x](int a, int b) { return a+b+x; };
        return lambda;
    };
    auto lambda = factory(1);
    FunctionNHolderConst<decltype(lambda),int,int,int> holder(util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(holder(1,2), 4);
    EXPECT_EQ(holder.apply({Any(1),Any(2)}).getRef<int>(), 4);
}

TEST(FunctionNHolderConst, ptr_args)
{
    auto lambda = [](int* a, int b) { return *a+b; };
    FunctionNHolderConst<decltype(lambda),int,int*,int> holder(util::forward<decltype(lambda)>(lambda));
    int x = 1;
    int y = 2;
    EXPECT_EQ(holder(&x,y), 3);
    EXPECT_EQ(holder.apply({Any(&x),Any(y)}).getRef<int>(), 3);
}

TEST(FunctionNHolderConst, reference_args)
{
    auto lambda = [](int& a, int b) { a += b; return a; };
    FunctionNHolderConst<decltype(lambda),int,int&,int> holder(util::forward<decltype(lambda)>(lambda));
    int x = 1;
    int y = 2;
    EXPECT_EQ(holder(x,y), 3);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(holder.apply({Any(x),Any(y)}).getRef<int>(), 5);
    EXPECT_EQ(x, 3); // x does not change
}

TEST(FunctionNHolder, ptr)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<FunctionNHolder<int(int,int)>> lambda0 = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ((*lambda0)(1,2), 3);
    EXPECT_EQ(lambda0->invoke(Any(1),Any(2)).getRef<int>(), 3);
}

TEST(Function, fptr)
{
    auto lambda0 = +[]() { return 1; };
    Function<int()> function0(lambda0);
    EXPECT_EQ(function0(), 1);
    auto lambda = +[](int a, int b) { return a+b; };
    Function<int(int,int)> function(lambda);
    EXPECT_EQ(function(1,2), 3);

    Function<int(int,int)> function2([](int a, int b) { return a+b; });
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, lambda)
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

TEST(Function, ptr_args_mutable)
{
    auto lambda = [](int* a, int b) { return *a+b; };
    Function<int(int*,int)> function(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ(function(&x,y), 3);
}

TEST(Function, reference_args_mutable)
{
    auto lambda = [](int& a, int b) { a = a + b; return a + b; };
    Function<int(int&,int)> function(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ(function(x,y), 5);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(function(x,y), 7);
    EXPECT_EQ(x, 5);
}

TEST(Function, make_function)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = util::make_function(lambda);
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
    shared_ptr<FunctionHolder> functionNHolderImpl = util::make_shared<FunctionNHolderConst<decltype(lambda),int,int,int>>(util::forward<decltype(lambda)>(lambda));
    Function<int(int,int)> function(functionNHolderImpl);
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, copy_and_reset_original)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = util::make_function(lambda);
    Function<int(int,int)> function2(function);
    EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(int,int)>> function3 = util::make_shared<Function<int(int,int)>>(util::make_function(lambda));
    Function<int(int,int)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(1,2), 3);
}

TEST(Function, void_return)
{
    auto lambda = [](int a, int b) { return; };
    Function<void(int,int)> function = util::make_function(lambda);
    function(1,2);
}

TEST(Function, noncopyable_args)
{
    struct NonCopyable {
        NonCopyable() = default;
        NonCopyable(const NonCopyable&) = delete;
    };
    auto lambda = [](const NonCopyable& a, const NonCopyable& b) { return 0; };
    Function<int(const NonCopyable&, const NonCopyable&)> function = util::make_function(lambda);
    function(NonCopyable(), NonCopyable());
}

TEST(Function, Any_as_parameter)
{
    auto lambda = [](Any a, Any b) -> int { return a.getRef<int>() + b.getRef<int>(); };
    Function<int(Any,Any)> function = util::make_function(lambda);
    Function<int(Any,Any)> function2(function);
    EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(Any,Any)>> function3 = util::make_shared<Function<int(Any,Any)>>(util::make_function(lambda));
    Function<int(Any,Any)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(Any(1),Any(2)), 3);
}

TEST(Function, Any_reference_as_parameter)
{
    auto lambda = [](const Any& a, const Any& b) -> int { return a.getRef<int>() + b.getRef<int>(); };
    Function<int(const Any&,const Any&)> function = util::make_function(lambda);
    Function<int(const Any&,const Any&)> function2(function);
    // EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(const Any&,const Any&)>> function3 = util::make_shared<Function<int(const Any&,const Any&)>>(util::make_function(lambda));
    Function<int(const Any&,const Any&)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(Any(1),Any(2)), 3);
}

TEST(Function, from_AnyFunction)
{
    auto lambda = [](int a, int b) { return a+b; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    Function<int(int,int)> function = anyFunction;
    EXPECT_EQ(function(1,2), 3);
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, from_incompatible_AnyFunction)
{
    auto lambda = [](int a, int b) { return a+b; };
    AnyFunction anyFunction = util::make_anyfunction(lambda);
    Function<int(int,int,int)> function = anyFunction;
    EXPECT_THROW(function(1,2,3), invalid_argument);
    Function<int(int,string)> function2 = anyFunction;
    EXPECT_THROW(function2(1,"2"), invalid_cast_error);
    Function<int(int,double)> function3 = anyFunction;
    EXPECT_THROW(function3(1, 2.0), invalid_cast_error);
}

TEST(Function, in_capture)
{
    auto outerFactory = [](const Function<int(int,int)>& inner) {
        return [inner](int a, int b) -> int { return inner(a, b); };
    };
    auto factory = [](int a) {
        auto lambda = [a](int b) -> int {
            cout << "captured: " << a << ", " << &a << endl;
            return a+b;
        };
        auto lambda2 = lambda;
        EXPECT_EQ(lambda2(a), 2*a);
        cout << "lambda2(a) = " << lambda2(a) << ", sizeof: " << sizeof(lambda) << endl;
        return Function<int(int)>(lambda);
    };

    auto factory2 = [](int a) {
        auto lambda = [a](int b) -> int { return a+b; };
        return FunctionNHolderConst<decltype(lambda),int,int>(util::forward<decltype(lambda)>(lambda));
    };
    auto functionx = factory2(1);
    EXPECT_EQ(functionx(2), 3);
    Function<int(int)> function0 = factory(1);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);

    Function<int(int)> function0_2 = function0;
    EXPECT_EQ(function0_2(2), 3);
    Function<int(int)> function0_3 = function0_2;
    EXPECT_EQ(function0_2(2), function0_3(2));
    EXPECT_EQ(function0(2), 3);
    cout << "all ok so far" << endl;
    Function<int(int,int)> function = [function0](int a, int b) { return function0(a) + b; };
    EXPECT_EQ(function(1,2), 4);
    Function<int(int,int)> function2 = outerFactory(function);
    EXPECT_EQ(function2(1,2), 4);
}

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
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int>>(
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
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderMutable<Functor,int,int,int>>(functor);
    EXPECT_EQ(anyFunctionN->invoke(Any(1),Any(2)).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->call<int>(1,2), 3);
}

TEST(AnyFunctionNHolder, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<AnyFunctionNHolder<2>> anyFunctionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    EXPECT_EQ(anyFunctionN->invoke(Any(1),Any(2)).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunctionN->call<int>(1,2), 3);
}

TEST(FunctionHolder, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    unique_ptr<FunctionNHolder<int(int, int)>> functionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),int,int,int>>(
        util::forward<decltype(lambda)>(lambda));
    unique_ptr<FunctionHolder> anyFunction = util::move(functionN);
    EXPECT_EQ(anyFunction->apply({Any(1),Any(2)}).getRef<int>(), 3);
    EXPECT_EQ(anyFunction->call<int>(1,2), 3);
}

TEST(FunctionHolder, void)
{
    auto lambda = [](int a, int b) { return; };
    unique_ptr<FunctionNHolder<void(int, int)>> functionN = util::make_unique<FunctionNHolderMutable<decltype(lambda),void,int,int>>(
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
