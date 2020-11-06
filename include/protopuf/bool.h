#ifndef PROTOPUF_BOOL_H
#define PROTOPUF_BOOL_H

#include "coder.h"
#include "byte.h"
#include "varint.h"

namespace pp {

    struct bool_coder {
        using value_type = bool;

        bool_coder() = delete;

        static constexpr bytes encode(bool i, bytes b) {
            return integer_coder<uint<1>>::encode(i, b);
        }

        static constexpr decode_result<bool> decode(bytes b) {
            return integer_coder<uint<1>>::decode(b);
        }
    };

}

#endif //PROTOPUF_BOOL_H
