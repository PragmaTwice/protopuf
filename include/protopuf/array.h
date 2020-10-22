#ifndef PROTOPUF_ARRAY_H
#define PROTOPUF_ARRAY_H

#include "coder.h"
#include <ranges>
#include <vector>
#include "varint.h"

namespace pp {

    template <typename T>
    concept insertable_range = std::ranges::range<T> && requires (T t) {
        sizeof(std::inserter(t, t.end()));
    };

    template <typename T>
    concept insertable_sized_range = insertable_range<T> && std::ranges::sized_range<T>;

    template <coder C, std::ranges::sized_range R = std::vector<typename C::value_type>>
    struct array_coder {
        using value_type = R;

        array_coder() = delete;

        static constexpr bytes encode(const R& con, bytes b) {
            b = varint_coder<uint<8>>::encode(std::ranges::size(con), b);

            for(const auto& i : con) {
                b = C::encode(i, b);
            }

            return b;
        }

        static constexpr decode_result<R> decode(bytes b) {
            uint<8> len = 0;
            std::tie(len, b) = varint_coder<uint<8>>::decode(b);
            R con;

            while(len--) {
                std::tie(*std::inserter(con, con.end()), b) = C::decode(b);
            }

            return {con, b};
        }
    };

    template <integral T>
    using basic_string_coder = array_coder<integer_coder<T>, std::basic_string<T>>;

    using string_coder = basic_string_coder<std::string::value_type>;

    using bytes_coder = array_coder<integer_coder<uint<1>>>;

}

#endif //PROTOPUF_ARRAY_H
