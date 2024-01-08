
SET(CMAKE_VERBOSE_MAKEFILE ON)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT("cppproptest"  VERSION 1.0)

#SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Werror -Wimplicit-int-conversion -Werror=unused-local-typedefs -Wsign-compare")

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
    proptest/test/test_typelist.cpp
    proptest/test/test_function_traits.cpp
    proptest/test/test_any.cpp
    proptest/test/test_anyfunction.cpp
    proptest/test/test_tupleorvector.cpp
    proptest/test/test_stream.cpp
    proptest/test/test_shrinkable.cpp
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
    proptest/test/compile/compile_any.cpp
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
