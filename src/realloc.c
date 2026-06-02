#include "realloc.h"

#include "malloc.h"
#include "free.h"
#include "chunk.h"
#include "zone.h"
#include "def.h"

void *ft_realloc(void *ptr, size_t size) {
    chunk_t chunk;
    chunk_t new_chunk;
    zone_t  zone;

    size = ALIGN_MEM(size);
    if (ptr == NULL) {
        return ft_malloc(size);
    }
    chunk = chunk_validate(ptr, &zone, NULL);
    if (chunk == NULL) {
        //TODO: better error printing

        // dprintf(STDERR_FILENO, "ft_realloc(): invalid pointer\n");
        return NULL;
    }

    if (chunk->size >= size) {
        //Here the chunk is large enough to contain the requested size
        //so we simply try to split it
        chunk_split(chunk, size);
        return ptr;
    }
    if (zone && chunk->next && chunk->next->free
        && (chunk->size + CHUNK_METADATA_SIZE + chunk->next->size)) {
        //There is enough space in the next chunk
        chunk_fusion_next(chunk);
        chunk_split(chunk, size);
    } else {
        //No space, we need to allocate a new block
        new_chunk = chunk_get(size);
        chunk_copy(chunk, new_chunk);
        ft_free(ptr);
        return new_chunk->data;
    }
    return ptr;
}
