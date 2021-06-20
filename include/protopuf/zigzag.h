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

#ifndef PROTOPUF_ZIGZAG_H
#define PROTOPUF_ZIGZAG_H

#include <cstddef>
#include "int.h"
#include "varint.h"

namespace pp {

    /// @brief A ZigZag encoded signed integer.
    /// @param N the byte length of the underlying integer type, i.e. `2` for `uint<2>` as well as `std::int16_t`.
    ///
    /// Unlike two's complement, Zigzag encoding use the least-significant bit for sign,
    /// so that encoded 0 corresponds to 0, 1 to −1, 10 to 1, 11 to −2, 100 to 2, etc.
    ///
    /// Reference:
    /// - https://en.wikipedia.org/wiki/Variable-length_quantity#Zigzag_encoding
    /// - https://developers.google.com/protocol-buffers/docs/encoding#signed_integers
    template <std::size_t N>
    class sint_zigzag {
    public:

        /// The underlying type of the Zigzag encoded integer, as where the integer data stores.
        using underlying_type = uint<N>;

    private:
        underlying_type v;

        constexpr static uint<N> from_sint(sint<N> in) {
            return (in << 1) ^ (in >> (N * 8 - 1));
        }

        constexpr static sint<N> to_sint(uint<N> in) {
            return (in >> 1) ^ -(in & 1);
        }

    public:
        /// Default constructor, a new Zigzag encoded integer with value `0`
        constexpr sint_zigzag() : v(0) {}

        /// Construct the Zigzag encoded integer with value `in`
        constexpr explicit sint_zigzag(sint<N> in) : v(from_sint(in)) {}
        /// Construct the Zigzag encoded integer with value converted from the byte sequence `in`
        constexpr explicit sint_zigzag(std::span<std::byte, N> in) : v(bytes_to_int(in)) {}

        /// Copy constructor, copy from `sint_zigzag<M>` to this `sint_zigzag<N>`, where `M <= N`
        template <std::size_t M> requires (M <= N)
        constexpr sint_zigzag(const sint_zigzag<M>& i) : v(i.v) {}

        /// Convert the Zigzag encoding integer to a normal signed integer (two's complement encoding)
        constexpr sint<N> get() const {
            return to_sint(v);
        }

        /// Explicit type cast to `sint<N>`, same as @ref get
        constexpr explicit operator sint<N>() const {
            return get();
        }

        /// Construct a Zigzag encoded integer directly from the underlying data (in integer type)
        static constexpr sint_zigzag from_uint(underlying_type in) {
            sint_zigzag s;
            s.v = in;
            return s;
        }

        /// Get the underlying data (in integer type) of the Zigzag encoded integer
        constexpr underlying_type get_underlying() const {
            return v;
        }

        /// Dump the underlying data into a byte sequence with length `N` (no ownership)
        constexpr void dump_to(std::span<std::byte, N> out) const {
            int_to_bytes<N>(v, out);
        }

        /// Dump the underlying data to a byte array with length `N` (with ownership)
        constexpr std::array<std::byte, N> dump() const {
            return int_to_bytes<N>(v);
        }
        
        /// Assignment operator, copy from `sint_zigzag<M>` to this `sint_zigzag<N>`, where `M <= N`
        template <std::size_t M> requires (M <= N)
        constexpr sint_zigzag& operator=(const sint_zigzag<M>& i) {
            v = i.v;
            return *this;
        }

        constexpr bool operator==(const sint_zigzag& x) const {
            return v == x.v;
        }

        constexpr bool operator!=(const sint_zigzag& x) const {
            return !(*this == x);
        }
    };

    template <std::size_t N>
    struct is_integral<sint_zigzag<N>> : std::true_type {};

    template <std::size_t N>
    class integer_coder<sint_zigzag<N>> {
        using T = sint_zigzag<N>;

    public:
        using value_type = T;

        integer_coder() = delete;

        static constexpr bytes encode(T i, bytes bytes) {
            return integer_coder<uint<N>>::encode(i.get_underlying(), bytes);
        }

        static constexpr decode_result<T> decode(bytes bytes) {
            auto p = integer_coder<uint<N>> ::decode(bytes);
            return {T::from_uint(p.first), p.second};
        }
    };


    template<std::size_t N>
    class varint_coder<sint_zigzag<N>> {
        using T = sint_zigzag<N>;

    public:
        using value_type = T;

        varint_coder() = delete;

        static constexpr bytes encode(T n, bytes s) {
            return varint_coder<uint<N>>::encode(n.get_underlying(), s);
        }

        static constexpr decode_result<T> decode(bytes s) {
            auto p = varint_coder<uint<N>>::decode(s);
            return {T::from_uint(p.first), p.second};
        }
    };
}

#endif //PROTOPUF_ZIGZAG_H
