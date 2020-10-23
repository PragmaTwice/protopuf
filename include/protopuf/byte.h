#ifndef PROTOPUF_BYTE_H
#define PROTOPUF_BYTE_H

#include <cstddef>
#include <span>

namespace pp {

    inline std::byte operator "" _b(unsigned long long int i) {
        return std::byte(i);
    }

    template <std::size_t N>
    using sized_bytes = std::span<std::byte, N>;

    using bytes = std::span<std::byte>;

    inline std::size_t begin_diff(bytes a, bytes b) {
        return a.begin() - b.begin();
    }

}

#endif //PROTOPUF_BYTE_H
