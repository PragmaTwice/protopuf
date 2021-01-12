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

#ifndef PROTOPUF_CODER_H
#define PROTOPUF_CODER_H

#include <utility>
#include "byte.h"

namespace pp {

    template<typename T>
    using decode_result = std::pair<T, bytes>;

    template<typename T>
    concept encoder = requires(typename T::value_type v, bytes s) {
        typename T::value_type;
        { T::encode(v, s) } -> std::same_as<bytes>;
    };

    template<typename T>
    concept decoder = requires(bytes s) {
        typename T::value_type;
        { T::decode(s) } -> std::same_as<decode_result<typename T::value_type>>;
    };

    template<typename T>
    concept coder = encoder<T> && decoder<T>;

}

#endif //PROTOPUF_CODER_H
