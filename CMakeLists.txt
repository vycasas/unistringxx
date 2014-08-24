cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(unistringxx CXX)

option(UNISTRINGXX_BUILD_LIBRARY "Build as a standalone library." OFF)
option(UNISTRINGXX_BUILD_TESTS "Build testing suites." OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CXX11_OPTION "-std=c++11 -pedantic -pedantic-errors -W -Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CXX11_OPTION "-std=c++11 -pedantic -pedantic-errors -W -Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CXX11_OPTION "-std=c++11 -pedantic -pedantic-errors -W -Wall -Wextra -Werror")
else ()
    message(FATAL_ERROR "Unsupported compiler: \"${CMAKE_CXX_COMPILER_ID}\"")
endif ()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(${CXX11_OPTION} HAVE_CXX11_SUPPORT)

if (HAVE_CXX11_SUPPORT)
    set(CMAKE_CXX_FLAGS "${CXX11_OPTION} ${CMAKE_CXX_FLAGS}")
endif ()

if (UNISTRINGXX_BUILD_LIBRARY)
    option(UNISTRINGXX_BUILD_SHARED "Build as a shared library." ON)
    option(UNISTRINGXX_BUILD_STATIC "Build as a static library." ON)
else ()
    unset(UNISTRINGXX_BUILD_SHARED CACHE)
    unset(UNISTRINGXX_BUILD_STATIC CACHE)
endif ()

include(version.txt)

configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${CMAKE_BINARY_DIR}/config.h
    @ONLY
)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

set(
    UNISTRINGXX_HEADERS
    ${CMAKE_BINARY_DIR}/config.h
    ${CMAKE_SOURCE_DIR}/include/unistringxx/common.hpp
    ${CMAKE_SOURCE_DIR}/include/unistringxx/core.hpp
    ${CMAKE_SOURCE_DIR}/include/unistringxx/uchar.hpp
    ${CMAKE_SOURCE_DIR}/include/unistringxx/ustring.hpp
    ${CMAKE_SOURCE_DIR}/include/unistringxx/utils.hpp
)

if (UNISTRINGXX_BUILD_TESTS)
    enable_testing()

    # Use Google test (https://code.google.com/p/googletest/)
    #add_subdirectory(${CMAKE_SOURCE_DIR}/test/gtest)
    find_package(GTest)
    if (GTEST_FOUND)
        set(GTEST_LIBS ${GTEST_BOTH_LIBRARIES})
    else ()
        message(STATUS "Google Test not found by package, attempting to build from source.")
        if (GTEST_SRC_DIR)
            message(STATUS "Using Google Test source @ \"${GTEST_SRC_DIR}\"")
            # Unfortunately, gtest does not pass compilation without -Wno-missing-field-initializers
            set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
            add_subdirectory(${GTEST_SRC_DIR})
            # Undo warning circumvention for gtest
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")

            set(GTEST_INCLUDE_DIRS ${GTEST_SRC_DIR}/include)
            set(GTEST_LIBS gtest gtest_main)
        else ()
            message(FATAL_ERROR "Cannot find Google Test. Please specify GTEST_SRC_DIR CMake variable.")
        endif ()
    endif ()

    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions(-DUNISTRINGXX_TEST)

    add_executable(header_compile ${CMAKE_SOURCE_DIR}/test/header_compile.cpp ${UNISTRINGXX_HEADERS})
    target_link_libraries(header_compile ${GTEST_LIBS})
    add_test(header_compile header_compile)
    set(CHECK_DEPS ${CHECK_DEPS} header_compile)

    add_executable(core_test ${CMAKE_SOURCE_DIR}/test/core_test.cpp)
    target_link_libraries(core_test ${GTEST_LIBS})
    add_test(core_test core_test)
    set(CHECK_DEPS ${CHECK_DEPS} core_test)

    add_executable(uchar_test ${CMAKE_SOURCE_DIR}/test/uchar_test.cpp)
    target_link_libraries(uchar_test ${GTEST_LIBS})
    add_test(uchar_test uchar_test)
    set(CHECK_DEPS ${CHECK_DEPS} uchar_test)

    add_executable(ustring_test ${CMAKE_SOURCE_DIR}/test/ustring_test.cpp)
    target_link_libraries(ustring_test ${GTEST_LIBS})
    add_test(ustring_test ustring_test)
    set(CHECK_DEPS ${CHECK_DEPS} ustring_test)

    add_custom_target(test_verbose COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS ${CHECK_DEPS})
    add_custom_target(check DEPENDS test_verbose)
endif ()

set(
    UNISTRINGXX_HEADERS_API
    ${CMAKE_SOURCE_DIR}/include/unistringxx/unichar
    ${CMAKE_SOURCE_DIR}/include/unistringxx/unistring
)

install(
    FILES ${UNISTRINGXX_HEADERS} ${UNISTRINGXX_HEADERS_API}
    DESTINATION include/unistringxx
)
