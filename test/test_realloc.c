#include <string.h>

#include "unity.h"

#include "realloc.h"
#include "chunk.h"
#include "def.h"
#include "free.h"
#include "zone.h"

#define LARGE_CHUNK_SIZE (SMALL_CHUNK_SIZE * 8)

void setUp(void) {}
void tearDown(void) {}

void test_realloc_tiny_smaller_new_size(void);
void test_realloc_small_smaller_new_size(void);
void test_realloc_large_new_size(void);
void test_realloc_tiny_bigger_new_size_contiguous_no_gap();
void test_realloc_tiny_bigger_new_size_contiguous_with_gap();
void test_realloc_small_bigger_new_size_contiguous_no_gap();
void test_realloc_small_bigger_new_size_contiguous_with_gap();
void test_realloc_tiny_new_allocation_same_zone();
void test_realloc_small_new_allocation_same_zone();
void test_realloc_tiny_new_allocation_new_zone();
void test_realloc_small_new_allocation_new_zone();

void realloc_smaller_new_size_test(size_t chunk_size);
void realloc_bigger_new_size_contiguous_no_gap_test(size_t chunk_size);
void realloc_bigger_new_size_contiguous_with_gap_test(size_t chunk_size);
void realloc_new_allocation_same_zone_test(size_t chunk_size);
void realloc_new_allocation_new_zone_test(size_t chunk_size);
void realloc_fill_chunk_test(chunk_t chunk, size_t len);
void realloc_fill_chunk(chunk_t chunk);

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_realloc_tiny_smaller_new_size);
    RUN_TEST(test_realloc_small_smaller_new_size);
    RUN_TEST(test_realloc_large_new_size);
    RUN_TEST(test_realloc_tiny_bigger_new_size_contiguous_no_gap);
    RUN_TEST(test_realloc_small_bigger_new_size_contiguous_no_gap);
    RUN_TEST(test_realloc_tiny_bigger_new_size_contiguous_with_gap);
    RUN_TEST(test_realloc_small_bigger_new_size_contiguous_with_gap);
    RUN_TEST(test_realloc_tiny_new_allocation_same_zone);
    RUN_TEST(test_realloc_small_new_allocation_same_zone);
    RUN_TEST(test_realloc_tiny_new_allocation_new_zone);
    RUN_TEST(test_realloc_small_new_allocation_new_zone);

    return UNITY_END();
}

void test_realloc_tiny_smaller_new_size() {
    realloc_smaller_new_size_test(TINY_CHUNK_SIZE);
}

void test_realloc_small_smaller_new_size() {
    realloc_smaller_new_size_test(SMALL_CHUNK_SIZE);
}

void test_realloc_large_new_size() {
    void *addr_1 = ft_realloc(NULL, LARGE_CHUNK_SIZE);
    chunk_t chunk_1 = addr_1 - CHUNK_METADATA_SIZE;

    realloc_fill_chunk(chunk_1);
    void *addr_2 = ft_realloc(addr_1, LARGE_CHUNK_SIZE / 2);
    chunk_t chunk_2 = addr_2 - CHUNK_METADATA_SIZE;
    realloc_fill_chunk_test(chunk_2, chunk_2->size);
    TEST_ASSERT_NOT_EQUAL(addr_1, addr_2);
    TEST_ASSERT_EQUAL(LARGE_CHUNK_SIZE / 2, chunk_2->size);
    TEST_ASSERT_EQUAL(chunk_2, large_head);
    ft_free(addr_2);
}

void test_realloc_tiny_bigger_new_size_contiguous_no_gap() {
    realloc_bigger_new_size_contiguous_no_gap_test(TINY_CHUNK_SIZE);
}

void test_realloc_tiny_bigger_new_size_contiguous_with_gap() {
    realloc_bigger_new_size_contiguous_with_gap_test(TINY_CHUNK_SIZE);
}

void test_realloc_small_bigger_new_size_contiguous_no_gap() {
    realloc_bigger_new_size_contiguous_no_gap_test(SMALL_CHUNK_SIZE);
}

void test_realloc_small_bigger_new_size_contiguous_with_gap() {
    realloc_bigger_new_size_contiguous_with_gap_test(SMALL_CHUNK_SIZE);
}


void test_realloc_tiny_new_allocation_same_zone() {
    realloc_new_allocation_same_zone_test(TINY_CHUNK_SIZE);
}

void test_realloc_small_new_allocation_same_zone() {
    realloc_new_allocation_same_zone_test(SMALL_CHUNK_SIZE);
}

void test_realloc_tiny_new_allocation_new_zone() {
    void *addr_1, *addr_2;
    chunk_t chunk_1, chunk_2, new_chunk;

    addr_1 = ft_realloc(NULL, TINY_CHUNK_SIZE);
    addr_2 = ft_realloc(NULL, TINY_CHUNK_SIZE);
    chunk_1 = addr_1 - CHUNK_METADATA_SIZE;
    chunk_2 = addr_2 - CHUNK_METADATA_SIZE;
    realloc_fill_chunk(chunk_1);
    new_chunk = ft_realloc(addr_1, SMALL_CHUNK_SIZE) - CHUNK_METADATA_SIZE;
    realloc_fill_chunk_test(new_chunk, TINY_CHUNK_SIZE);
    TEST_ASSERT_TRUE(chunk_1->free);
    TEST_ASSERT_EQUAL(new_chunk, small_head->data);
    TEST_ASSERT_EQUAL(chunk_2, chunk_1->next);
    TEST_ASSERT_EQUAL(SMALL_CHUNK_SIZE, new_chunk->size);
    ft_free(addr_2);
    ft_free(new_chunk->data);
}

void test_realloc_small_new_allocation_new_zone() {
    void *addr_1, *addr_2;
    chunk_t chunk_1, chunk_2, new_chunk;

    addr_1 = ft_realloc(NULL, SMALL_CHUNK_SIZE);
    addr_2 = ft_realloc(NULL, SMALL_CHUNK_SIZE);
    chunk_1 = addr_1 - CHUNK_METADATA_SIZE;
    chunk_2 = addr_2 - CHUNK_METADATA_SIZE;
    realloc_fill_chunk(chunk_1);
    new_chunk = ft_realloc(addr_1, LARGE_CHUNK_SIZE) - CHUNK_METADATA_SIZE;
    realloc_fill_chunk_test(new_chunk, SMALL_CHUNK_SIZE);
    TEST_ASSERT_TRUE(chunk_1->free);
    TEST_ASSERT_EQUAL(new_chunk, large_head);
    TEST_ASSERT_EQUAL(chunk_2, chunk_1->next);
    TEST_ASSERT_EQUAL(LARGE_CHUNK_SIZE, new_chunk->size);
    ft_free(addr_2);
    ft_free(new_chunk->data);
}

void realloc_smaller_new_size_test(size_t chunk_size) {
    void *addr_1 = ft_realloc(NULL, chunk_size);
    chunk_t chunk = addr_1 - CHUNK_METADATA_SIZE;

    TEST_ASSERT_EQUAL(chunk_size, chunk->size);
    realloc_fill_chunk(chunk);
    void *addr = ft_realloc(addr_1, chunk_size / 2);
    realloc_fill_chunk_test(chunk, chunk->size);
    TEST_ASSERT_EQUAL(addr_1, addr);
    TEST_ASSERT_EQUAL(chunk_size / 2, chunk->size);
    TEST_ASSERT_EQUAL(addr_1 + chunk->size, chunk->next);
    ft_free(addr);
}

void realloc_bigger_new_size_contiguous_no_gap_test(size_t chunk_size) {
    const size_t new_chunk_size = chunk_size * 2 + CHUNK_METADATA_SIZE;
    void *addr_1, *addr_2, *addr_3;
    chunk_t chunk_1, chunk_3;

    addr_1 = ft_realloc(NULL, chunk_size);
    addr_2 = ft_realloc(NULL, chunk_size);
    addr_3 = ft_realloc(NULL, chunk_size);
    chunk_1 = addr_1 - CHUNK_METADATA_SIZE;
    chunk_3 = addr_3 - CHUNK_METADATA_SIZE;
    ft_free(addr_2);
    realloc_fill_chunk(chunk_1);
    ft_realloc(addr_1, new_chunk_size);
    realloc_fill_chunk_test(chunk_1, chunk_size);
    TEST_ASSERT_EQUAL(new_chunk_size, chunk_1->size);
    TEST_ASSERT_EQUAL(chunk_3, chunk_1->next);
    TEST_ASSERT_EQUAL(chunk_1, chunk_3->prev);
    TEST_ASSERT_FALSE(chunk_1->free);
    ft_free(addr_1);
    ft_free(addr_3);
}

void realloc_bigger_new_size_contiguous_with_gap_test(size_t chunk_size) {
    const size_t new_chunk_size = chunk_size * 2 - ALIGN_SIZE;
    void *addr_1, *addr_2, *addr_3;
    chunk_t chunk_1, chunk_2, chunk_3;

    addr_1 = ft_realloc(NULL, chunk_size);
    addr_2 = ft_realloc(NULL, chunk_size);
    addr_3 = ft_realloc(NULL, chunk_size);
    chunk_1 = addr_1 - CHUNK_METADATA_SIZE;
    chunk_3 = addr_3 - CHUNK_METADATA_SIZE;
    ft_free(addr_2);
    realloc_fill_chunk(chunk_1);
    ft_realloc(addr_1, new_chunk_size);
    realloc_fill_chunk_test(chunk_1, chunk_size);
    TEST_ASSERT_EQUAL(new_chunk_size, chunk_1->size);
    chunk_2 = chunk_1->next;
    TEST_ASSERT_EQUAL(chunk_2, chunk_1->next);
    TEST_ASSERT_EQUAL(chunk_2, chunk_3->prev);
    TEST_ASSERT_EQUAL(ALIGN_SIZE, chunk_2->size);
    TEST_ASSERT_FALSE(chunk_1->free);
    TEST_ASSERT_TRUE(chunk_2->free);
    ft_free(addr_1);
    ft_free(addr_3);
}

void realloc_new_allocation_same_zone_test(size_t chunk_size) {
    const size_t new_chunk_size = chunk_size;
    void *addr_1, *addr_2, *addr_3;
    chunk_t chunk_1, chunk_3, new_chunk;

    addr_1 = ft_realloc(NULL, chunk_size / 4);
    addr_2 = ft_realloc(NULL, chunk_size / 4);
    addr_3 = ft_realloc(NULL, chunk_size / 4);
    chunk_1 = addr_1 - CHUNK_METADATA_SIZE;
    chunk_3 = addr_3 - CHUNK_METADATA_SIZE;
    ft_free(addr_2);
    realloc_fill_chunk(chunk_1);
    new_chunk = ft_realloc(addr_1, new_chunk_size) - CHUNK_METADATA_SIZE;
    realloc_fill_chunk_test(new_chunk, chunk_size / 4);
    TEST_ASSERT_EQUAL(new_chunk_size, new_chunk->size);
    TEST_ASSERT_EQUAL(new_chunk, chunk_3->next);
    TEST_ASSERT_EQUAL(chunk_3, new_chunk->prev);
    TEST_ASSERT_EQUAL(new_chunk_size, new_chunk->size);
    TEST_ASSERT_TRUE(chunk_1->free);
    TEST_ASSERT_EQUAL(chunk_1->size, chunk_size / 2 + CHUNK_METADATA_SIZE);
    ft_free(new_chunk->data);
    ft_free(addr_3);
}

void realloc_fill_chunk(chunk_t chunk) {
    uint8_t fill = 0;
    for (size_t i = 0; i < chunk->size; i++) {
        chunk->data[i] = fill++;
    }
}

void realloc_fill_chunk_test(chunk_t chunk, size_t len) {
    uint8_t fill = 0;
    for (size_t i = 0; i < len; i++) {
        TEST_ASSERT_EQUAL(chunk->data[i], fill++);
    }
}
