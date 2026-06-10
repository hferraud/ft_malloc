#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <stddef.h>

#define CHUNK_PER_ZONE      128
#define TINY_CHUNK_SIZE     128
#define SMALL_CHUNK_SIZE    4096
#define CHUNK_METADATA_SIZE (sizeof(void*) * 3 + sizeof(size_t))

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

extern chunk_t large_head;

chunk_t     chunk_get(size_t size);
void        chunk_init(chunk_t chunk, size_t size);
chunk_t     chunk_new(size_t size);
void        chunk_push_back(chunk_t *head, chunk_t chunk);
chunk_t     chunk_search(chunk_t c_head, size_t size);
chunk_t     chunk_split(chunk_t chunk, size_t size);
void        chunk_fusion(chunk_t chunk);
void        chunk_fusion_next(chunk_t chunk);
void        chunk_fusion_prev(chunk_t chunk);
void        chunk_copy(chunk_t src, chunk_t dst);
chunk_t     chunk_validate(void *addr, zone_t *zone, zone_t **zone_head);

#endif //CHUNK_H