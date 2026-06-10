#ifndef ZONE_H
#define ZONE_H

#include <stdint.h>
#include <stddef.h>

#define ZONE_METADATA_SIZE  (sizeof(void*) + sizeof(size_t))

typedef struct chunk_s *chunk_t;
typedef struct zone_s *zone_t;

struct zone_s {
    size_t          size;
    struct zone_s   *next;
    uint8_t         data[1];
};

zone_t  zone_new(zone_t last, size_t chunk_size);
void    zone_unmap(zone_t* zone_head);
chunk_t zone_search(zone_t z_head, zone_t *z_last, size_t size);
zone_t  zone_validate(uintptr_t addr, zone_t head);
chunk_t zone_get_chunk(zone_t zone);

extern zone_t tiny_head;
extern zone_t small_head;

#endif //ZONE_H