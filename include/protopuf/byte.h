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

#ifndef PROTOPUF_BYTE_H
#define PROTOPUF_BYTE_H

#include <cstddef>
#include <span>

namespace pp {

    /// Byte literals, i.e. `0x12_b` as `std::byte{0x12}`.
    inline constexpr std::byte operator "" _b(unsigned long long int i) {
        return std::byte(i);
    }

    /// A sized byte (contiguous) sequence reference (no ownership).
    template <std::size_t N>
    using sized_bytes = std::span<std::byte, N>;

    /// A byte (contiguous) sequence reference (no ownership).
    using bytes = std::span<std::byte>;

}

#endif //PROTOPUF_BYTE_H
