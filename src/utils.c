#include "utils.h"

#include <stddef.h>
#include <sys/mman.h>

void *mmap_wrapper(size_t size) {
    void *ret;

    ret = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    return ret;
}