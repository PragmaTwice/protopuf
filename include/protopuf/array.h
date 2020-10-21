#ifndef PROTOPUF_ARRAY_H
#define PROTOPUF_ARRAY_H

#include "coder.h"
#include <ranges>
#include <vector>
#include "varint.h"

namespace pp {
    template <coder C, std::ranges::range R = std::vector<typename C::value_type>>
    struct array_coder {
        using value_type = R;

        array_coder() = delete;

//        static constexpr std::size_t encode(const R& con, bytes b) {
//            return 0;
//        }
    };
}

#endif //PROTOPUF_ARRAY_H
