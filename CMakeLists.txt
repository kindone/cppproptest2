
SET(CMAKE_VERBOSE_MAKEFILE ON)
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

PROJECT("cppproptest"  VERSION 1.0)

#SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
SET(CMAKE_CXX_STANDARD 20)
SET(CLANG_DEBUG_OPTIONS, "-g -O0 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")

# Set compiler-specific flags
if(MSVC)
    # MSVC flags: /WX treats warnings as errors (equivalent to -Werror), /W4 is warning level 4
    # /O2 is optimization (same as GCC's -O2)
    # /wd4251 disables C4251 warning about DLL interface for std::string/std::vector etc.
    # /wd4273 disables C4273 warning about inconsistent dll linkage (common with exported classes)
    # These are common MSVC warnings when building DLLs
    # Note: C2491 errors need to be fixed by properly exporting symbols, but suppressing C4273 helps
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W4 /O2 /wd4251 /wd4273")
else()
    # GCC/Clang flags
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Werror -Wnarrowing -Werror=comment -Werror=redundant-decls  -Wpessimizing-move -Werror=unused-local-typedefs -Wsign-compare -Werror=unused-parameter -Werror=dangling-else -Werror=sign-compare -Werror=tautological-compare -Werror=unused-but-set-parameter -Werror=nonnull")
endif()
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

# target_precompile_headers(proptest
#     PUBLIC
#     "proptest/proptest.hpp"
#     "proptest/Generator.hpp"
#     "proptest/util/any.hpp"
#     "proptest/util/tuple.hpp"
#     "proptest/util/Function.hpp"
#     "proptest/Property.hpp"
#     "proptest/PropertyBase.hpp"
#     "proptest/Shrinkable.hpp"
#     "proptest/Stream.hpp"
# )

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
    proptest/test/test_gen.cpp
    proptest/test/test_tupleorvector.cpp
    proptest/test/test_stream.cpp
    proptest/test/test_shrinkable.cpp
    proptest/test/test_shrinker.cpp
    proptest/test/test_shrinker2.cpp
    proptest/test/test_printing.cpp
    proptest/test/test_random.cpp
    proptest/test/test_arbitrary.cpp
    proptest/test/test_generator.cpp
    proptest/test/test_container_variants.cpp
    proptest/test/test_combinator.cpp
    proptest/test/test_combinator2.cpp
    proptest/test/test_invoke.cpp
    proptest/test/test_property.cpp
    proptest/test/test_proptest.cpp
    proptest/test/test_stateful_function.cpp
    proptest/test/test_concurrency_function.cpp
    proptest/test/test_performance.cpp
    proptest/test/test_flatmap_edge_cases.cpp
    proptest/test/test_floating.cpp
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
    proptest/test/compile/generator.cpp
    proptest/test/compile/shrinker.cpp
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

### compile_fail: each .cpp in proptest/test/compile_fail/ must fail to compile.
# Goal: verify that invalid API usage yields useful error messages (e.g. static_assert),
# not just that compilation fails. Each file gets compile_fail_<name> target.
# Build fails (expected); CTest WILL_FAIL so test passes when build fails.
# Add new test: create .cpp file, run cmake to reconfigure.
FILE(GLOB compile_fail_sources "proptest/test/compile_fail/*.cpp")
FOREACH(src ${compile_fail_sources})
    GET_FILENAME_COMPONENT(name ${src} NAME_WE)
    ADD_EXECUTABLE(compile_fail_${name} EXCLUDE_FROM_ALL ${src})
    TARGET_LINK_LIBRARIES(compile_fail_${name} PRIVATE proptest)
    ADD_TEST(NAME compile_fail_${name}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target compile_fail_${name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    SET_TESTS_PROPERTIES(compile_fail_${name} PROPERTIES WILL_FAIL TRUE EXCLUDE_FROM_ALL TRUE)
ENDFOREACH()

SET(test_proptest_mini_sources
    proptest/test/test_mini.cpp
)

ADD_EXECUTABLE(test_proptest_mini
    EXCLUDE_FROM_ALL
    ${test_proptest_mini_sources}
)

TARGET_LINK_LIBRARIES(test_proptest_mini
    PRIVATE
        proptest
	gtest_main
)

ADD_TEST(test_proptest_mini_gtest
    EXCLUDE_FROM_ALL
    test_proptest_mini)
