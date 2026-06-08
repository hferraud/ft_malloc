#include "free.h"

#include <sys/mman.h>

#include "chunk.h"

//TODO: We need to unmap the unallocated zones
void ft_free(void *ptr) {
    chunk_t chunk;
    zone_t zone;
    zone_t zone_head;

    if (ptr == NULL) {
        return;
    }
    chunk = chunk_validate(ptr, &zone, &zone_head);
    if (chunk == NULL) {
        //TODO: better error printing

        // dprintf(STDERR_FILENO, "free(): invalid pointer\n");
        return;
    }
    if (chunk->free == 1) {
        //TODO: better error printing

        // dprintf(STDERR_FILENO, "free(): double free detected\n");
        return;
    }
    if (zone == NULL) {
        //no zone were found so the chunk is a large one, we need to use munmap
        //TODO move this in a chunk function or something maybe one that takes a fct pointer to free
        if (chunk == large_head) {
            large_head = chunk->next;
        }
        if (chunk->prev) {
            chunk->prev->next = chunk->next;
        }
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
        chunk->free = 1;
        if (munmap(chunk, chunk->size + CHUNK_METADATA_SIZE) == -1) {
            //TODO: better error printing

            // dprintf(STDERR_FILENO, "munmap(): failed\n");
        }
        return;
    }
    chunk->free = 1;
    chunk_fusion(chunk);
    //TODO: Here we need to unmap the zones if possible by
    //iterating onto them and finding at least two free zones
}

