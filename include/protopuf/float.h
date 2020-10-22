#ifndef PROTOPUF_FLOAT_H
#define PROTOPUF_FLOAT_H

#include <concepts>
#include "coder.h"
#include "int.h"

namespace pp {

    template<std::floating_point T>
    struct float_coder {
        using value_type = T;

    private:
        using underlying_type = uint<sizeof(T)>;

        static constexpr underlying_type underlying_cast(value_type t) {
            return *(underlying_type*)&t;
        }

        static constexpr value_type value_cast(underlying_type t) {
            return *(value_type*)&t;
        }

    public:
        float_coder() = delete;

        static constexpr bytes encode(T v, bytes b) {
            return integer_coder<underlying_type>::encode(underlying_cast(v), b);
        }

        static constexpr decode_result<T> decode(bytes b) {
            auto p = integer_coder<underlying_type>::decode(b);

            return {value_cast(p.first), p.second};
        }
    };

    template <std::size_t N>
    struct floating_impl;

    template <>
    struct floating_impl<4> {
        using type = float;
    };

    template <>
    struct floating_impl<8> {
        using type = double;
    };

    template <std::size_t N>
    using floating = typename floating_impl<N>::type;

    template <typename T, std::size_t N>
    concept sized_floating_point = std::floating_point<T> && sizeof(T) == N;

    template <typename T>
    concept floating_point32 = sized_floating_point<T, 4>;

    template <typename T>
    concept floating_point64 = sized_floating_point<T, 8>;
}

#endif //PROTOPUF_FLOAT_H
