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
};

static chunk_t  chunk_get(size_t size);
static zone_t   zone_search(zone_t head, zone_t *last, size_t size);
static chunk_t  chunk_search(chunk_t head, chunk_t *last, size_t size);
static zone_t   zone_create(zone_t last, size_t size);
static chunk_t  chunk_create(zone_t zone, chunk_t last, size_t size);
static void     chunk_fusion(chunk_t chunk);
static void     zone_show_mem(zone_t zone);
static void     chunk_show_mem(chunk_t chunk);
static chunk_t  chunk_validate(void *addr, zone_t *zone);
static zone_t  zone_validate(uintptr_t addr, zone_t head);

static zone_t tiny_head = NULL;
static zone_t small_head = NULL;
static zone_t large_head = NULL;

void *ft_malloc(size_t size) {
    chunk_t chunk;

    size = ALIGN_MEM(size);
    // printf("alignment %zu\n", alignof(max_align_t));
    // printf("sizeof size: %zu\n", sizeof(chunk->size));
    // printf("sizeof next: %zu\n", sizeof(chunk_t));
    // printf("sizeof free: %zu\n", sizeof(chunk->free));
    chunk = chunk_get(size);
    if (chunk == NULL) {
        return NULL;
    }
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
    chunk->free = 1;
    if (zone) {
        zone->free_space += chunk->size + CHUNK_METADATA_SIZE;
    }
    chunk_fusion(chunk);
}

void show_alloc_mem(void) {
    zone_show_mem(tiny_head);
    zone_show_mem(small_head);
}

static chunk_t chunk_get(size_t size) {
    zone_t *zone_head;
    zone_t zone;
    zone_t last_zone;
    chunk_t chunk;
    chunk_t last_chunk;

    if (size <= TINY_CHUNK_SIZE) {
        zone_head = &tiny_head;
    } else if (size <= SMALL_CHUNK_SIZE) {
        zone_head = &small_head;
    } else {
        //TODO: Handle large zones
        zone_head = &large_head;
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

    last_chunk = NULL;
    // we search for a free chunk of sufficient size
    chunk = chunk_search(zone->chunk, &last_chunk, size);
    if (chunk == NULL) {
        // we didn't find any fitting chunk, so we create one
        chunk = chunk_create(zone, last_chunk, size);
    }
    // TODO: We need to split the chunk correctly
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
        zone_size = size;
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

    if (last == NULL) {
        new_chunk = (chunk_t)zone->data;
        zone->chunk = new_chunk;
    } else {
        new_chunk = (chunk_t)(last->data + last->size);
        last->next = new_chunk;
    }
    new_chunk->size = size;
    new_chunk->free = 0;
    new_chunk->next = NULL;
    new_chunk->magic |= MAGIC_SERIALIZE((uintptr_t)new_chunk->data);
    zone->free_space -= size + CHUNK_METADATA_SIZE;
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

    printf("magic: %lx\n", magic);
    printf("data:  %lx\n", data);
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

static void chunk_fusion(chunk_t chunk) {
    if (chunk->next && chunk->next->free) {
        chunk->size += CHUNK_METADATA_SIZE + chunk->next->size;
        chunk->next = chunk->next->next;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
    }
    if (chunk->prev && chunk->prev->free) {
        chunk->prev->size += CHUNK_METADATA_SIZE + chunk->size;
        chunk->prev->next = chunk->next;
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
    }
}

static zone_t zone_search(zone_t head, zone_t *last, size_t size) {
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

static void zone_show_mem(zone_t zone) {
    while (zone) {
        printf("TINY : %p\n", zone->data);
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