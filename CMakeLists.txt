
SET(CMAKE_VERBOSE_MAKEFILE ON)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT("cppproptest"  VERSION 1.0)

#SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
SET(CMAKE_CXX_STANDARD 20)
SET(CLANG_DEBUG_OPTIONS, "-g -O0 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Werror -Wnarrowing -Werror=comment -Werror=redundant-decls  -Wpessimizing-move -Werror=unused-local-typedefs -Wsign-compare -Werror=unused-parameter -Werror=dangling-else -Werror=sign-compare -Werror=tautological-compare -Werror=unused-but-set-parameter -Werror=nonnull")
# add_subdirectory("proptest")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#include_directories(. googletest/googletest/include googletest/googlemock/include/)
include_directories(.)

### library
SET(proptest_sources
    proptest/util/any.cpp
    proptest/util/misc.cpp
    proptest/Stream.cpp
    proptest/Shrinkable.cpp
    proptest/Generator.cpp
    proptest/Random.cpp
    proptest/util/unicode.cpp
    proptest/util/utf8string.cpp
    proptest/util/utf16string.cpp
    proptest/util/cesu8string.cpp
    proptest/util/printing.cpp
    proptest/util/assert.cpp
    proptest/shrinker/integral.cpp
    proptest/shrinker/bool.cpp
    proptest/shrinker/floating.cpp
    proptest/shrinker/pair.cpp
    proptest/shrinker/listlike.cpp
    proptest/shrinker/string.cpp
    proptest/shrinker/tuple.cpp
    proptest/generator/integral.cpp
    proptest/generator/floating.cpp
    proptest/generator/bool.cpp
    proptest/generator/unicode.cpp
    proptest/generator/string.cpp
    proptest/generator/utf8string.cpp
    proptest/generator/utf16string.cpp
    proptest/generator/cesu8string.cpp
    proptest/combinator/filter.cpp
    proptest/combinator/transform.cpp
    proptest/combinator/dependency.cpp
    proptest/combinator/derive.cpp
    proptest/combinator/chain.cpp
    proptest/combinator/intervals.cpp
    proptest/combinator/aggregate.cpp
    proptest/combinator/oneof.cpp
    proptest/combinator/accumulate.cpp
    proptest/PropertyContext.cpp
    proptest/PropertyBase.cpp
    proptest/util/bitmap.cpp
    proptest/instantiate.cpp
)

ADD_LIBRARY(proptest
    SHARED
    ${proptest_sources}
    #NO_PACKAGE
)

set_target_properties(proptest PROPERTIES
	COMPILE_FLAGS "-DPROPTEST_DLL -DPROTEST_DLL_EXPORTS")

TARGET_LINK_LIBRARIES(proptest
    PRIVATE
)

#TARGET_PRECOMPILE_HEADERS(proptest
#    PUBLIC
#    proptest.hpp
#    PRIVATE
#)

### tests
SET(proptest_testsources
    proptest/test/test_lazy.cpp
    proptest/test/test_typelist.cpp
    proptest/test/test_function_traits.cpp
    proptest/test/test_any.cpp
    proptest/test/test_anyfunction.cpp
    proptest/test/test_function.cpp
    proptest/test/test_tupleorvector.cpp
    proptest/test/test_stream.cpp
    proptest/test/test_shrinkable.cpp
    proptest/test/test_shrinker.cpp
    proptest/test/test_shrinker2.cpp
    proptest/test/test_printing.cpp
    proptest/test/test_random.cpp
    proptest/test/test_arbitrary.cpp
    proptest/test/test_generator.cpp
    proptest/test/test_combinator.cpp
    proptest/test/test_combinator2.cpp
    proptest/test/test_invoke.cpp
    proptest/test/test_property.cpp
    proptest/test/test_proptest.cpp
    proptest/test/test_stateful_function.cpp
    proptest/test/test_concurrency_function.cpp
    proptest/test/test_performance.cpp
    proptest/test/testutil.cpp
)


ADD_EXECUTABLE(test_proptest
    ${proptest_testsources}
)

TARGET_LINK_LIBRARIES(test_proptest
    PRIVATE
        proptest
	gtest_main
	gmock_main
)

ADD_TEST(NAME proptest_gtest COMMAND test_proptest)

### compile
SET(compile_sources
    proptest/test/compile/accumulate.cpp
    proptest/test/compile/aggregate.cpp
    proptest/test/compile/any.cpp
    proptest/test/compile/arbitrary.cpp
    proptest/test/compile/concept.cpp
    proptest/test/compile/container.cpp
    proptest/test/compile/derive.cpp
    proptest/test/compile/elementof.cpp
    proptest/test/compile/example.cpp
    proptest/test/compile/filter.cpp
    proptest/test/compile/flatmap.cpp
    proptest/test/compile/floating.cpp
    proptest/test/compile/integral.cpp
    proptest/test/compile/just.cpp
    proptest/test/compile/lazy.cpp
    proptest/test/compile/map.cpp
    proptest/test/compile/oneof.cpp
    proptest/test/compile/pair.cpp
    proptest/test/compile/printing.cpp
    proptest/test/compile/property.cpp
    proptest/test/compile/shrinkable.cpp
    proptest/test/compile/stream.cpp
    proptest/test/compile/string.cpp
    proptest/test/compile/tuple.cpp
    proptest/test/compile/unicode.cpp
    proptest/test/compile/vector.cpp
)

ADD_EXECUTABLE(compile_proptest
    EXCLUDE_FROM_ALL
    ${compile_sources}
)

TARGET_LINK_LIBRARIES(compile_proptest
    PRIVATE
        proptest
	gtest_main
)

ADD_TEST(compile_proptest_gtest
    EXCLUDE_FROM_ALL
    compile_proptest)
