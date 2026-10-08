//   Copyright 2020-2021 PragmaTwice
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

#ifndef PROTOPUF_INT_H
#define PROTOPUF_INT_H

#include <cstdint>
#include <cstddef>
#include <version>

#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
#include <bit>
#endif

#include "coder.h"
#include "byte.h"

namespace pp {
    template <std::size_t N>
    struct sint_impl;

    template <>
    struct sint_impl<1> {
        using type = std::int8_t;
    };

    template <>
    struct sint_impl<2> {
        using type = std::int16_t;
    };

    template <>
    struct sint_impl<4> {
        using type = std::int32_t;
    };

    template <>
    struct sint_impl<8> {
        using type = std::int64_t;
    };

    /// @brief Type alias for signed integer.
    /// @param N byte length of the integer, i.e. `2` for `std::int16_t`.
    template <std::size_t N>
    using sint = typename sint_impl<N>::type;

    template <std::size_t N>
    struct uint_impl;

    template <>
    struct uint_impl<1> {
        using type = std::uint8_t;
    };

    template <>
    struct uint_impl<2> {
        using type = std::uint16_t;
    };

    template <>
    struct uint_impl<4> {
        using type = std::uint32_t;
    };

    template <>
    struct uint_impl<8> {
        using type = std::uint64_t;
    };

    /// @brief Type alias for unsigned integer.
    /// @param N byte length of the integer, i.e. `2` for `std::uint16_t`.
    template <std::size_t N>
    using uint = typename uint_impl<N>::type;

    /// @brief Checks whether `T` is an integral type.
    ///
    /// We need it because specializing `std::is_integral` is not allowed, 
    /// but something like @ref sint_zigzag is also an integral type in protopuf.
    template <typename T>
    struct is_integral : std::is_integral<T> {};

    /// Checks whether `T` is an integral type.
    template <typename T>
    constexpr bool is_integral_v = is_integral<T>::value;

    /// @brief A concept satisfied if and only if `T` is an integral type.
    template <typename T>
    concept integral = is_integral_v<T>;

    /// @brief A concept satisfied if and only if `T` is an integral type, 
    /// and the size of `T` equals to `N`.
    template <typename T, std::size_t N>
    concept sized_integral = integral<T> && sizeof(T) == N;

    /// @brief A concept satisfied if and only if `T` is an integral type, 
    /// and the byte size of `T` equals to `4`.
    template <typename T>
    concept integral32 = sized_integral<T, 4>;

    /// @brief A concept satisfied if and only if `T` is an integral type, 
    /// and the byte size of `T` equals to `8`.
    template <typename T>
    concept integral64 = sized_integral<T, 8>;

    /// Construct a `std::array<T, N>` from values of a `std::span<T, N>`
    template <typename T, std::size_t N>
    constexpr auto make_array(std::span<T, N> s) {
        return [&s] <std::size_t ...I> (std::index_sequence<I...>) {
            return std::array<T, N> { s[I]... };
        }(std::make_index_sequence<N>{});
    }

    /// Copy values of a `std::array<T, N>` to a `std::span<T, N>`
    template <typename T, std::size_t N>
    constexpr void copy_to_span(const std::array<T, N>& a, std::span<T, N> s) {
        [&a, &s] <std::size_t ...I> (std::index_sequence<I...>) {
            ((s[I] = a[I]), ...);
        }(std::make_index_sequence<N>{});
    }

    /// @brief Convert some bytes (with length `N`) to an unsigned integer `uint<N>`.
    ///
    /// @param bytes the input bytes (with length `N`) to be coverted
    /// @returns the coverted unsigned integer `uint<N>`
    template <std::size_t N>
    constexpr uint<N> bytes_to_int(sized_bytes<N> bytes) {
    #if defined(INT_CONVERSION_RECURSIVE_IMPL) || !(__cpp_lib_bit_cast >= 201806L)
        return bytes_to_int(bytes.template subspan<0, N/2>()) | bytes_to_int(bytes.template subspan<N/2>()) << N*4;
    #elif defined(INT_CONVERSION_UB_IMPL)
        return *reinterpret_cast<uint<N>*>(bytes.data());
    #else
        auto arr = make_array(bytes);
        return std::bit_cast<uint<N>>(arr);
    #endif
    }
    template <>
    constexpr uint<1> bytes_to_int(sized_bytes<1> bytes) {
        return static_cast<uint<1>>(bytes.front());
    }

    /// @brief Convert an unsigned integer (with byte length `N`) into a byte sequence with length `N` (no ownership).
    ///
    /// @param i the unsigned integer to be converted
    /// @param bytes the byte sequence which the integer is converted into (with length `N`)
    template <std::size_t N>
    constexpr void int_to_bytes(uint<N> i, sized_bytes<N> bytes) {
    #if defined(INT_CONVERSION_RECURSIVE_IMPL) || !(__cpp_lib_bit_cast >= 201806L)
        int_to_bytes<N/2>(i, bytes.template subspan<0, N/2>());
        int_to_bytes<N/2>(i >> N*4, bytes.template subspan<N/2>());
    #elif defined(INT_CONVERSION_UB_IMPL)
        *reinterpret_cast<uint<N>*>(bytes.data()) = i;
    #else
        auto arr = std::bit_cast<std::array<std::byte, N>>(i);
        copy_to_span(arr, bytes);
    #endif
    }
    template <>
    constexpr void int_to_bytes(uint<1> i, sized_bytes<1> bytes) {
        bytes.front() = static_cast<std::byte>(i);
    }

    /// @brief Convert an unsigned integer (with byte length `N`) into an byte array with length `N` (with ownership).
    /// @param i the unsigned integer to be converted
    /// @returns a byte array which contains the coverted integer (with length `N` and ownership)
    template <std::size_t N>
    constexpr auto int_to_bytes(uint<N> i) {
    #if __cpp_lib_bit_cast >= 201806L && !(defined(INT_CONVERSION_RECURSIVE_IMPL) || defined(INT_CONVERSION_UB_IMPL))
        return std::bit_cast<std::array<std::byte, N>>(i);
    #else
        std::array<std::byte, N> a;
        int_to_bytes(i, std::span(a));
        return a;
    #endif
    }

    /// A @ref coder for fixed-length signed/unsigned integer
    template <typename>
    class integer_coder;

    template <std::unsigned_integral T>
    class integer_coder<T> {
    public:
        using value_type = T;

        integer_coder() = delete;

        static constexpr std::size_t N = sizeof(T);

        template <coder_mode Mode = safe_mode>
        static constexpr encode_result<Mode> encode(T i, bytes b) {
            if constexpr (Mode::need_checks) {
                if (!Mode::check_bytes_span(b, N)) {
                    return {};
                }
            }
            
            int_to_bytes<N>(i, b.subspan<0, N>());
            return encode_result<Mode>{b.subspan<N>()};
        }

        template <coder_mode Mode = safe_mode>
        static constexpr decode_result<T, Mode> decode(bytes b) {
            if constexpr (Mode::need_checks) {
                if (!Mode::check_bytes_span(b, N)) {
                    return {};
                }
            }

            return Mode::template make_result<decode_result<T, Mode>>(bytes_to_int<N>(b.subspan<0, N>()), b.subspan<N>());
        }
    };

    template <std::signed_integral T>
    class integer_coder<T> {
    public:
        using value_type = T;

        integer_coder() = delete;

        template <coder_mode Mode = safe_mode>
        static constexpr encode_result<Mode> encode(T i, bytes b) {
            return integer_coder<std::make_unsigned_t<T>>::template encode<Mode>(static_cast<std::make_unsigned_t<T>>(i), b);
        }

        template <coder_mode Mode = safe_mode>
        static constexpr decode_result<T, Mode> decode(bytes b) {
            return integer_coder<std::make_unsigned_t<T>>::template decode<Mode>(b);
        }
    };

}

#endif //PROTOPUF_INT_H
