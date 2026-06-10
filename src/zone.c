#include "zone.h"

#include <unistd.h>
#include <sys/mman.h>
#include "chunk.h"
#include "utils.h"

zone_t tiny_head = NULL;
zone_t small_head = NULL;

/**
 * @brief Create a new zone
 * @param last The last zone search, new zone will be placed next
 * @param chunk_size The chunk size that will be placed in the zone
 * @return The newly created zone
 */
zone_t zone_new(zone_t last, size_t chunk_size) {
    zone_t new_zone;
    size_t zone_size;
    const size_t page_size = sysconf(_SC_PAGESIZE);

    if (chunk_size <= TINY_CHUNK_SIZE) {
        zone_size = TINY_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
    } else if (chunk_size <= SMALL_CHUNK_SIZE) {
        zone_size = SMALL_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
    } else {
        return NULL;
    }
    zone_size += page_size - zone_size % page_size;
    new_zone = mmap_wrapper(zone_size);
    if (new_zone == NULL) {
        return NULL;
    }
    if (last != NULL) {
        last->next = new_zone;
    }
    new_zone->next = NULL;
    new_zone->size = zone_size - ZONE_METADATA_SIZE;
    //Now we add free chunk of the size of the remaining space
    chunk_t new_chunk = zone_get_chunk(new_zone);
    chunk_init(new_chunk, zone_size - ZONE_METADATA_SIZE - CHUNK_METADATA_SIZE);
    new_chunk->free = 1;
    return new_zone;
}

/**
 * @brief Unmap the second free zone found, ensuring that at least one free zone
 * remain
 * @param zone_head The zone head to update if it is unmapped
 */
void zone_unmap(zone_t* zone_head) {
    zone_t it = *zone_head;
    zone_t prev = NULL;
    uint8_t zone_found = 0;

    while (it) {
        chunk_t chunk = (chunk_t)it->data;
        if (it->size == chunk->size + CHUNK_METADATA_SIZE && chunk->free) {
            //The zone is free
            if (zone_found) {
                if (prev == NULL) {
                    *zone_head = it->next;
                }
                else {
                    prev->next = it->next;
                }
                munmap(it, it->size);
                return;
            }
            zone_found = 1;
        }
        prev = it;
        it = it->next;
    }
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

zone_t  zone_validate(uintptr_t addr, zone_t head) {
    while (head) {
        if (addr >= (uintptr_t)head->data && addr < (uintptr_t)head->data + head->size) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

chunk_t  zone_get_chunk(zone_t zone) {
    chunk_t chunk = (chunk_t)zone->data;
    return chunk;
}

