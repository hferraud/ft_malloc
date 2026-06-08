#include <string.h>

#include "unity.h"

#include "chunk.h"

void test_chunk_init_tiny(void);
void test_chunk_init_small(void);
void test_chunk_init_large(void);
static void chunk_init_test(chunk_t chunk, size_t size);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_chunk_init_tiny);
    RUN_TEST(test_chunk_init_small);
    RUN_TEST(test_chunk_init_large);

    return UNITY_END();
}

void test_chunk_init_tiny(void) {
    chunk_t chunk = chunk_new(TINY_CHUNK_SIZE);

    chunk_init_test(chunk, TINY_CHUNK_SIZE);
    memset(chunk, 0xFF, TINY_CHUNK_SIZE + CHUNK_METADATA_SIZE);
    chunk_init_test(chunk, TINY_CHUNK_SIZE);
}

void test_chunk_init_small(void) {
    chunk_t chunk = chunk_new(SMALL_CHUNK_SIZE);

    chunk_init_test(chunk, SMALL_CHUNK_SIZE);
    memset(chunk, 0xFF, SMALL_CHUNK_SIZE + CHUNK_METADATA_SIZE);
    chunk_init_test(chunk, SMALL_CHUNK_SIZE);
}

void test_chunk_init_large(void) {
    const size_t LARGE_CHUNK_SIZE = SMALL_CHUNK_SIZE * 8;
    chunk_t chunk = chunk_new(LARGE_CHUNK_SIZE);

    chunk_init_test(chunk, LARGE_CHUNK_SIZE);
    memset(chunk, 0xFF, LARGE_CHUNK_SIZE + CHUNK_METADATA_SIZE);
    chunk_init_test(chunk, LARGE_CHUNK_SIZE);
}

static void chunk_init_test(chunk_t chunk, size_t size) {

    chunk_init(chunk, size);
    TEST_ASSERT_EQUAL(chunk->size, size);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    TEST_ASSERT_EQUAL(chunk->free, 0);
    TEST_ASSERT_EQUAL(chunk->data, (void*)chunk + CHUNK_METADATA_SIZE);
    TEST_ASSERT_EQUAL((chunk->magic >> 8), chunk->data);
}
