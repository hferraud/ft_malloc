#include "display.h"

#include <stdio.h>
#include <zone.h>
#include <chunk.h>

static void zone_display_memory(zone_t zone);
static void chunk_display_memory(chunk_t chunk);

void display_memory(void) {
    printf("--------------------\n");
    if (tiny_head) {
        printf("TINY : %p\n", tiny_head->data);
        zone_display_memory(tiny_head);
    }
    if (small_head) {
        printf("SMALL: %p\n", small_head->data);
        zone_display_memory(small_head);
    }
    if (large_head) {
        printf("LARGE : %p\n", large_head);
        chunk_display_memory(large_head);
    }
    printf("--------------------\n");
}

static void zone_display_memory(zone_t zone) {
    while (zone) {
        chunk_display_memory(zone_get_chunk(zone));
        zone = zone->next;
    }
}

static void chunk_display_memory(chunk_t chunk) {
    while (chunk) {
        printf("%p - %p : %zu bytes ", chunk->data, chunk->data + chunk->size, chunk->size);
        if (chunk->free) {
            printf("*FREE*");
        }
        printf("\n");
        chunk = chunk->next;
    }
}