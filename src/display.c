#include "display.h"

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "memory.h"
#include "chunk.h"
#include "zone.h"

#define HEXDUMP_WORD_SIZE 16

static void zone_display_memory(zone_t zone);
static void chunk_display_memory(chunk_t chunk);
static void zone_display_memory_ex(zone_t zone);
static void chunk_display_memory_ex(chunk_t chunk);
static void hexdump(void* addr, size_t size);
static void hexdump_print_hex(uint8_t *data);
static void hexdump_print_ascii(uint8_t *data);

void display_memory(void) {
    printf("--------------------\n");
    if (memory_g.tiny_head) {
        printf("TINY : %p\n", memory_g.tiny_head->data);
        zone_display_memory(memory_g.tiny_head);
    }
    if (memory_g.small_head) {
        printf("SMALL: %p\n", memory_g.small_head->data);
        zone_display_memory(memory_g.small_head);
    }
    if (memory_g.large_head) {
        printf("LARGE : %p\n", memory_g.large_head);
        chunk_display_memory(memory_g.large_head);
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

void display_memory_ex(void) {
    if (memory_g.tiny_head) {
        printf("--------------- TINY HEAD ---------------\n");
        zone_display_memory_ex(memory_g.tiny_head);
    }
    if (memory_g.small_head) {
        printf("--------------- SMALL HEAD ---------------\n");
        zone_display_memory_ex(memory_g.small_head);
    }
    if (memory_g.large_head) {
        printf("--------------- CHUNK HEAD ---------------\n");
        chunk_display_memory_ex(memory_g.large_head);
    }
}

static void zone_display_memory_ex(zone_t zone) {
    while (zone) {
        printf("[ZONE %p]\n", zone);
        printf(".size: %zu (0x%zx)\n", zone->size, zone->size);
        printf(".next: %p\n", zone->next);
        printf(".data: %p\n", zone->data);
        hexdump(zone, ZONE_METADATA_SIZE);
        chunk_display_memory_ex(zone_get_chunk(zone));
        printf("[ZONE END]\n");
        zone = zone->next;
    }
}

static void chunk_display_memory_ex(chunk_t chunk) {
    while (chunk) {
        printf("[CHUNK %p]\n", chunk);
        printf(".size:  %zu (0x%zx)\n", chunk->size, chunk->size);
        printf(".next:  %p\n", chunk->next);
        printf(".prev:  %p\n", chunk->prev);
        printf(".free:  %d\n", chunk->free);
        printf(".magic: 0x%zx\n", chunk->magic >> 8);
        printf(".data:  %p\n", chunk->data);
        hexdump(chunk, ZONE_METADATA_SIZE + chunk->size);
        printf("[CHUNK END]\n");
        chunk = chunk->next;
    }
}

static void hexdump(void* addr, size_t size) {
    uint8_t buf[16];
    uint8_t old_buf[16];
    bool print_repeat_char = true;

    memcpy(old_buf, addr, 16);
    printf("%p  ", addr);
    hexdump_print_hex(old_buf);
    hexdump_print_ascii(old_buf);
    for (size_t i = 16; i < size; i += 16) {
        memcpy(buf, addr + i, 16);
        if (i < size - HEXDUMP_WORD_SIZE && memcmp(buf, old_buf, 16) == 0) {
            if (print_repeat_char) {
                printf("*\n");
                print_repeat_char = false;
            }
            continue;
        }
        print_repeat_char = true;
        printf("%p  ", addr + i);
        hexdump_print_hex(buf);
        hexdump_print_ascii(buf);
        memcpy(old_buf, buf, 16);
    }
}

static void hexdump_print_hex(uint8_t *data) {
    for (size_t i = 0; i < HEXDUMP_WORD_SIZE; i++) {
        if (i == HEXDUMP_WORD_SIZE / 2) {
            printf(" ");
        }
        printf("%02x ", data[i]);
    }
}

static void hexdump_print_ascii(uint8_t *data) {
    printf("|");
    for (size_t i = 0; i < HEXDUMP_WORD_SIZE; i++) {
        if (i == HEXDUMP_WORD_SIZE / 2) {
            printf(" ");
        }
        if (isprint(data[i])) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    printf("|\n");
}
