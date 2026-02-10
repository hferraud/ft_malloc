#include "malloc.h"

#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>

//TODO: remove
#include <stdalign.h>
#include <stdio.h>

#define CHUNK_PER_ZONE      128
#define TINY_CHUNK_SIZE     128
#define SMALL_CHUNK_SIZE    4096
#define CHUNK_METADATA_SIZE 32
#define ZONE_METADATA_SIZE  32

#define ALIGN_SIZE      alignof(max_align_t)
#define ALIGN_MEM(x)    ((x + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))
#define MAGIC_SERIALIZE(x) (x << 8)
#define MAGIC_DESERIALIZE(x) (x >> 8)

typedef struct chunk_s *chunk_t;
typedef struct zone_s *zone_t;

struct chunk_s {
    size_t          size;
    struct chunk_s  *next;
    struct chunk_s  *prev;
    union {
        uint8_t     free;
        uintptr_t   magic; // Used for free ptr validation
    };
    uint8_t         data[1];
};

struct zone_s {
    size_t              size;
    size_t              free_space;
    chunk_t             chunk;
    struct zone_s       *next;
    uint8_t             data[1];
    //TODO: chunk == data, maybe we don't need the field
};

static chunk_t  chunk_get(size_t size);
static zone_t   zone_search(zone_t head, zone_t *last, size_t size);
static chunk_t  chunk_search(chunk_t head, chunk_t *last, size_t size);
static zone_t   zone_create(zone_t last, size_t size);
static chunk_t  chunk_create(zone_t zone, chunk_t last, size_t size);
static void     chunk_split(chunk_t chunk, size_t size);
static void     chunk_fusion_prev(chunk_t chunk);
static void     chunk_fusion_next(chunk_t chunk);
static void     chunk_fusion(chunk_t chunk);
static void     chunk_copy(chunk_t src, chunk_t dst);
static void     chunk_copy8(chunk_t src, chunk_t dst);
static void     chunk_copy16(chunk_t src, chunk_t dst);
static void     chunk_copy32(chunk_t src, chunk_t dst);
static void     zone_show_mem(zone_t zone);
static void     chunk_show_mem(chunk_t chunk);
static chunk_t  chunk_validate(void *addr, zone_t *zone);
static zone_t   zone_validate(uintptr_t addr, zone_t head);

static zone_t tiny_head = NULL;
static zone_t small_head = NULL;
static chunk_t large_head = NULL;

void *ft_malloc(size_t size) {
    chunk_t chunk;

    size = ALIGN_MEM(size);
    chunk = chunk_get(size);
    if (chunk == NULL) {
        return NULL;
    }
    printf("Allocated %zu bytes at address %p\n", size, chunk->data);
    return chunk->data;
}

void ft_free(void *ptr) {
    chunk_t chunk;
    zone_t zone;

    if (ptr == NULL) {
        return;
    }
    chunk = chunk_validate(ptr, &zone);
    if (chunk == NULL) {
        dprintf(STDERR_FILENO, "free(): invalid pointer\n");
        return;
    }
    if (chunk->free == 1) {
        dprintf(STDERR_FILENO, "free(): double free detected\n");
        return;
    }
    if (zone == NULL) {
        //no zone were found so the chunk is a large one, we need to use munmap
        if (chunk == large_head) {
            large_head = chunk->next;
        }
        if (munmap(chunk, chunk->size + CHUNK_METADATA_SIZE) == -1) {
            dprintf(STDERR_FILENO, "munmap(): failed\n");
            return;
        }
    }
    if (zone) {
        zone->free_space += chunk->size + CHUNK_METADATA_SIZE;
        chunk->free = 1;
        chunk_fusion(chunk);
    }
    printf("Freed %p\n", ptr);
}

void *ft_realloc(void *ptr, size_t size) {
    chunk_t chunk;
    chunk_t new_chunk;
    zone_t  zone;

    size = ALIGN_MEM(size);
    if (ptr == NULL) {
        return ft_malloc(size);
    }
    chunk = chunk_validate(ptr, &zone);
    if (chunk == NULL) {
        dprintf(STDERR_FILENO, "ft_realloc(): invalid pointer\n");
        return NULL;
    }

    if (chunk->size >= size) {
        if (chunk->size >= size + CHUNK_METADATA_SIZE + ALIGN_SIZE) {
            printf("Split block\n");
            chunk_split(chunk, size);
        }
        return ptr;
    }
    if (zone && chunk->next && chunk->next->free
        && (chunk->size + CHUNK_METADATA_SIZE + chunk->next->size)) {
        //There is enough space in the next chunk
        printf("Fusion block\n");
        chunk_fusion_next(chunk);
        if (size < chunk->size + CHUNK_METADATA_SIZE + ALIGN_SIZE) {
            chunk_split(chunk, size);
        }
    } else {
        // We need to do realloc a new block
        printf("New block\n");
        new_chunk = chunk_get(size);
        chunk_copy(chunk, new_chunk);
        ft_free(ptr);
        return new_chunk->data;
    }
    return ptr;
}

static chunk_t chunk_get(size_t size) {
    zone_t *zone_head;
    zone_t zone;
    zone_t last_zone;
    chunk_t *chunk_head;
    chunk_t chunk;
    chunk_t last_chunk;

    if (size <= TINY_CHUNK_SIZE) {
        zone_head = &tiny_head;
    } else if (size <= SMALL_CHUNK_SIZE) {
        zone_head = &small_head;
    } else {
        chunk_head = &large_head;
        zone = NULL;
        goto skip_zone_search;
    }

    last_zone = NULL;
    // we search for a zone with sufficient free space
    zone = zone_search(*zone_head, &last_zone, size);
    if (zone == NULL) {
        // we didn't find any fitting zone, so we create one
        zone = zone_create(last_zone, size);
        if (zone == NULL) {
            //mmap failed so we return NULL
            return NULL;
        }
        if (*zone_head == NULL) {
            *zone_head = zone;
        }
    }
    chunk_head = &zone->chunk;

    skip_zone_search:
    last_chunk = NULL;
    // we search for a free chunk of sufficient size
    chunk = chunk_search(*chunk_head, &last_chunk, size);
    if (chunk == NULL) {
        //we didn't find any fitting chunk, so we create one
        chunk = chunk_create(zone, last_chunk, size);
        if (*chunk_head == NULL) {
            *chunk_head = chunk;
        }
    } else {
        //we split the chunk if it is big enough
        if (chunk->size >= size + CHUNK_METADATA_SIZE + ALIGN_SIZE) {
            chunk_split(chunk, size);
        }
        chunk->free = 0;
    }
    return chunk;
}

static zone_t zone_create(zone_t last, size_t size) {
    zone_t new_zone;
    size_t zone_size;
    int page_size = getpagesize();

    printf("Creating new zone ");
    if (size <= TINY_CHUNK_SIZE) {
        printf("TINY\n");
        zone_size = TINY_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
        zone_size += page_size - zone_size % page_size;
    } else if (size <= SMALL_CHUNK_SIZE) {
        printf("SMALL\n");
        zone_size = SMALL_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
        zone_size += page_size - zone_size % page_size;
    } else {
        //TODO: Handle large zones
        dprintf(STDERR_FILENO, "zone_create(): size too big\n");
        return NULL;
    }
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
    new_zone->size = zone_size;
    new_zone->free_space = new_zone->size;
    new_zone->next = NULL;
    new_zone->chunk = NULL;
    return new_zone;
}

static chunk_t chunk_create(zone_t zone, chunk_t last, size_t size) {
    chunk_t new_chunk;

    if (zone == NULL) {
        //We are dealing with a large chunk so we need to allocate space
        new_chunk = mmap(
            NULL,
            size + CHUNK_METADATA_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0);
        //TODO: check this
        //We may allocate a bit more space than needed due to page boundaries
        //but since the allocation needs to be contiguous and large chunks are
        //bigger than a page, we can't really reuse this unused space
        if (new_chunk == MAP_FAILED) {
            return NULL;
        }
    }
    else if (last == NULL) {
        new_chunk = (chunk_t)zone->data;
    }
    else {
        new_chunk = (chunk_t)(last->data + last->size);
    }
    if (last != NULL) {
        last->next = new_chunk;
    }
    new_chunk->size = size;
    new_chunk->next = NULL;
    new_chunk->prev = last;
    new_chunk->magic = 0;
    new_chunk->magic |= MAGIC_SERIALIZE((uintptr_t)new_chunk->data);
    new_chunk->free = 0;
    if (zone != NULL) {
        zone->free_space -= size + CHUNK_METADATA_SIZE;
    }
    return new_chunk;
}

static chunk_t  chunk_validate(void *addr, zone_t *zone) {
    chunk_t chunk = (chunk_t)(addr - CHUNK_METADATA_SIZE);
    uintptr_t magic;
    uintptr_t data;
    //TODO first validate that the address is in a zone range
    //we check if the address is in a tiny zone
    *zone = zone_validate((uintptr_t)addr, tiny_head);
    if (*zone == NULL) {
        //if not we check the small zone
        *zone = zone_validate((uintptr_t)addr, small_head);
    }

    // we deserialize chunk->magic to remove chunk->free
    magic = MAGIC_DESERIALIZE(chunk->magic);
    // we serialize and deserialize chunk->data reproduce the same process magic goes though
    data = MAGIC_DESERIALIZE(MAGIC_SERIALIZE((uintptr_t)chunk->data));

    //now if they match it's very likely that the address given is a correct chunk
    return magic == data ? chunk : NULL;
}

static zone_t  zone_validate(uintptr_t addr, zone_t head) {
    while (head) {
        if (addr >= (uintptr_t)head->data && addr < (uintptr_t)head->data + head->size) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

//TODO: doxygen
//this function assume that 'chunk' is wide enough to contain 'size' + CHUNK_METADATA_SIZE + ALIGN_SIZE
static void chunk_split(chunk_t chunk, size_t size) {
    chunk_t new_chunk;

    new_chunk = (chunk_t)(chunk->data + size);
    new_chunk->size = chunk->size - size - CHUNK_METADATA_SIZE;
    new_chunk->next = chunk->next;
    new_chunk->prev = chunk;
    new_chunk->magic = 0;
    new_chunk->magic |= MAGIC_SERIALIZE((uintptr_t)new_chunk->data);
    new_chunk->free = 1;
    chunk->size = size;
    chunk->next = new_chunk;
}

static void chunk_fusion(chunk_t chunk) {
    chunk_fusion_next(chunk);
    chunk_fusion_prev(chunk);
}

static void chunk_fusion_next(chunk_t chunk) {
    if (chunk->next && chunk->next->free) {
        chunk->size += CHUNK_METADATA_SIZE + chunk->next->size;
        chunk->next = chunk->next->next;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
    }
}

static void chunk_fusion_prev(chunk_t chunk) {
    if (chunk->prev && chunk->prev->free) {
        chunk->prev->size += CHUNK_METADATA_SIZE + chunk->size;
        chunk->prev->next = chunk->next;
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
    }
}

static void chunk_copy(chunk_t src, chunk_t dst) {
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

static zone_t zone_search(zone_t head, zone_t *last, size_t size) {
    //TODO: checking free_space isn't enough because freed chunk can add
    //free_space and can't always be used here
    while (head && head->free_space < size + CHUNK_METADATA_SIZE) {
        *last = head;
        head = head->next;
    }
    return head;
}

static chunk_t chunk_search(chunk_t head, chunk_t *last, size_t size) {
    while (head && !(head->free && head->size >= size)) {
        *last = head;
        head = head->next;
    }
    return head;
}

void show_alloc_mem(void) {
    printf("--------------------\n");
    printf("TINY : %p\n", tiny_head->data);
    zone_show_mem(tiny_head);
    printf("SMALL: %p\n", small_head->data);
    zone_show_mem(small_head);
    if (large_head) {
        printf("LARGE : %p\n", large_head);
        chunk_show_mem(large_head);
    }
    printf("--------------------\n");
}

static void zone_show_mem(zone_t zone) {
    while (zone) {
        chunk_show_mem(zone->chunk);
        zone = zone->next;
    }
}

static void chunk_show_mem(chunk_t chunk) {
    while (chunk) {
        printf("%p - %p : %zu bytes ", chunk->data, chunk->data + chunk->size, chunk->size);
        if (chunk->free) {
            printf("*FREE*");
        }
        printf("\n");
        chunk = chunk->next;
    }
}