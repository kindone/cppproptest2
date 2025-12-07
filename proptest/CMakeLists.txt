SET_COMPONENT("RowStore")

ADD_SUBDIRECTORY(generator_ext)

### library
SET(proptest_sources
    util/any.cpp
    util/misc.cpp
    util/unicode.cpp
    util/utf8string.cpp
    util/utf16string.cpp
    util/cesu8string.cpp
    util/printing.cpp
    util/assert.cpp
    shrinker/integral.cpp
    shrinker/bool.cpp
    shrinker/floating.cpp
    shrinker/pair.cpp
    shrinker/listlike.cpp
    shrinker/string.cpp
    shrinker/tuple.cpp
    generator/integral.cpp
    generator/floating.cpp
    generator/bool.cpp
    generator/unicode.cpp
    generator/string.cpp
    generator/utf8string.cpp
    generator/utf16string.cpp
    generator/cesu8string.cpp
    combinator/intervals.cpp
    Stream.cpp
    ShrinkableAny.cpp
    Generator.cpp
    Random.cpp
    PropertyContext.cpp
    PropertyBase.cpp
    util/bitmap.cpp
    instantiate.cpp
    stateful/stateful.cpp
    stateful/concurrency.cpp
)

ADD_HDB_TEST_LIBRARY(proptest
    SHARED
    ${proptest_sources}
    EXPORT_SYMBOL "PROPTEST_DLL_EXPORTS" "PROPTEST_DLL"
    PACKAGE_TESTS_GMOCK
)

TARGET_LINK_LIBRARIES(proptest
    PRIVATE
        hdbbasis
        hdblttbase
)

### tests
SET(proptest_testsources
    test/test_lazy.cpp
    test/test_typelist.cpp
    test/test_function_traits.cpp
    test/test_any.cpp
    test/test_function.cpp
    test/test_anyfunction.cpp
    test/test_tupleorvector.cpp
    test/test_stream.cpp
    test/test_shrinkable.cpp
    test/test_shrinker.cpp
    test/test_shrinker2.cpp
    test/test_anyshrinkable.cpp
    test/test_printing.cpp
    test/test_random.cpp
    test/test_arbitrary.cpp
    test/test_generator.cpp
    test/test_floating.cpp
    test/test_combinator.cpp
    test/test_invoke.cpp
    test/test_property.cpp
    test/test_proptest.cpp
    test/test_stateful_function.cpp
    test/test_concurrency_function.cpp
)


ADD_HDB_TEST(test_proptest
    ${proptest_testsources}
    PACKAGE_TESTS_GMOCK
    ALTERNATIVE GMOCK_HDB_MAIN
    USE_GMOCKUTILS
)


TARGET_LINK_LIBRARIES(test_proptest
    PRIVATE
        proptest
        hdblttbase
        hdbbasement
        hdbbasis
)

ADD_TEST(NAME proptest_gtest COMMAND test_proptest)

## compile_proptest

SET(compile_sources
    test/compile/accumulate.cpp
    test/compile/aggregate.cpp
    test/compile/any.cpp
    test/compile/arbitrary.cpp
    test/compile/concept.cpp
    test/compile/container.cpp
    test/compile/derive.cpp
    test/compile/elementof.cpp
    test/compile/example.cpp
    test/compile/filter.cpp
    test/compile/flatmap.cpp
    test/compile/floating.cpp
    test/compile/integral.cpp
    test/compile/just.cpp
    test/compile/lazy.cpp
    test/compile/map.cpp
    test/compile/oneof.cpp
    test/compile/pair.cpp
    test/compile/printing.cpp
    test/compile/property.cpp
    test/compile/shrinkable.cpp
    test/compile/stream.cpp
    test/compile/string.cpp
    test/compile/tuple.cpp
    test/compile/unicode.cpp
    test/compile/vector.cpp
)

ADD_HDB_TEST(compile_proptest
    EXCLUDE_FROM_ALL
    NO_PACKAGE
    ALTERNATIVE GMOCK_HDB_MAIN
    USE_GMOCKUTILS
    ${compile_sources}
)


TARGET_LINK_LIBRARIES(compile_proptest
    PRIVATE
        proptest
        hdblttbase
        hdbbasement
        hdbbasis
)

