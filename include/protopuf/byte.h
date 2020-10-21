#ifndef PROTOPUF_BYTE_H
#define PROTOPUF_BYTE_H

#include <cstddef>

inline std::byte operator"" _b (unsigned long long int i){
    return std::byte(i);
};

#endif //PROTOPUF_BYTE_H
