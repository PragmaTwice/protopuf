#   Copyright 2020-2021 PragmaTwice
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

cmake_minimum_required(VERSION 3.16)
project(protopuf)

message(NOTICE "[TIP] Expected a compiler with C++20 support (i.e. GCC 11, Clang 12 or above)")
message(NOTICE "[TIP] Check out https://github.com/PragmaTwice/protopuf for more details")

set(CXX_STANDARD_REQUIRED ON)

add_library(protopuf INTERFACE)
target_include_directories(protopuf INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_features(protopuf INTERFACE cxx_std_20)

set(GTEST_REPO "https://github.com/google/googletest" CACHE STRING "url of GoogleTest repository")

option(DOWNLOAD_GTEST "Download the git repo of GoogleTest, build and use the master branch version" ON)
option(BUILD_TESTS "Build unit testings" ON)
option(ENABLE_COMPATIBILITY_TEST "Build compatibility testing between protopuf and protobuf" OFF)
option(ENABLE_BENCHMARK "Build benchmark testing between protopuf and protobuf (requires Release mode)" OFF)

if(BUILD_TESTS)
    if(DOWNLOAD_GTEST)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL ${GTEST_REPO}/archive/refs/tags/release-1.11.0.zip
        )

        set(BUILD_GMOCK OFF CACHE BOOL "")
        set(INSTALL_GTEST OFF CACHE BOOL "")

        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(googletest)

        set(GTEST_LIBS gtest gtest_main)
    else()
        set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
        set(THREADS_PREFER_PTHREAD_FLAG TRUE)

        find_package(Threads REQUIRED)
        find_package(GTest CONFIG REQUIRED)
        message(NOTICE "-- Found GTest: " ${GTest_DIR})

        set(GTEST_LIBS GTest::gtest GTest::gtest_main)
    endif()
    
    enable_testing()

    file(GLOB TESTS test/*.cpp)
    add_executable(protopuf_test ${TESTS})
    target_include_directories(protopuf_test PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(protopuf_test protopuf ${CMAKE_THREAD_LIBS_INIT} ${GTEST_LIBS})

    include(GoogleTest)
    gtest_discover_tests(protopuf_test)

    if(ENABLE_COMPATIBILITY_TEST)
        add_subdirectory(test/compatibility)
    endif()

    if(ENABLE_BENCHMARK)
        add_subdirectory(test/benchmark)
    endif()
endif()

install(DIRECTORY include DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(TARGETS protopuf EXPORT protopufConfig)
export(TARGETS protopuf FILE "${CMAKE_CURRENT_BINARY_DIR}/protopufConfig.cmake")
install(EXPORT protopufConfig DESTINATION "${CMAKE_INSTALL_PREFIX}/share/protopuf")
