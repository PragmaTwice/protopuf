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
#include "coder_mode.h"

namespace pp {

    /// @brief A type which `encoder`'s `encode` returns.
    /// @param Mode the encoding mode
    template<coder_mode Mode>
    using encode_result = typename Mode::template result_type<bytes>;

    /// @brief A pair type which `decoder`'s `decode` returns.
    /// - Left type of pair `T`: the type of decoded object.
    /// - Right type of pair `bytes`: the `bytes` which remains not decoded after finishing `decode`.
    template<typename T>
    using decode_value = std::pair<T, bytes>;

    /// @brief A type which `decoder`'s `decode` returns.
    /// @param T the type of decoded object
    /// @param Mode the decoding mode
    template<typename T, coder_mode Mode>
    using decode_result = typename Mode::template result_type<decode_value<T>>;

    /// @brief Describes a type with static member function `encode`, which serializes an object to `bytes` (no ownership).
    ///
    /// Encoding can be performed in different modes.
    /// Type alias `value_type` describes type of the object to be encoded.
    /// Static member function `encode`:
    /// @param v the object to be encoded (source object).
    /// @param s the bytes which the object `v` is encoded into (target bytes).
    /// @returns the @ref encode_result which depends on the encoding mode.
    /// The result contains a bytes from `begin(s) + encoding_length(v)` to `end(s)`, where `encoding_length` is the length of
    /// encoded object (bytes form), representing the left bytes which remains not used yet.
    template<typename T>
    concept encoder = requires(typename T::value_type v, bytes s) {
        typename T::value_type;
        { T::template encode<unsafe_mode>(v, s) } -> std::same_as<encode_result<unsafe_mode>>;
        { T::template encode<safe_mode>(v, s) } -> std::same_as<encode_result<safe_mode>>;
    };

    /// @brief Describes a type with static member function `decode`, which deserializes some `bytes` to an object.
    ///
    /// Decoding can be performed in different modes.
    /// Type alias `value_type` describes type of the object to be decoded.
    /// Static member function `decode`:
    /// @param s the bytes which the object is decoded from (source bytes).
    /// @returns the @ref decode_result which depends on the encoding mode.
    /// The result contains a pair including:
    /// - the decoded object `v`;
    /// - the bytes from `begin(s) + decoding_length(v)` to `end(s)`, where `decoding_length` is the length of
    /// decoded object (bytes form), representing the left bytes which remains not used yet.
    template<typename T>
    concept decoder = requires(bytes s) {
        typename T::value_type;
        { T::template decode<unsafe_mode>(s) } -> std::same_as<decode_result<typename T::value_type, unsafe_mode>>;
        { T::template decode<safe_mode>(s) } -> std::same_as<decode_result<typename T::value_type, safe_mode>>;
    };

    /// @brief Describes a type which is both @ref encoder and @ref decoder.
    template<typename T>
    concept coder = encoder<T> && decoder<T>;

}

#endif //PROTOPUF_CODER_H
