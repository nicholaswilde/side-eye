#pragma once
#include <stddef.h>

inline int mbedtls_base64_decode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen) {
    *olen = 0;
    return 0;
}
