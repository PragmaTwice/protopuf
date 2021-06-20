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

#ifndef PROTOPUF_ENUM_H
#define PROTOPUF_ENUM_H

#include "coder.h"
#include "varint.h"
#include <type_traits>

namespace pp {

    /// A concept to check whether type `T` is an enumeration type
    template <typename T>
    concept enum_c = std::is_enum_v<T>;

    /// A @ref coder for enumeration types
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
