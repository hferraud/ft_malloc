#include "unity.h"

#include "../src/malloc.c"

static void chunk_create_test(size_t size);

void setUp(void) {}
void tearDown(void) {}

void test_chunk_create_single(void) {
    chunk_create_test(SMALL_CHUNK_SIZE + 1);
    chunk_create_test(SMALL_CHUNK_SIZE * 2);
    chunk_create_test(SMALL_CHUNK_SIZE * 2 - 1);
    chunk_create_test(SMALL_CHUNK_SIZE * 128);
}

static void chunk_create_test(size_t size) {
    chunk_t chunk;

    chunk = chunk_create(size);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_EQUAL(chunk->size, size);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    TEST_ASSERT_EQUAL(chunk->free, 0);
    TEST_ASSERT_EQUAL(chunk->data, (void*)chunk + CHUNK_METADATA_SIZE);
    TEST_ASSERT_EQUAL(MAGIC_DESERIALIZE(chunk->magic), chunk->data);
}

void test_chunk_create_multiple(void) {
    chunk_t chunk;
    chunk_t prev;
    chunk_t head = NULL;

    chunk = chunk_create(SMALL_CHUNK_SIZE * 1);
    chunk_push_back(&head, chunk);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    prev = chunk;
    chunk = chunk_create(SMALL_CHUNK_SIZE * 2);
    chunk_push_back(&head, chunk);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_EQUAL(chunk->prev, prev);
    TEST_ASSERT_EQUAL(prev->next, chunk);
    TEST_ASSERT_NULL(prev->prev);
    prev = chunk;
    chunk = chunk_create(SMALL_CHUNK_SIZE * 2);
    chunk_push_back(&head, chunk);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_EQUAL(chunk->prev, prev);
    TEST_ASSERT_EQUAL(prev->next, chunk);
    TEST_ASSERT_EQUAL(prev->prev, head);
}
