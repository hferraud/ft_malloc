#ifndef MEMORY_H
#define MEMORY_H

#include "zone.h"

typedef struct {
    zone_t tiny_head;
    zone_t small_head;
    chunk_t large_head;
} memory_t;

extern memory_t memory_g;

#endif //MEMORY_H