#ifndef PROTOPUF_BYTE_H
#define PROTOPUF_BYTE_H

#include <cstddef>

std::byte operator"" _b (unsigned long long int i){
    return std::byte(i);
};

#endif //PROTOPUF_BYTE_H
