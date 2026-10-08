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

#ifndef PROTOPUF_CODER_MODE_H
#define PROTOPUF_CODER_MODE_H

#include <optional>
#include "byte.h"

namespace pp {

    /// @brief Describes a type with static member function `make_result`, 
    /// which make an encoding/decoding result which depends on the coding mode.
    /// @param v some object that could potentially be contained as a result of encoding.
    /// @returns the encoding/decoding result which depends on the coding mode.
   	template<typename T>
    concept coder_result_maker = requires(std::pair<int, int> v) {
        typename T::template result_type<decltype(v)>;

        { T::template make_result<typename T::template result_type<decltype(v)>>(0, 0) } -> 
            std::same_as<typename T::template result_type<decltype(v)>>;
    };

    /// @brief Describes a type with static member function `get_value_from_result`, 
    /// which extract value from encoding/decoding result depending on encoding/decoding mode.
    /// @param v some object that could potentially be extracted from the encoding result.
    /// @param r the encoding/decoding result.
    /// @returns true if the value is extracted, otherwise false.
   	template<typename T>
    concept coder_result_value_getter = requires(std::pair<int, int> v, typename T::template result_type<decltype(v)> r) {
        typename T::template result_type<decltype(v)>;

        { T::template get_value_from_result<typename T::template result_type<decltype(v)>>(std::move(r), v) } -> std::same_as<bool>;
    };

    /// @brief Describes a type with static member function `check_iterator`, 
    /// which checks if an iterator is valid depending on the encoding/decoding mode.
    /// @param itr the iterator that checks for validity.
    /// @param end the iterator to the element following the last element.
    /// @returns true if the iterator is valid, otherwise false.
   	template<typename T>
    concept iterator_checker = requires(bytes::iterator itr, bytes::iterator end) {
        { T::check_iterator(itr, end) } -> std::same_as<bool>;
    };

    /// @brief Describes a type with static member function `check_bytes_span`, 
    /// which checks if the span offset is valid depending on the encoding/decoding mode.
    /// @param b the byte span.
    /// @param offset offset into the span of byte that checks for validity.
    /// @returns true if the offset is valid, otherwise false.
   	template<typename T>
    concept bytes_span_checker = requires(bytes b, std::size_t offset) {
        { T::check_bytes_span(b, offset) } -> std::same_as<bool>;
    };

    /// @brief Describes a type for the coder operating mode.
    template<typename T>
    concept coder_mode = coder_result_maker<T> && coder_result_value_getter<T> && iterator_checker<T> && bytes_span_checker<T>;

    /// @brief Unsafe @ref coder_mode to perform coding without buffer overflow checking
    struct unsafe_mode {
        template<typename T>
        using result_type = std::remove_reference_t<T>;

        static constexpr bool need_checks = false;

        template<typename R, typename... Args>
        static constexpr R make_result(Args&&... args) {
            return R{std::forward<Args>(args)...};
        }

        template<typename T>
        static constexpr bool get_value_from_result(T&& result, auto& value) {
            value = std::forward<T>(result);
            return true;
        }

        static constexpr bool check_iterator(bytes::iterator, bytes::iterator) {
            return true;
        }

        static constexpr bool check_bytes_span(bytes, std::size_t) {
           return true;
        }
    };

    /// @brief Safe @ref coder_mode to perform coding with buffer overflow checking (the coding result is wrapped into std::optional)
    struct safe_mode {
        template<typename T>
        using result_type = std::optional<std::remove_reference_t<T>>;

        static constexpr bool need_checks = true;

        template<typename R, typename... Args>
        static constexpr R make_result(Args&&... args) {
            return R{std::in_place, std::forward<Args>(args)...};
        }

        template<typename T>
        static constexpr bool get_value_from_result(T&& result, auto& value) {
            if (result.has_value()) {
                value = std::forward<decltype(*result)>(*result);
            } else {
                return false;
            }
            return true;
        }

        static constexpr bool check_iterator(bytes::iterator iter, bytes::iterator end) {
            return iter != end;
        }

        static constexpr bool check_bytes_span(bytes b, std::size_t offset) {
           return b.size() >= offset;
        }
    };

}

#endif //PROTOPUF_CODER_MODE_H
