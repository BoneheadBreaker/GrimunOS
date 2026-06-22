#include <stdint.h>

void* memcpy(void* dest, const void* src, unsigned int n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* s, int c, unsigned int n) {
    char* p = (char*)s;
    while (n--) *p++ = (char)c;
    return s;
}

int memcmp(const void* a, const void* b, unsigned int n) {
    const unsigned char* x = a;
    const unsigned char* y = b;
    while (n--) {
        if (*x != *y) return *x - *y;
        x++; y++;
    }
    return 0;
}

int strlen(const char* s) {
    int i = 0;
    while (s[i]) i++;
    return i;
}