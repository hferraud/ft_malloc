#include "chunk.h"

#include "zone.h"
#include "def.h"
#include "utils.h"
#include "../cmake-build-test/_deps/unity-src/src/unity.h"

#define MAGIC_SERIALIZE(x) (x << 8)
#define MAGIC_DESERIALIZE(x) (x >> 8)

static void     chunk_copy8(chunk_t src, chunk_t dst);
static void     chunk_copy16(chunk_t src, chunk_t dst);
static void     chunk_copy32(chunk_t src, chunk_t dst);

chunk_t large_head = NULL;

chunk_t chunk_get(size_t size) {
    zone_t *zone_head;
    zone_t zone;
    zone_t last_zone;
    chunk_t chunk;

    zone_head = NULL;
    if (size <= TINY_CHUNK_SIZE) {
        zone_head = &tiny_head;
    } else if (size <= SMALL_CHUNK_SIZE) {
        zone_head = &small_head;
    }

    if (zone_head != NULL) {
        last_zone = NULL;
        chunk = zone_search(*zone_head, &last_zone, size);
        if (chunk == NULL) {
            //If no chunk were found this mean we need to allocate more space
            zone = zone_new(last_zone, size);
            if (zone == NULL) {
                return NULL;
            }
            if (*zone_head == NULL) {
                *zone_head = zone;
            }
            chunk = zone_get_chunk(zone);
        }
        chunk_split(chunk, size);
        chunk->free = 0;
    } else {
        chunk = chunk_new(size);
        if (chunk == NULL) {
            return NULL;
        }
        chunk_init(chunk, size);
        chunk_push_back(&large_head, chunk);
    }
    return chunk;
}

void chunk_init(chunk_t chunk, size_t size) {
    chunk->size = size;
    chunk->next = NULL;
    chunk->prev = NULL;
    chunk->magic = 0;
    chunk->magic |= MAGIC_SERIALIZE((uintptr_t)chunk->data);
    chunk->free = 0;
}

chunk_t chunk_new(size_t size) {
    chunk_t new_chunk;

    new_chunk = mmap_wrapper(size + CHUNK_METADATA_SIZE);
    return new_chunk;
}

/**
 * @brief Push a chunk to the back of the chunk linked list
 * @param head the head of the linked list
 * @param chunk the chunk to push back
 */
void chunk_push_back(chunk_t *head, chunk_t chunk) {
    chunk_t it = *head;
    if (*head == NULL) {
        *head = chunk;
        return;
    }
    while (it->next) {
        it = it->next;
    }
    it->next = chunk;
    chunk->prev = it;
}

/**
 * @brief Search a chunk list to find a chunk wide enough to contain size
 * @param c_head head of chunks to search
 * @param size size searched
 * @return the first chunk found, NULL if no chunk fit we found
 */
chunk_t chunk_search(chunk_t c_head, size_t size) {
    while (c_head && !(c_head->free && c_head->size >= size)) {
        c_head = c_head->next;
    }
    return c_head;
}

chunk_t  chunk_validate(void *addr, zone_t *zone, zone_t **zone_head) {
    chunk_t chunk = (chunk_t)(addr - CHUNK_METADATA_SIZE);
    uintptr_t magic;
    uintptr_t data;
    //we check if the address is in a tiny zone
    *zone_head = &tiny_head;
    *zone = zone_validate((uintptr_t)addr, tiny_head);
    if (*zone == NULL) {
        //if not we check the small zone
        *zone_head = &small_head;
        *zone = zone_validate((uintptr_t)addr, small_head);
    }

    // we deserialize chunk->magic to remove chunk->free
    magic = MAGIC_DESERIALIZE(chunk->magic);
    // we serialize and deserialize chunk->data to reproduce the same process magic goes though
    data = MAGIC_DESERIALIZE(MAGIC_SERIALIZE((uintptr_t)chunk->data));

    //now if they match it's very likely that the address given is a correct chunk
    return magic == data ? chunk : NULL;
}

/**
 * @brief Split \a chunk into two chunk. Afterward, \a chunk is of size \a size
 * and chunk->next is of the remaining size. If \a chunk cannot contain \a size
 * and a new chunk of minimal alignment size, return NULL.
 * @param chunk The chunk to split
 * @param size The new size of \a chunk
 * @return The newly created chunk, which is also chunk->next or NULL if space
 * was insufficient
 */
chunk_t chunk_split(chunk_t chunk, size_t size) {
    chunk_t new_chunk;

    if (chunk->size < size + CHUNK_METADATA_SIZE + ALIGN_SIZE) {
        return NULL;
    }
    new_chunk = (chunk_t)(chunk->data + size);
    chunk_init(new_chunk, chunk->size - size - CHUNK_METADATA_SIZE);
    new_chunk->next = chunk->next;
    new_chunk->prev = chunk;
    new_chunk->free = 1;
    if (new_chunk->next) {
        new_chunk->next->prev = new_chunk;
    }
    chunk->size = size;
    chunk->next = new_chunk;
    return new_chunk;
}

void chunk_fusion(chunk_t chunk) {
    chunk_fusion_next(chunk);
    chunk_fusion_prev(chunk);
}

void chunk_fusion_next(chunk_t chunk) {
    if (chunk->next && chunk->next->free) {
        chunk->size += CHUNK_METADATA_SIZE + chunk->next->size;
        chunk->next = chunk->next->next;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
    }
}

void chunk_fusion_prev(chunk_t chunk) {
    if (chunk->prev && chunk->prev->free) {
        chunk->prev->size += CHUNK_METADATA_SIZE + chunk->size;
        chunk->prev->next = chunk->next;
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
    }
}

void chunk_copy(chunk_t src, chunk_t dst) {
    size_t align_size = ALIGN_SIZE;

    //knowing that the blocks are aligned, we can speed up the copy
    switch (align_size) {
        case 16:
            chunk_copy16(src, dst);
            break;
        case 32:
            chunk_copy32(src, dst);
            break;
        default:
            chunk_copy8(src, dst);
    }
}

static void chunk_copy8(chunk_t src, chunk_t dst) {
    uint8_t *src_data = (uint8_t*)src->data;
    uint8_t *dst_data = (uint8_t*)dst->data;

    for (size_t i = 0; i < src->size; i++) {
        dst_data[i] = src_data[i];
    }
}

static void chunk_copy16(chunk_t src, chunk_t dst) {
    uint16_t *src_data = (uint16_t*)src->data;
    uint16_t *dst_data = (uint16_t*)dst->data;

    for (size_t i = 0; i * 2 < src->size; i++) {
        dst_data[i] = src_data[i];
    }
}

static void chunk_copy32(chunk_t src, chunk_t dst) {
    uint32_t *src_data = (uint32_t*)src->data;
    uint32_t *dst_data = (uint32_t*)dst->data;

    for (size_t i = 0; i * 4 < src->size; i++) {
        dst_data[i] = src_data[i];
    }
}

