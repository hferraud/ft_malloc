#include <stdlib.h>

#include "unity.h"

#include "free.h"
#include "malloc.h"
#include "chunk.h"

#define LARGE_CHUNK_SIZE (SMALL_CHUNK_SIZE * 8)
#define TINY_CHUNK_SIZE_1_1 TINY_CHUNK_SIZE
#define TINY_CHUNK_SIZE_1_2 (TINY_CHUNK_SIZE / 2)
#define SMALL_CHUNK_SIZE_1_1 SMALL_CHUNK_SIZE
#define SMALL_CHUNK_SIZE_1_2 (SMALL_CHUNK_SIZE / 2)

void test_chunk_gap_tiny_fit(void);
void test_chunk_gap_tiny_bigger(void);
void test_chunk_gap_tiny_smaller(void);
void test_chunk_gap_tiny_many(void);
void test_chunk_gap_small_fit(void);
void test_chunk_gap_small_bigger(void);
void test_chunk_gap_small_smaller(void);
void test_chunk_gap_small_many(void);

chunk_t tiny_lower_gap_chunk, tiny_upper_gap_chunk, tiny_gap_chunk;
chunk_t small_lower_gap_chunk, small_upper_gap_chunk, small_gap_chunk;

void setUp(void) {
    tiny_lower_gap_chunk = ft_malloc(TINY_CHUNK_SIZE_1_1) - CHUNK_METADATA_SIZE;
    tiny_gap_chunk = ft_malloc(TINY_CHUNK_SIZE_1_2) - CHUNK_METADATA_SIZE;
    tiny_upper_gap_chunk = ft_malloc(TINY_CHUNK_SIZE_1_1) - CHUNK_METADATA_SIZE;
    ft_free(tiny_gap_chunk->data);

    small_lower_gap_chunk = ft_malloc(SMALL_CHUNK_SIZE_1_1) - CHUNK_METADATA_SIZE;
    small_gap_chunk = ft_malloc(SMALL_CHUNK_SIZE_1_2) - CHUNK_METADATA_SIZE;
    small_upper_gap_chunk = ft_malloc(SMALL_CHUNK_SIZE_1_1) - CHUNK_METADATA_SIZE;
    ft_free(small_gap_chunk->data);
}
void tearDown(void) {
    ft_free(tiny_lower_gap_chunk->data);
    ft_free(tiny_upper_gap_chunk->data);

    ft_free(small_lower_gap_chunk->data);
    ft_free(small_upper_gap_chunk->data);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_chunk_gap_tiny_fit);
    RUN_TEST(test_chunk_gap_tiny_bigger);
    RUN_TEST(test_chunk_gap_tiny_smaller);
    RUN_TEST(test_chunk_gap_tiny_many);

    RUN_TEST(test_chunk_gap_small_fit);
    RUN_TEST(test_chunk_gap_small_bigger);
    // RUN_TEST(test_chunk_gap_small_smaller);
    RUN_TEST(test_chunk_gap_small_many);

    return UNITY_END();
}

void test_chunk_gap_tiny_fit(void) {
    void *addr;
    chunk_t chunk;

    //Now we try to create a chunk that will fill the gap
    addr = ft_malloc(TINY_CHUNK_SIZE_1_2);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_FALSE(tiny_gap_chunk->free);
    TEST_ASSERT_EQUAL(chunk, tiny_gap_chunk);
    ft_free(addr);
}

void test_chunk_gap_small_fit(void) {
    void *addr;
    chunk_t chunk;

    //Now we try to create a chunk that will fill the gap
    addr = ft_malloc(SMALL_CHUNK_SIZE_1_2);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_FALSE(small_gap_chunk->free);
    TEST_ASSERT_EQUAL(chunk, small_gap_chunk);
    ft_free(addr);
}

void test_chunk_gap_tiny_bigger(void) {
    void *addr;
    chunk_t chunk;

    addr = ft_malloc(TINY_CHUNK_SIZE_1_2 + 1);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(chunk, tiny_upper_gap_chunk->next);
    TEST_ASSERT_TRUE(tiny_gap_chunk->free); //the gap chunk should still be free
    TEST_ASSERT_FALSE(chunk->free);
    ft_free(addr);
}

void test_chunk_gap_small_bigger(void) {
    void *addr;
    chunk_t chunk;

    addr = ft_malloc(SMALL_CHUNK_SIZE_1_2 + 1);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(chunk, small_upper_gap_chunk->next);
    TEST_ASSERT_TRUE(small_gap_chunk->free); //the gap chunk should still be free
    TEST_ASSERT_FALSE(chunk->free);
    ft_free(addr);
}

void test_chunk_gap_tiny_smaller(void) {
    void *addr;
    chunk_t chunk;

    addr = ft_malloc(TINY_CHUNK_SIZE_1_2 / 2);
    chunk = addr - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(chunk, tiny_lower_gap_chunk->next);
    TEST_ASSERT_EQUAL(chunk->next, tiny_upper_gap_chunk->prev);
    TEST_ASSERT_TRUE(chunk->next->free);
    TEST_ASSERT_FALSE(chunk->free);
    ft_free(addr);
}

// void test_chunk_gap_small_smaller(void) {
//     void *addr;
//     chunk_t chunk;
//
//     addr = ft_malloc(SMALL_CHUNK_SIZE_1_2 / 2);
//     chunk = addr - CHUNK_METADATA_SIZE;
//     // TEST_ASSERT_EQUAL(chunk, small_upper_gap_chunk->next);
//     // TEST_ASSERT_TRUE(small_gap_chunk->free); //the gap chunk should still be free
//     // TEST_ASSERT_FALSE(chunk->free);
//     ft_free(addr);
// }

void test_chunk_gap_tiny_many(void) {
    void *addr;
    chunk_t split_chunk_1, split_chunk_2;

    //Now we try to create two chunk inside the gap
    addr = ft_malloc(TINY_CHUNK_SIZE_1_2 / 2 - CHUNK_METADATA_SIZE);
    split_chunk_1 = addr - CHUNK_METADATA_SIZE;
    addr = ft_malloc(TINY_CHUNK_SIZE_1_2 / 2);
    split_chunk_2 = addr - CHUNK_METADATA_SIZE;

    TEST_ASSERT_EQUAL(tiny_lower_gap_chunk, split_chunk_1->prev);
    TEST_ASSERT_EQUAL(split_chunk_1, tiny_lower_gap_chunk->next);

    TEST_ASSERT_EQUAL(split_chunk_1, split_chunk_2->prev);
    TEST_ASSERT_EQUAL(split_chunk_2, split_chunk_1->next);
    TEST_ASSERT_FALSE(split_chunk_1->free);

    TEST_ASSERT_EQUAL(split_chunk_2, tiny_upper_gap_chunk->prev);
    TEST_ASSERT_EQUAL(tiny_upper_gap_chunk, split_chunk_2->next);
    TEST_ASSERT_FALSE(split_chunk_2->free);
    ft_free(split_chunk_1->data);
    ft_free(split_chunk_2->data);
}

void test_chunk_gap_small_many(void) {
    void *addr;
    chunk_t split_chunk_1, split_chunk_2;

    //Now we try to create two chunk inside the gap
    addr = ft_malloc(SMALL_CHUNK_SIZE_1_2 / 2 - CHUNK_METADATA_SIZE);
    split_chunk_1 = addr - CHUNK_METADATA_SIZE;
    addr = ft_malloc(SMALL_CHUNK_SIZE_1_2 / 2);
    split_chunk_2 = addr - CHUNK_METADATA_SIZE;

    TEST_ASSERT_EQUAL(small_lower_gap_chunk, split_chunk_1->prev);
    TEST_ASSERT_EQUAL(split_chunk_1, small_lower_gap_chunk->next);

    TEST_ASSERT_EQUAL(split_chunk_1, split_chunk_2->prev);
    TEST_ASSERT_EQUAL(split_chunk_2, split_chunk_1->next);
    TEST_ASSERT_FALSE(split_chunk_1->free);

    TEST_ASSERT_EQUAL(split_chunk_2, small_upper_gap_chunk->prev);
    TEST_ASSERT_EQUAL(small_upper_gap_chunk, split_chunk_2->next);
    TEST_ASSERT_FALSE(split_chunk_2->free);
    ft_free(split_chunk_1->data);
    ft_free(split_chunk_2->data);
}
