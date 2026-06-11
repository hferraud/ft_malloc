#include "calloc.h"

#include <stdint.h>
#include <string.h>

#include "malloc.h"

static void mem_zero(void *addr, size_t size);
static void mem_zero8(void *addr, size_t size);
static void mem_zero16(void *addr, size_t size);
static void mem_zero32(void *addr, size_t size);

void *calloc(size_t nmemb, size_t size) {
    void *addr;

    if (nmemb == 0 || size == 0) {
        return NULL;
    }
    if (size > SIZE_MAX / nmemb) {
        return NULL;
    }
    addr = malloc(nmemb * size);
    mem_zero(addr, nmemb * size);
    return addr;
}

static void mem_zero(void *addr, size_t size) {
    //knowing that memory is aligned, we can speed up the zeroing
    if (size % 4 == 0) {
        return mem_zero32(addr, size);
    }
    if (size % 2 == 0) {
        return mem_zero16(addr, size);
    }
    mem_zero8(addr, size);
}

static void mem_zero8(void *addr, size_t size) {
    uint8_t *tmp;

    tmp = (uint8_t*)addr;
    for (size_t i = 0; i < size; i++) {
        *tmp++ = 0;
    }
}

static void mem_zero16(void *addr, size_t size) {
    uint16_t *tmp;

    tmp = (uint16_t*)addr;
    for (size_t i = 0; i < size / 2; i++) {
        *tmp++ = 0;
    }
}

static void mem_zero32(void *addr, size_t size) {
    uint32_t *tmp;

    tmp = (uint32_t*)addr;
    for (size_t i = 0; i < size / 4; i++) {
        *tmp++ = 0;
    }
}
