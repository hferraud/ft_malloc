#include "unity.h"

#include "chunk.h"

static void chunk_create_test(size_t size);
void test_chunk_create_single(void);
void test_chunk_create_multiple(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_chunk_create_single);
    RUN_TEST(test_chunk_create_multiple);

    return UNITY_END();
}

void test_chunk_create_single(void) {
    chunk_create_test(SMALL_CHUNK_SIZE + 1);
    chunk_create_test(SMALL_CHUNK_SIZE * 2);
    chunk_create_test(SMALL_CHUNK_SIZE * 2 - 1);
    chunk_create_test(SMALL_CHUNK_SIZE * 128);
}

static void chunk_create_test(size_t size) {
    chunk_t chunk;

    chunk = chunk_new(size);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_EQUAL(chunk->size, size);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    TEST_ASSERT_EQUAL(chunk->free, 0);
    TEST_ASSERT_EQUAL(chunk->data, (void*)chunk + CHUNK_METADATA_SIZE);
    TEST_ASSERT_EQUAL((chunk->magic >> 8), chunk->data);
}

void test_chunk_create_multiple(void) {
    chunk_t chunk;
    chunk_t prev;
    chunk_t head = NULL;

    chunk = chunk_new(SMALL_CHUNK_SIZE * 1);
    chunk_push_back(&head, chunk);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    prev = chunk;
    chunk = chunk_new(SMALL_CHUNK_SIZE * 2);
    chunk_push_back(&head, chunk);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_EQUAL(chunk->prev, prev);
    TEST_ASSERT_EQUAL(prev->next, chunk);
    TEST_ASSERT_NULL(prev->prev);
    prev = chunk;
    chunk = chunk_new(SMALL_CHUNK_SIZE * 2);
    chunk_push_back(&head, chunk);
    TEST_ASSERT_NOT_NULL(chunk);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_EQUAL(chunk->prev, prev);
    TEST_ASSERT_EQUAL(prev->next, chunk);
    TEST_ASSERT_EQUAL(prev->prev, head);
}
