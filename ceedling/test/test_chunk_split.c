#include <string.h>

#include "unity.h"

#include "../src/malloc.c"

static void chunk_split_test(chunk_t chunk, size_t new_size);

void setUp(void) {}
void tearDown(void) {}

void test_chunk_split_single(void) {
    const size_t OLD_SIZE = 1024;
    const size_t NEW_SIZE = 512;

    chunk_t chunk;
    chunk_t head = NULL;

    chunk = chunk_create(OLD_SIZE);
    chunk_split_test(chunk, NEW_SIZE);
}

void test_chunk_split_many(void) {
    const size_t OLD_SIZE = 2048;
    const size_t NEW_SIZE_1 = 1024;
    const size_t NEW_SIZE_2 = 512;

    chunk_t chunk, new_chunk_1;

    chunk = chunk_create(OLD_SIZE);
    chunk_split_test(chunk, NEW_SIZE_1);
    new_chunk_1 = chunk->next;
    chunk_split_test(chunk, NEW_SIZE_2);
    chunk_split_test(new_chunk_1, NEW_SIZE_2);
}


void test_chunk_split_filled(void) {
    const size_t OLD_SIZE = 1024;
    const size_t NEW_SIZE = 512;

    chunk_t chunk;

    chunk = chunk_create(OLD_SIZE);
    memset(chunk->data, 0xff, chunk->size);
    chunk_split_test(chunk, NEW_SIZE);
}

static void chunk_split_test(chunk_t chunk, size_t new_size) {
    chunk_t new_chunk;
    chunk_t prev = chunk->prev;
    chunk_t next = chunk->next;
    size_t old_size = chunk->size;

    chunk_split(chunk, new_size);
    TEST_ASSERT_EQUAL(chunk->size, new_size);
    TEST_ASSERT_NOT_NULL(chunk->next);
    TEST_ASSERT_EQUAL(chunk->prev, prev);
    new_chunk = chunk->next;
    TEST_ASSERT_EQUAL(new_chunk->size, old_size - new_size - CHUNK_METADATA_SIZE);
    TEST_ASSERT_EQUAL(new_chunk->prev, chunk);
    TEST_ASSERT_EQUAL(new_chunk->next, next);
    TEST_ASSERT_EQUAL(MAGIC_DESERIALIZE(new_chunk->magic), new_chunk->data);
    TEST_ASSERT_EQUAL(new_chunk->free, 1);
}
