//   Copyright 2020-2021 PragmaTwice
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef PROTOPUF_MAP_H
#define PROTOPUF_MAP_H

#include "message.h"

namespace pp {

    template<coder key_coder, coder value_coder>
    struct map_element : message<field<"key", 1, key_coder>, field<"value", 2, value_coder>> {
        using base_type = message<field<"key", 1, key_coder>, field<"value", 2, value_coder>>;
        using pair_type = std::pair<const typename key_coder::value_type, typename value_coder::value_type>;

        map_element(const pair_type& v) : base_type(v.first, v.second) {}

        using base_type::base_type;

        operator pair_type() const {
            return { this->template get<1>().value(), this->template get<2>().value() };
        }
    };

    template <typename T1, typename T2>
    constexpr bool is_message <map_element<T1, T2>> = true;

    template <typename T1, typename T2>
    struct message_decode_map<map_element<T1, T2>> : message_decode_map<typename map_element<T1, T2>::base_type> {};

    template<basic_fixed_string S, uint<4> N, coder key_coder, coder value_coder,
            typename Container = std::map<typename key_coder::value_type, typename value_coder::value_type>>
    using map_field = message_field<S, N, map_element<key_coder, value_coder>, repeated, Container>;

}

#endif //PROTOPUF_MAP_H
