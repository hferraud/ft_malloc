#include "malloc.h"

#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>

//TODO: remove
#include <stdio.h>

#define CHUNK_PER_ZONE      128
#define TINY_CHUNK_SIZE     128
#define SMALL_CHUNK_SIZE    4096
#define ZONE_METADATA_SIZE  24

#define BLOCK_SIZE          24

typedef struct chunk_s *chunk_t;
typedef struct zone_s *zone_t;

struct chunk_s {
    size_t          size;
    struct chunk_s  *next;
    int32_t         free;
    uint8_t         data[1];
};

struct zone_s {
    size_t              size;
    size_t              free_space;
    chunk_t             chunk;
    struct zone_s       *next;
};

static void* allocate(void* addr, size_t size);

static chunk_t get_chunk(size_t size);
static zone_t search_zone(zone_t head, zone_t *last, size_t size);
static chunk_t search_chunk(chunk_t head, chunk_t *last, size_t size);
static zone_t create_zone(zone_t last, size_t size);
static chunk_t create_chunk(zone_t zone, chunk_t last, size_t size);

static zone_t tiny_head = NULL;
static zone_t small_head = NULL;
static zone_t large_head = NULL;

void *ft_malloc(size_t size) {
    chunk_t chunk;

    chunk = get_chunk(size);
    if (chunk == NULL) {
        return NULL;
    }
    return chunk->data;
}

static chunk_t get_chunk(size_t size) {
    zone_t *zone_head;
    zone_t zone;
    zone_t last_zone;
    chunk_t chunk;
    chunk_t last_chunk;

    if (size < TINY_CHUNK_SIZE) {
        zone_head = &tiny_head;
    } else if (size < SMALL_CHUNK_SIZE) {
        zone_head = &small_head;
    } else {
        //TODO: Handle large zones
        zone_head = &large_head;
    }

    last_zone = NULL;
    zone = search_zone(*zone_head, &last_zone, size);
    if (zone == NULL) {
        zone = create_zone(last_zone, size);
        if (zone == NULL) {
            return NULL;
        }
        if (*zone_head == NULL) {
            *zone_head = zone;
        }
    }
    last_chunk = NULL;
    chunk = search_chunk(zone->chunk, &last_chunk, size);
    if (chunk == NULL) {
        chunk = create_chunk(zone, last_chunk, size);
    }
    return chunk;
}

static zone_t search_zone(zone_t head, zone_t *last, size_t size) {
    while (head && head->free_space < size) {
        *last = head;
        head = head->next;
    }
    return head;
}

static chunk_t search_chunk(chunk_t head, chunk_t *last, size_t size) {
    while (head && !head->free && head->size < size + BLOCK_SIZE) {
        *last = head;
        head = head->next;
    }
    return head;
}

static zone_t create_zone(zone_t last, size_t size) {
    zone_t new_zone;
    size_t zone_size;
    int page_size = getpagesize();

    if (size < TINY_CHUNK_SIZE) {
        zone_size = TINY_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
        zone_size += page_size - zone_size % page_size;
    } else if (size < SMALL_CHUNK_SIZE) {
        zone_size = SMALL_CHUNK_SIZE * CHUNK_PER_ZONE + ZONE_METADATA_SIZE;
        zone_size += page_size - zone_size % page_size;
    } else {
        //TODO: Handle large zones
        zone_size = size;
    }
    //TODO: Check fail
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

static chunk_t create_chunk(zone_t zone, chunk_t last, size_t size) {
    chunk_t new_chunk;

    new_chunk = (chunk_t)last->data + last->size;
    if (last != NULL) {
        last->next = new_chunk;
    }
    new_chunk->size = size;
    new_chunk->free = 0;
    new_chunk->next = NULL;
    return new_chunk;
}