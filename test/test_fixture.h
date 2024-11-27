//   Copyright 2020-2024 PragmaTwice
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef PROTOPUF_TEST_FIXTURE_H
#define PROTOPUF_TEST_FIXTURE_H

#include <version>

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
#include <source_location>
using source_location = std::source_location;
#else
#include <experimental/source_location>
using source_location = std::experimental::source_location;
#endif

#include <gtest/gtest.h>
#include <protopuf/coder.h>

template<typename T>
struct test_fixture : testing::Test {
    using mode = T;
};

using coder_mode_types = testing::Types<pp::unsafe_mode, pp::safe_mode>;

class test_name_generator {
public:
    template<typename T>
    static std::string GetName(int) {
        if constexpr (std::is_same_v<T, pp::safe_mode>) return "safe";
        if constexpr (std::is_same_v<T, pp::unsafe_mode>) return "unsafe";
  }
};

template<pp::coder Coder, std::size_t size>
inline void run_safe_encode_test_with_insufficient_buffer_size(const auto& container, const source_location& location) {
    std::array<std::byte, size> a{};
    ASSERT_FALSE(Coder::template encode<pp::safe_mode>(container, a)) << "Buffer size " << size << ' ' <<
        '(' << location.file_name() << ':' << location.line() << ')';
}

template<pp::coder Coder, std::size_t size>
inline void run_safe_encode_tests_with_insufficient_buffer_size(const auto& container, const source_location location =
               source_location::current()) {
    [] <std::size_t... sizes> (const auto& container, std::index_sequence<sizes...>, const source_location& location) {
        (run_safe_encode_test_with_insufficient_buffer_size<Coder, sizes>(container, location), ...);
    } (container, std::make_index_sequence<size>{}, location);
}

template<pp::coder Coder, std::size_t size>
inline void run_safe_decode_test_with_insufficient_buffer_size(pp::bytes buffer, const source_location& location =
                source_location::current()) {
    ASSERT_FALSE(Coder::template decode<pp::safe_mode>(buffer.subspan(0, size))) << "Buffer size " << size << ' ' <<
        '(' << location.file_name() << ':' << location.line() << ')';
}

template<pp::coder Coder, std::size_t size>
inline void run_safe_decode_tests_with_insufficient_buffer_size(std::array<std::byte, size>& buffer, const source_location& location =
               source_location::current()) {
    [] <std::size_t... sizes> (pp::bytes buffer, std::index_sequence<sizes...>, const source_location& location) {
        (run_safe_decode_test_with_insufficient_buffer_size<Coder, sizes>(buffer, location), ...);
    } (buffer, std::make_index_sequence<size>{}, location);
}

#endif //PROTOPUF_TEST_FIXTURE_H