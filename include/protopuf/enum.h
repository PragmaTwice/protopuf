#ifndef PROTOPUF_ENUM_H
#define PROTOPUF_ENUM_H

#include "coder.h"
#include "varint.h"
#include <type_traits>

namespace pp {

    template <typename T>
    concept enum_c = std::is_enum_v<T>;

    template<enum_c T>
    struct enum_coder {
        using value_type = T;

        enum_coder() = delete;

        static constexpr bytes encode(T i, bytes b) {
            return varint_coder<std::underlying_type_t<T>>::encode(static_cast<std::underlying_type_t<T>>(i), b);
        }

        static constexpr decode_result<T> decode(bytes b) {
            auto [res, bytes] = varint_coder<std::underlying_type_t<T>>::decode(b);
            return {static_cast<T>(res), bytes};
        }
    };

}

#endif //PROTOPUF_ENUM_H
