#include "unity.h"

#include "free.h"
#include "malloc.h"
#include "chunk.h"

#define LARGE_CHUNK_SIZE (SMALL_CHUNK_SIZE * 8)

void test_free_basic_tiny(void);
void test_free_basic_small(void);
void test_free_basic_large_next_chunk_freed(void);
void test_free_basic_large_prev_chunk_freed(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    //Basic tests, we try to malloc and free twice and see if everything is good
    RUN_TEST(test_free_basic_tiny);
    RUN_TEST(test_free_basic_small);
    RUN_TEST(test_free_basic_large_next_chunk_freed);
    RUN_TEST(test_free_basic_large_prev_chunk_freed);

    return UNITY_END();
}

void test_free_basic_tiny(void) {
    const size_t CHUNK_SIZE = TINY_CHUNK_SIZE;
    void *addr;
    chunk_t chunk;
    size_t free_chunk_size;

    addr = ft_malloc(CHUNK_SIZE);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_FALSE(chunk->free);
    free_chunk_size = chunk->next->size;
    ft_free(addr);
    TEST_ASSERT_TRUE(chunk->free);
    TEST_ASSERT_EQUAL(free_chunk_size + CHUNK_SIZE + CHUNK_METADATA_SIZE, chunk->size);
    ft_malloc(CHUNK_SIZE);
    TEST_ASSERT_FALSE(chunk->free);
    TEST_ASSERT_EQUAL(free_chunk_size, chunk->next->size);
    ft_free(addr);
    TEST_ASSERT_TRUE(chunk->free);
    TEST_ASSERT_EQUAL(free_chunk_size + CHUNK_SIZE + CHUNK_METADATA_SIZE, chunk->size);
}

void test_free_basic_small(void) {
    const size_t CHUNK_SIZE = SMALL_CHUNK_SIZE;
    void *addr;
    chunk_t chunk;
    size_t free_chunk_size;

    addr = ft_malloc(CHUNK_SIZE);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_FALSE(chunk->free);
    free_chunk_size = chunk->next->size;

    ft_free(addr);
    TEST_ASSERT_TRUE(chunk->free);
    TEST_ASSERT_EQUAL(free_chunk_size + CHUNK_SIZE + CHUNK_METADATA_SIZE, chunk->size);

    ft_malloc(CHUNK_SIZE);
    TEST_ASSERT_FALSE(chunk->free);
    TEST_ASSERT_EQUAL(free_chunk_size, chunk->next->size);

    ft_free(addr);
    TEST_ASSERT_TRUE(chunk->free);
    TEST_ASSERT_EQUAL(free_chunk_size + CHUNK_SIZE + CHUNK_METADATA_SIZE, chunk->size);
}

void test_free_basic_large_next_chunk_freed(void) {
    const size_t CHUNK_SIZE = SMALL_CHUNK_SIZE * 8;
    void *addr1, *addr2;
    chunk_t chunk1;

    addr1 = ft_malloc(CHUNK_SIZE);
    chunk1 = addr1 - CHUNK_METADATA_SIZE;
    TEST_ASSERT_FALSE(chunk1->free);
    addr2 = ft_malloc(CHUNK_SIZE);
    TEST_ASSERT_NOT_NULL(chunk1->next);

    TEST_ASSERT_EQUAL(chunk1, large_head);
    ft_free(addr2);
    //Here the chunk should be unmapped
    //TODO: check if munmap was called
    TEST_ASSERT_NULL(chunk1->next);
    ft_free(addr1);
    //Here the chunk should be unmapped
    //TODO: check if munmap was called
    TEST_ASSERT_NULL(large_head);
}

void test_free_basic_large_prev_chunk_freed(void) {
    const size_t CHUNK_SIZE = SMALL_CHUNK_SIZE * 8;
    void *addr1, *addr2;
    chunk_t chunk1, chunk2;

    addr1 = ft_malloc(CHUNK_SIZE);
    chunk1 = addr1 - CHUNK_METADATA_SIZE;
    addr2 = ft_malloc(CHUNK_SIZE);
    chunk2 = addr2 - CHUNK_METADATA_SIZE;
    TEST_ASSERT_NOT_NULL(chunk2->prev);

    TEST_ASSERT_EQUAL(chunk1, large_head);
    ft_free(addr1);
    //Here the chunk should be unmapped
    //TODO: check if munmap was called
    TEST_ASSERT_NULL(chunk2->prev);
    TEST_ASSERT_EQUAL(chunk2, large_head);
    ft_free(addr2);
    //Here the chunk should be unmapped
    //TODO: check if munmap was called
    TEST_ASSERT_NULL(large_head);
}
