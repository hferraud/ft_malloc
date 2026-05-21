#include "unity.h"

#include "../src/malloc.c"

void setUp(void) {}
void tearDown(void) {}

void test_chunk_fusion_single(void) {
    const size_t CHUNK_SIZE_1 = 256;
    const size_t CHUNK_SIZE_2 = 1024;
    const size_t CHUNK_SIZE_3 = 4096;

    chunk_t chunk;

    chunk = chunk_create(CHUNK_SIZE_1);
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE_1, chunk->size);
    TEST_ASSERT_EQUAL(NULL, chunk->next);
    TEST_ASSERT_EQUAL(NULL, chunk->prev);
    chunk = chunk_create(CHUNK_SIZE_2);
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE_2, chunk->size);
    TEST_ASSERT_EQUAL(NULL, chunk->next);
    TEST_ASSERT_EQUAL(NULL, chunk->prev);
    chunk = chunk_create(CHUNK_SIZE_3);
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE_3, chunk->size);
    TEST_ASSERT_EQUAL(NULL, chunk->next);
    TEST_ASSERT_EQUAL(NULL, chunk->prev);
}

void test_chunk_fusion_prev_free(void) {
    const size_t CHUNK_SIZE = 4096;
    const size_t PREV_SIZE = 2048;

    chunk_t chunk, prev;

    chunk = chunk_create(CHUNK_SIZE);
    prev = chunk_create(PREV_SIZE);
    chunk->prev = prev;
    prev->next = chunk;
    prev->free = 1;
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE + PREV_SIZE + CHUNK_METADATA_SIZE, prev->size);
    TEST_ASSERT_EQUAL(NULL, prev->next);
    TEST_ASSERT_EQUAL(NULL, prev->prev);
}

void test_chunk_fusion_prev_not_free(void) {
    const size_t CHUNK_SIZE = 4096;
    const size_t PREV_SIZE = 2048;

    chunk_t chunk, prev;

    chunk = chunk_create(CHUNK_SIZE);
    prev = chunk_create(PREV_SIZE);
    chunk->prev = prev;
    prev->next = chunk;
    prev->free = 0;
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE, chunk->size);
    TEST_ASSERT_EQUAL(PREV_SIZE, prev->size);
    TEST_ASSERT_EQUAL(prev, chunk->prev);
    TEST_ASSERT_EQUAL(NULL, chunk->next);
    TEST_ASSERT_EQUAL(chunk, prev->next);
    TEST_ASSERT_EQUAL(NULL, prev->prev);
}

void test_chunk_fusion_next_free(void) {
    const size_t CHUNK_SIZE = 4096;
    const size_t NEXT_SIZE = 2048;

    chunk_t chunk, next;

    chunk = chunk_create(CHUNK_SIZE);
    next = chunk_create(NEXT_SIZE);
    chunk->next = next;
    next->prev = chunk;
    next->free = 1;
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE + NEXT_SIZE + CHUNK_METADATA_SIZE, chunk->size);
    TEST_ASSERT_EQUAL(NULL, chunk->next);
    TEST_ASSERT_EQUAL(NULL, chunk->prev);
}

void test_chunk_fusion_next_not_free(void) {
    const size_t CHUNK_SIZE = 4096;
    const size_t NEXT_SIZE = 2048;

    chunk_t chunk, next;

    chunk = chunk_create(CHUNK_SIZE);
    next = chunk_create(NEXT_SIZE);
    chunk->next = next;
    next->prev = chunk;
    next->free = 0;
    chunk_fusion(chunk);
    TEST_ASSERT_EQUAL(CHUNK_SIZE, chunk->size);
    TEST_ASSERT_EQUAL(NEXT_SIZE, next->size);
    TEST_ASSERT_EQUAL(next, chunk->next);
    TEST_ASSERT_EQUAL(NULL, chunk->prev);
    TEST_ASSERT_EQUAL(chunk, next->prev);
    TEST_ASSERT_EQUAL(NULL, next->next);
}
