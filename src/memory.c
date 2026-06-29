#include "memory.h"

memory_t memory_g = {
    .tiny_head = NULL,
    .small_head = NULL,
    .large_head = NULL,
};