#ifndef PROTOPUF_ARRAY_H
#define PROTOPUF_ARRAY_H

#include "coder.h"
#include <ranges>
#include <vector>
#include "varint.h"

namespace pp {

    template <coder C, std::ranges::sized_range R = std::vector<typename C::value_type>>
    struct array_coder {
        using value_type = R;

        array_coder() = delete;

        static constexpr std::size_t encode(const R& con, bytes b) {
            auto n = varint_coder<uint<8>>::encode(std::ranges::size(con), b);
            b = b.subspan(n);

            for(const auto& i : con) {
                auto m = C::encode(i, b);
                b = b.subspan(m);
                n += m;
            }

            return n;
        }

        static constexpr decode_result<R> decode(bytes b) {
            auto [len, n] = varint_coder<uint<8>>::decode(b);
            b = b.subspan(n);
            R con;

            while(len--) {
                auto [v, m] = C::decode(b);
                *std::inserter(con, con.end()) = v;
                b = b.subspan(m);
                n += m;
            }

            return {con, n};
        }
    };

    template <integral T>
    struct basic_string_coder : array_coder<integer_coder<T>, std::basic_string<T>> {};

    struct string_coder : basic_string_coder<std::string::value_type> {};

}

#endif //PROTOPUF_ARRAY_H
