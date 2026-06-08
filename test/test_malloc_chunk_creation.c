#include "unity.h"


#include "malloc.h"
#include "chunk.h"

#define LARGE_CHUNK_SIZE (SMALL_CHUNK_SIZE * 8)

static void test_malloc_chunk_creation_tiny(void);
static void test_malloc_chunk_creation_small(void);
static void test_malloc_chunk_creation_large(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    //Here we check that malloc create chunk correctly
    //ie by splitting the big chunk that is in the newly created zone
    RUN_TEST(test_malloc_chunk_creation_tiny);
    RUN_TEST(test_malloc_chunk_creation_small);
    //The check for a large chunk are a bit different
    //since they don't live in a zone
    RUN_TEST(test_malloc_chunk_creation_large);

    return UNITY_END();
}

static void test_malloc_chunk_creation_tiny(void) {
    void *addr1, *addr2;
    size_t chunk_free_size;

    addr1 = ft_malloc(TINY_CHUNK_SIZE);
    chunk_t chunk1 = addr1 - CHUNK_METADATA_SIZE;
    chunk_t chunk_free = chunk1->next;
    TEST_ASSERT_EQUAL(TINY_CHUNK_SIZE, chunk1->size);
    TEST_ASSERT_NULL(chunk1->prev);
    TEST_ASSERT_TRUE(chunk_free->free);
    chunk_free_size = chunk_free->size;

    addr2 = ft_malloc(TINY_CHUNK_SIZE);
    chunk_t chunk2 = addr2 - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(TINY_CHUNK_SIZE, chunk2->size);
    chunk_free = chunk2->next;
    TEST_ASSERT_EQUAL(chunk1, chunk2->prev);
    TEST_ASSERT_EQUAL(chunk2, chunk1->next);
    TEST_ASSERT_EQUAL(chunk_free_size - chunk2->size - CHUNK_METADATA_SIZE, chunk_free->size);
}

static void test_malloc_chunk_creation_small(void) {
    void *addr1, *addr2;
    size_t chunk_free_size;

    addr1 = ft_malloc(SMALL_CHUNK_SIZE);
    chunk_t chunk1 = addr1 - CHUNK_METADATA_SIZE;
    chunk_t chunk_free = chunk1->next;
    TEST_ASSERT_EQUAL(SMALL_CHUNK_SIZE, chunk1->size);
    TEST_ASSERT_NULL(chunk1->prev);
    TEST_ASSERT_TRUE(chunk_free->free);
    chunk_free_size = chunk_free->size;

    addr2 = ft_malloc(SMALL_CHUNK_SIZE);
    chunk_t chunk2 = addr2 - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(SMALL_CHUNK_SIZE, chunk2->size);
    chunk_free = chunk2->next;
    TEST_ASSERT_EQUAL(chunk1, chunk2->prev);
    TEST_ASSERT_EQUAL(chunk2, chunk1->next);
    TEST_ASSERT_EQUAL(chunk_free_size - chunk2->size - CHUNK_METADATA_SIZE, chunk_free->size);
}

static void test_malloc_chunk_creation_large(void) {
    void *addr1, *addr2;

    addr1 = ft_malloc(LARGE_CHUNK_SIZE);
    chunk_t chunk1 = addr1 - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(LARGE_CHUNK_SIZE, chunk1->size);
    TEST_ASSERT_NULL(chunk1->next);
    TEST_ASSERT_NULL(chunk1->prev);

    addr2 = ft_malloc(LARGE_CHUNK_SIZE);
    chunk_t chunk2 = addr2 - CHUNK_METADATA_SIZE;
    TEST_ASSERT_EQUAL(LARGE_CHUNK_SIZE, chunk2->size);
    TEST_ASSERT_NULL(chunk2->next);
    TEST_ASSERT_EQUAL(chunk1, chunk2->prev);
    TEST_ASSERT_EQUAL(chunk2, chunk1->next);
}
