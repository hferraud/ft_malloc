#include "zone.h"

#include <unistd.h>
#include <sys/mman.h>
#include "chunk.h"

zone_t tiny_head = NULL;
zone_t small_head = NULL;

zone_t zone_new(zone_t last, size_t size) {
    zone_t new_zone;
    size_t zone_size;
    const size_t page_size = sysconf(_SC_PAGESIZE);

    if (size <= TINY_CHUNK_SIZE) {
        zone_size = TINY_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
    } else if (size <= SMALL_CHUNK_SIZE) {
        zone_size = SMALL_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
    } else {
        return NULL;
    }
    zone_size += page_size - zone_size % page_size;
    new_zone = mmap(
        NULL,
        zone_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
    if (new_zone == MAP_FAILED) {
        return NULL;
    }
    if (last != NULL) {
        last->next = new_zone;
    }
    new_zone->next = NULL;
    new_zone->size = zone_size;
    //Now we add free chunk of the size of the remaining space
    chunk_t new_chunk = zone_get_chunk(new_zone);
    chunk_init(new_chunk, zone_size - ZONE_METADATA_SIZE - CHUNK_METADATA_SIZE);
    new_chunk->free = 1;
    return new_zone;
}

zone_t  zone_validate(uintptr_t addr, zone_t head) {
    while (head) {
        if (addr >= (uintptr_t)head->data && addr < (uintptr_t)head->data + head->size) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

/**
 * @brief Search a zone list to find a chunk wide enough to contain size
 * @param z_head head of zones to search
 * @param z_last last zone checked
 * @param size size searched
 * @return the first chunk found, NULL if no chunk fit we found
 */
chunk_t zone_search(zone_t z_head, zone_t *z_last, size_t size) {
    while (z_head) {
        chunk_t chunk = chunk_search(zone_get_chunk(z_head), size);
        if (chunk != NULL) {
            return chunk;
        }
        *z_last = z_head;
        z_head = z_head->next;
    }
    return NULL;
}

chunk_t  zone_get_chunk(zone_t zone) {
    chunk_t chunk = (chunk_t)zone->data;
    return chunk;
}

