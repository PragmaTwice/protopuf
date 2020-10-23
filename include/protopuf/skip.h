#ifndef PROTOPUF_SKIP_H
#define PROTOPUF_SKIP_H

#include "coder.h"
#include "int.h"
#include "varint.h"
#include "zigzag.h"

namespace pp {

    template <typename T>
    concept encode_skipper = coder<T> && requires(typename T::value_type v) {
        { T::encode_skip(v) } -> std::same_as<std::size_t>;
    };

    template <typename T>
    concept decode_skipper = coder<T> && requires(bytes v) {
        { T::decode_skip(v) } -> std::same_as<bytes>;
    };

    template <typename T>
    concept skipper_c = encode_skipper<T> && decode_skipper<T>;

    template <coder C>
    struct skipper;

    template <typename T>
    struct skipper<integer_coder<T>> {
        using value_type = T;

        static constexpr std::size_t encode_skip(T) {
            return sizeof(T);
        }

        static constexpr bytes decode_skip(bytes b) {
            return b.subspan<sizeof(T)>();
        }
    };

    template <std::unsigned_integral T>
    struct skipper<varint_coder<T>> {
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            std::size_t n = 0;

            do {
                v >>= 7, ++n;
            } while(v != 0);

            return n;
        }

        static constexpr bytes decode_skip(bytes b) {
            auto iter = b.begin();
            while((*iter++ >> 7) == 1_b) {}
            iter++;

            return {iter, b.end()};
        }
    };

    template <std::signed_integral T>
    struct skipper<varint_coder<T>> {
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            return skipper<varint_coder<std::make_unsigned_t<T>>>::encode_skip(v);
        }

        static constexpr bytes decode_skip(bytes b) {
            return skipper<varint_coder<std::make_unsigned_t<T>>>::decode_skip(b);
        }
    };

    template <std::size_t N>
    struct skipper<varint_coder<sint_zigzag<N>>> {
    private:
        using T = sint_zigzag<N>;

    public:
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            return skipper<varint_coder<uint<N>>>::encode_skip(v.get_underlying());
        }

        static constexpr bytes decode_skip(bytes b) {
            return skipper<varint_coder<uint<N>>>::decode_skip(b);
        }
    };



}

#endif //PROTOPUF_SKIP_H
