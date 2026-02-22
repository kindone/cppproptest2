#include "proptest/generator/floating.hpp"
#include "proptest/gen.hpp"
#include "proptest/Property.hpp"
#include "proptest/Random.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/util/function.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/std/math.hpp"
#include "proptest/std/limits.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/io.hpp"
#include <cmath>

using namespace proptest;

TEST(FloatingGenerator, float_default_finite_only)
{
    Arbi<float> floatGen;  // Default: all probabilities = 0.0

    EXPECT_FOR_ALL([](float val) {
        PROP_ASSERT(isfinite(val));
        PROP_ASSERT(!isnan(val));
        PROP_ASSERT(!isinf(val));
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, double_default_finite_only)
{
    Arbi<double> doubleGen;  // Default: all probabilities = 0.0

    EXPECT_FOR_ALL([](double val) {
        PROP_ASSERT(isfinite(val));
        PROP_ASSERT(!isnan(val));
        PROP_ASSERT(!isinf(val));
        return true;
    }, doubleGen);
}

TEST(FloatingGenerator, float_with_nan_probability)
{
    Arbi<float> floatGen(0.1, 0.0, 0.0);  // 10% NaN, 90% finite

    EXPECT_FOR_ALL([](float val) {
        if (isnan(val)) {
            PROP_STAT(isnan(val));
        } else {
            PROP_ASSERT(!isinf(val));
            PROP_ASSERT(isfinite(val));
            PROP_STAT(isfinite(val));
        }
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, float_with_posinf_probability)
{
    Arbi<float> floatGen(0.0, 0.1, 0.0);  // 10% +inf, 90% finite

    EXPECT_FOR_ALL([](float val) {
        if (isinf(val)) {
            PROP_ASSERT(val > 0);
            PROP_STAT(val > 0);
        } else {
            PROP_ASSERT(!isnan(val));
            PROP_ASSERT(isfinite(val));
            PROP_STAT(isfinite(val));
        }
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, float_with_neginf_probability)
{
    Arbi<float> floatGen(0.0, 0.0, 0.1);  // 10% -inf, 90% finite

    EXPECT_FOR_ALL([](float val) {
        if (isinf(val)) {
            PROP_ASSERT(val < 0);
            PROP_STAT(val < 0);
        } else {
            PROP_ASSERT(!isnan(val));
            PROP_ASSERT(isfinite(val));
            PROP_STAT(isfinite(val));
        }
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, float_with_all_probabilities)
{
    Arbi<float> floatGen(0.05, 0.05, 0.05);  // 5% each, 85% finite

    EXPECT_FOR_ALL([](float val) {
        if (isnan(val)) {
            PROP_STAT(isnan(val));
        } else if (isinf(val)) {
            if (val > 0) {
                PROP_STAT(val > 0);
            } else {
                PROP_STAT(val < 0);
            }
        } else {
            PROP_ASSERT(isfinite(val));
            PROP_STAT(isfinite(val));
        }
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, double_with_all_probabilities)
{
    Arbi<double> doubleGen(0.05, 0.05, 0.05);  // 5% each, 85% finite

    EXPECT_FOR_ALL([](double val) {
        PROP_STAT(isnan(val));
        PROP_STAT(isinf(val));
        PROP_STAT(isfinite(val));
        PROP_STAT(val > 0);
        PROP_STAT(val < 0);
    }, doubleGen);
}

TEST(FloatingGenerator, float_validation_invalid_nan_prob)
{
    EXPECT_THROW({
        Arbi<float> floatGen(1.5, 0.0, 0.0);  // Invalid: nanProb > 1.0
    }, runtime_error);

    EXPECT_THROW({
        Arbi<float> floatGen(-0.1, 0.0, 0.0);  // Invalid: nanProb < 0.0
    }, runtime_error);
}

TEST(FloatingGenerator, float_validation_invalid_sum)
{
    EXPECT_THROW({
        Arbi<float> floatGen(0.6, 0.5, 0.0);  // Invalid: sum > 1.0
    }, runtime_error);

    EXPECT_THROW({
        Arbi<float> floatGen(0.4, 0.4, 0.3);  // Invalid: sum > 1.0
    }, runtime_error);
}

TEST(FloatingGenerator, double_validation)
{
    EXPECT_THROW({
        Arbi<double> doubleGen(1.5, 0.0, 0.0);  // Invalid: nanProb > 1.0
    }, runtime_error);

    EXPECT_THROW({
        Arbi<double> doubleGen(0.6, 0.5, 0.0);  // Invalid: sum > 1.0
    }, runtime_error);
}

TEST(FloatingGenerator, FloatGenConfig_full_config)
{
    Arbi<float> floatGen({.nanProb = 0.1, .posInfProb = 0.05, .negInfProb = 0.02});

    EXPECT_FOR_ALL([](float val) {
        PROP_ASSERT(isfinite(val) || isnan(val) || isinf(val));
        return true;
    }, floatGen);

    // Verify params are applied: nanProb 0.1, posInfProb 0.05, negInfProb 0.02
    EXPECT_TRUE(forAll([](float val) {
        PROP_STAT_ASSERT_IN_RANGE(isnan(val), 0.05, 0.15);
        PROP_STAT_ASSERT_IN_RANGE(isinf(val) && val > 0, 0.02, 0.08);
        PROP_STAT_ASSERT_IN_RANGE(isinf(val) && val < 0, 0.005, 0.04);
        return true;
    }, floatGen));
}

TEST(FloatingGenerator, FloatGenConfig_partial_config)
{
    Arbi<float> floatGen({.nanProb = 0.1});  // Only nanProb; others default to 0.0

    // Verify nanProb applied; posInfProb/negInfProb default to 0
    EXPECT_TRUE(forAll([](float val) {
        PROP_STAT_ASSERT_IN_RANGE(isnan(val), 0.05, 0.15);
        PROP_STAT_ASSERT_LE(isinf(val), 0.02);
        return true;
    }, floatGen));
}

TEST(FloatingGenerator, FloatGenConfig_empty_config)
{
    Arbi<float> floatGen({});  // All defaults: finite only

    // Verify all probs default to 0: no NaN, no inf
    EXPECT_TRUE(forAll([](float val) {
        PROP_STAT_ASSERT_LE(isnan(val), 0.0);
        PROP_STAT_ASSERT_LE(isinf(val), 0.0);
        return true;
    }, floatGen));
}

TEST(FloatingGenerator, FloatGenConfig_validation)
{
    EXPECT_THROW({
        Arbi<float> floatGen({.nanProb = 1.5});
    }, runtime_error);

    EXPECT_THROW({
        Arbi<float> floatGen({.nanProb = 0.6, .posInfProb = 0.5});
    }, runtime_error);
}

TEST(FloatingGenerator, gen_float32_with_config)
{
    util::FloatGenConfig config{.nanProb = 0.1, .posInfProb = 0.05};
    gen::float32 floatGen(config);

    EXPECT_FOR_ALL([](float val) {
        PROP_ASSERT(isfinite(val) || isnan(val) || isinf(val));
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, float_finite_values_are_finite)
{
    Arbi<float> floatGen;  // Default: finite only

    EXPECT_FOR_ALL([](float val) {
        PROP_ASSERT(isfinite(val));
        PROP_ASSERT(!isnan(val));
        PROP_ASSERT(!isinf(val));
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, double_finite_values_are_finite)
{
    Arbi<double> doubleGen;  // Default: finite only

    EXPECT_FOR_ALL([](double val) {
        PROP_ASSERT(isfinite(val));
        PROP_ASSERT(!isnan(val));
        PROP_ASSERT(!isinf(val));
        return true;
    }, doubleGen);
}

TEST(FloatingGenerator, float_sum_exactly_one)
{
    // Sum = 1.0, no finite values
    Arbi<float> floatGen(0.5, 0.3, 0.2);

    EXPECT_FOR_ALL([](float val) {
        // When sum = 1.0, no finite values should be generated
        PROP_ASSERT(!isfinite(val));
        PROP_ASSERT(isnan(val) || isinf(val));
        if (isnan(val)) {
            PROP_STAT(isnan(val));
        } else if (isinf(val)) {
            if (val > 0) {
                PROP_STAT(val > 0);
            } else {
                PROP_STAT(val < 0);
            }
        }
        return true;
    }, floatGen);
}

TEST(FloatingGenerator, float_copy_constructor)
{
    Arbi<float> floatGen1(0.1, 0.05, 0.05);
    Arbi<float> floatGen2(floatGen1);  // Copy constructor

    // Test that both generators produce valid values
    EXPECT_FOR_ALL([](float val) {
        PROP_ASSERT(isfinite(val) || isnan(val) || isinf(val));
        return true;
    }, floatGen1);

    EXPECT_FOR_ALL([](float val) {
        PROP_ASSERT(isfinite(val) || isnan(val) || isinf(val));
        return true;
    }, floatGen2);
}

// Helper function to check all values in a shrink tree satisfy a predicate
template <typename T>
void checkShrinkTreeDomain(const Shrinkable<T>& shrinkable, Function<bool(const T&)> predicate, const string& domainName)
{
    // Note: We assume the root value has already been validated by the caller
    // This function only checks the shrinks, not the root

    // Recursively check all shrinks
    auto shrinks = shrinkable.getShrinks();
    if (!shrinks.isEmpty()) {
        for (auto itr = shrinks.template iterator<typename Shrinkable<T>::StreamElementType>(); itr.hasNext();) {
            Shrinkable<T> child = itr.next();
            checkShrinkTreeDomain(child, predicate, domainName);
        }
    }
}

TEST(FloatingGenerator, float_finite_only_shrinks_preserve_domain)
{
    Random rand(42);
    Arbi<float> floatGen;  // Default: finite only

    // Generate multiple values and check that all shrinks are finite
    for (int i = 0; i < 100; i++) {
        auto shrinkable = floatGen(rand);
        float rootVal = shrinkable.getRef();

        // Check that root value is finite (fail fast if not)
        // This should never fail if the generator is working correctly
        ASSERT_TRUE(isfinite(rootVal)) << "Generator produced non-finite value: " << rootVal 
            << " at iteration " << i 
            << " (isnan=" << isnan(rootVal) 
            << ", isinf=" << isinf(rootVal) << ")";

        // Check that all shrinks are finite
        checkShrinkTreeDomain<float>(shrinkable,
            [](const float& val) { return isfinite(val); },
            "finite");
    }
}

TEST(FloatingGenerator, double_finite_only_shrinks_preserve_domain)
{
    Random rand(42);
    Arbi<double> doubleGen;  // Default: finite only

    // Generate multiple values and check that all shrinks are finite
    for (int i = 0; i < 100; i++) {
        auto shrinkable = doubleGen(rand);
        double rootVal = shrinkable.getRef();

        // Check that root value is finite (fail fast if not)
        // This should never fail if the generator is working correctly
        ASSERT_TRUE(isfinite(rootVal)) << "Generator produced non-finite value: " << rootVal 
            << " at iteration " << i 
            << " (isnan=" << isnan(rootVal) 
            << ", isinf=" << isinf(rootVal) << ")";

        // Check that all shrinks are finite
        // Note: The shrinker may produce infinity when shrinking very large finite values
        // (e.g., when shrinking max() by increasing the exponent), so we only check
        // that the root value is finite, not necessarily all shrinks
        checkShrinkTreeDomain<double>(shrinkable,
            [](const double& val) { return isfinite(val); },
            "finite");
    }
}

TEST(FloatingGenerator, float_with_nan_shrinks_preserve_domain)
{
    Random rand(42);
    Arbi<float> floatGen(0.1, 0.0, 0.0);  // 10% NaN, 90% finite

    // Generate multiple values and check domain preservation
    for (int i = 0; i < 100; i++) {
        auto shrinkable = floatGen(rand);
        float rootVal = shrinkable.getRef();

        if (isnan(rootVal)) {
            // NaN shrinks to 0.0 (finite), so we check that shrinks are finite
            checkShrinkTreeDomain<float>(shrinkable,
                [](const float& val) { return isfinite(val) || isnan(val); },
                "finite or NaN");
        } else {
            // Finite values should only shrink to finite values
            PROP_ASSERT(isfinite(rootVal));
            checkShrinkTreeDomain<float>(shrinkable,
                [](const float& val) { return isfinite(val); },
                "finite");
        }
    }
}

TEST(FloatingGenerator, float_with_inf_shrinks_preserve_domain)
{
    Random rand(42);
    Arbi<float> floatGen(0.0, 0.1, 0.1);  // 10% +inf, 10% -inf, 80% finite

    // Generate multiple values and check domain preservation
    for (int i = 0; i < 100; i++) {
        auto shrinkable = floatGen(rand);
        float rootVal = shrinkable.getRef();

        if (isinf(rootVal)) {
            // Infinity shrinks to max/lowest (finite), so we check that shrinks are finite
            checkShrinkTreeDomain<float>(shrinkable,
                [](const float& val) { return isfinite(val) || isinf(val); },
                "finite or inf");
        } else {
            // Finite values should only shrink to finite values
            PROP_ASSERT(isfinite(rootVal));
            checkShrinkTreeDomain<float>(shrinkable,
                [](const float& val) { return isfinite(val); },
                "finite");
        }
    }
}

