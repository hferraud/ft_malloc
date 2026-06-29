#include "free.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "chunk.h"
#include "zone.h"
#include "memory.h"

#define ERROR_INVALID_PTR_MSG "free(): invalid pointer\n"
#define ERROR_INVALID_PTR_LEN 24
#define ERROR_DOUBLE_FREE_MSG "free(): double free detected\n"
#define ERROR_DOUBLE_FREE_LEN 29

void free(void *ptr) {
    chunk_t chunk;
    zone_t zone;
    zone_t *zone_head;

    if (ptr == NULL) {
        return;
    }
    chunk = chunk_validate(ptr, &zone, &zone_head);
    if (chunk == NULL) {
        write(STDERR_FILENO, ERROR_INVALID_PTR_MSG, ERROR_INVALID_PTR_LEN);
        return;
    }
    if (chunk->free == 1) {
        write(STDERR_FILENO, ERROR_DOUBLE_FREE_MSG, ERROR_DOUBLE_FREE_LEN);
        return;
    }
    if (zone == NULL) {
        if (chunk == memory_g.large_head) {
            memory_g.large_head = chunk->next;
        }
        if (chunk->prev) {
            chunk->prev->next = chunk->next;
        }
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
        chunk->free = 1;
        if (munmap(chunk, chunk->size + CHUNK_METADATA_SIZE) == -1) {
            perror("free: munmap");
        }
        return;
    }
    chunk->free = 1;
    chunk_fusion(chunk);
    zone_unmap(zone_head);
}

