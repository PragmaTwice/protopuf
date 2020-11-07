#   Copyright 2020 PragmaTwice
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

cmake_minimum_required(VERSION 3.17)
project(protopuf)

message(NOTICE "[TIP] Expected a compiler with C++20 support (i.e. GCC 10 or above)")

set(CXX_STANDARD_REQUIRED ON)

add_library(protopuf INTERFACE)
target_include_directories(protopuf INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_features(protopuf INTERFACE cxx_std_20)

option(DOWNLOAD_GTEST "Download the git repo of GoogleTest, build and use the master branch version" OFF)
option(BUILD_TESTS "Build unit testings" ON)

if(BUILD_TESTS)
    if(DOWNLOAD_GTEST)
        # Download and unpack googletest at configure time
        configure_file(googletest/CMakeLists.txt.in googletest/CMakeLists.txt)
        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest )
        if(result)
            message(FATAL_ERROR "CMake step for googletest failed: ${result}")
        endif()
        execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest )
        if(result)
            message(FATAL_ERROR "Build step for googletest failed: ${result}")
        endif()

        # Prevent overriding the parent project's compiler/linker
        # settings on Windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # Add googletest directly to our build. This defines
        # the gtest and gtest_main targets.
        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)

    else()
        set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
        set(THREADS_PREFER_PTHREAD_FLAG TRUE)

        find_package(Threads REQUIRED)
        find_package(GTest CONFIG REQUIRED)
    endif()
    
    enable_testing()

    file(GLOB TESTS test/*.cpp)
    add_executable(protopuf_test ${TESTS})
    target_include_directories(protopuf_test PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(protopuf_test protopuf ${CMAKE_THREAD_LIBS_INIT} gtest gtest_main)

    include(GoogleTest)
    gtest_discover_tests(protopuf_test)
endif()

install(DIRECTORY include DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(TARGETS protopuf EXPORT protopufConfig)
export(TARGETS protopuf FILE "${CMAKE_CURRENT_BINARY_DIR}/protopufConfig.cmake")
install(EXPORT protopufConfig DESTINATION "${CMAKE_INSTALL_PREFIX}/share/protopuf")
