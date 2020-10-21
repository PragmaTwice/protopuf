#ifndef PROTOPUF_VARINT_H
#define PROTOPUF_VARINT_H

#include <int.h>
#include <byte.h>
#include <concepts>
#include <coder.h>

namespace pp {

    // varint encoder/decoder
    // template <pp::integral> // failed to pass GCC, strangely
    template <typename>
    class varint_coder;

    template<std::unsigned_integral T>
    class varint_coder<T> {
    public:
        using value_type = T;

        varint_coder() = delete;

        static constexpr std::size_t encode(T n, std::span<std::byte> s) {
            auto iter = s.begin();
            for (; std::byte(n) != 0_b; n >>= 7, ++iter) {
                *iter = 0b1000'0000_b | std::byte(n);
            }
            *(iter - 1) &= 0b0111'1111_b;

            return iter - s.begin();
        }

        static constexpr decode_result<T> decode(std::span<std::byte> s) {
            T n = 0;

            auto iter = s.begin();
            std::size_t i = 0;
            for(; (*iter >> 7) == 1_b; ++iter, ++i) {
                n |= static_cast<T>(*iter & 0b0111'1111_b) << 7*i;
            }
            n |= static_cast<T>(*iter) << 7*i;

            return {n, iter - s.begin()};
        }
    };

    template<std::signed_integral T>
    class varint_coder<T> {
    public:
        using value_type = T;

        varint_coder() = delete;

        static constexpr std::size_t encode(T n, std::span<std::byte> s) {
            return varint_coder<std::make_unsigned_t<T>>::encode(n, s);
        }

        static constexpr decode_result<T> decode(std::span<std::byte> s) {
            return varint_coder<std::make_unsigned_t<T>>::decode(s);
        }
    };


}

#endif //PROTOPUF_VARINT_H
