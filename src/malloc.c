#include "malloc.h"

#include <unistd.h>

#include "chunk.h"
#include "def.h"

void *malloc(size_t size) {
    chunk_t chunk;

    size = ALIGN_MEM(size);
    chunk = chunk_get(size);
    if (chunk == NULL) {
        return NULL;
    }
    return chunk->data;
}