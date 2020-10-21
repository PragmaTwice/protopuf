#ifndef PROTOPUF_VARINT_H
#define PROTOPUF_VARINT_H

#include <int.h>
#include <zigzag.h>
#include <byte.h>
#include <concepts>

namespace pp {

    // template <pp::integral> // failed to pass GCC, strangely
    template <typename>
    class varint;

    template<std::unsigned_integral T>
    class varint<T> {
    public:
        varint() = delete;

        static constexpr void encode(T n, std::span<std::byte> s) {
            auto iter = s.begin();
            for (; std::byte(n) != 0_b; n >>= 7, ++iter) {
                *iter = 0b1000'0000_b | std::byte(n);
            }
            *(iter - 1) &= 0b0111'1111_b;
        }

        static constexpr T decode(std::span<std::byte> s) {
            T n = 0;

            auto iter = s.begin();
            std::size_t i = 0;
            for(; (*iter >> 7) == 1_b; ++iter, ++i) {
                n |= static_cast<T>(*iter & 0b0111'1111_b) << 7*i;
            }
            n |= static_cast<T>(*iter) << 7*i;

            return n;
        }
    };

    template<std::signed_integral T>
    class varint<T> {
    public:
        varint() = delete;

        static constexpr void encode(T n, std::span<std::byte> s) {
            varint<std::make_unsigned_t<T>>::encode(n, s);
        }

        static constexpr T decode(std::span<std::byte> s) {
            return varint<std::make_unsigned_t<T>>::decode(s);
        }
    };

    template<std::size_t N>
    class varint<sint_zigzag<N>> {
        using T = sint_zigzag<N>;

    public:
        varint() = delete;

        static constexpr void encode(T n, std::span<std::byte> s) {
            varint<uint<N>>::encode(n.get_underlying(), s);
        }

        static constexpr T decode(std::span<std::byte> s) {
            return T::from_uint(varint<uint<N>>::decode(s));
        }
    };

}

#endif //PROTOPUF_VARINT_H
