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

#ifndef PROTOPUF_BOOL_H
#define PROTOPUF_BOOL_H

#include "coder.h"
#include "byte.h"
#include "int.h"

namespace pp {

    /// A @ref coder for boolean type
    struct bool_coder {
        using value_type = bool;

        bool_coder() = delete;

        template <coder_mode Mode = unsafe_mode>
        static constexpr encode_result<Mode> encode(bool i, bytes b) {
            return integer_coder<uint<1>>::encode<Mode>(i, b);
        }

        template <coder_mode Mode = unsafe_mode>
        static constexpr decode_result<bool, Mode> decode(bytes b) {
            return integer_coder<uint<1>>::decode<Mode>(b);
        }
    };

}

#endif //PROTOPUF_BOOL_H
