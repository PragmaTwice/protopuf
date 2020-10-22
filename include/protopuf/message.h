#ifndef PROTOPUF_MESSAGE_H
#define PROTOPUF_MESSAGE_H

#include "coder.h"
#include "int.h"
#include "varint.h"
#include "zigzag.h"
#include "array.h"
#include "float.h"

namespace pp {
    template <coder T>
    constexpr uint<1> wire_type = -1;

    template <typename T>  requires integral32<T> || integral64<T>
    constexpr uint<1> wire_type<varint_coder<T>> = 0;

    template <integral64 T>
    constexpr uint<1> wire_type<integer_coder<T>> = 1;

    template <floating_point64 T>
    constexpr uint<1> wire_type<float_coder<T>> = 1;

    template <typename T, typename C>
    constexpr uint<1> wire_type<array_coder<T, C>> = 2;

    template <integral32 T>
    constexpr uint<1> wire_type<integer_coder<T>> = 5;

    template <floating_point32 T>
    constexpr uint<1> wire_type<float_coder<T>> = 5;

    enum attribute {
        singular,
        repeated
    };

    template <uint<4> N, coder C, attribute A = singular>
    struct field {
        static constexpr uint<4> number = N;

        static constexpr uint<4> key = (N << 3) | wire_type<C>;

        using coder = C;

        static constexpr attribute attr = A;
    };
}

#endif //PROTOPUF_MESSAGE_H
