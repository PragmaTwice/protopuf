#ifndef PROTOPUF_CODER_H
#define PROTOPUF_CODER_H

#include <utility>
#include <span>

namespace pp {

    template<typename T>
    using decode_result = std::pair<T, std::size_t>;

    template<typename T>
    concept encoder = requires(T, typename T::value_type v, std::span<std::byte> s) {
        typename T::value_type;
        { T::encode(v, s) } -> std::same_as<std::size_t>;
    };

    template<typename T>
    concept decoder = requires(T, std::span<std::byte> s) {
        typename T::value_type;
        { T::decode(s) } -> std::same_as<decode_result<typename T::value_type>>;
    };

    template<typename T>
    concept coder = encoder<T> && decoder<T>;

}

#endif //PROTOPUF_CODER_H
