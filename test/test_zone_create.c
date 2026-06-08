#include "unity.h"

#include <unistd.h>
#include "chunk.h"
#include "zone.h"

static void zone_create_test(size_t requested_size, size_t chunk_size);
void test_zone_create_tiny(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_zone_create_tiny);

    return UNITY_END();
}

void test_zone_create_tiny(void) {
    zone_create_test(0, TINY_CHUNK_SIZE);
    zone_create_test(TINY_CHUNK_SIZE / 2, TINY_CHUNK_SIZE);
    zone_create_test(TINY_CHUNK_SIZE, TINY_CHUNK_SIZE);
}

void test_zone_create_small(void) {
    zone_create_test(TINY_CHUNK_SIZE + 1, SMALL_CHUNK_SIZE);
    zone_create_test(SMALL_CHUNK_SIZE / 2, SMALL_CHUNK_SIZE);
    zone_create_test(SMALL_CHUNK_SIZE, SMALL_CHUNK_SIZE);
}

void test_zone_create_large(void) {
    TEST_ASSERT_NULL(zone_new(NULL, SMALL_CHUNK_SIZE + 1));
}

static void zone_create_test(size_t requested_size, size_t chunk_size) {
    zone_t zone;
    zone_t last;
    chunk_t chunk;
    const size_t page_size = sysconf(_SC_PAGESIZE);

    zone = zone_new(NULL, requested_size);
    TEST_ASSERT_NOT_NULL(zone);
    TEST_ASSERT_NULL(zone->next);
    TEST_ASSERT_GREATER_OR_EQUAL(chunk_size * CHUNK_PER_ZONE, zone->size);
    //Zone size should be page-aligned
    TEST_ASSERT_EQUAL(0, (zone->size + ZONE_METADATA_SIZE) % page_size);
    TEST_ASSERT_GREATER_OR_EQUAL(requested_size, zone->size);

    chunk = zone_get_chunk(zone);
    TEST_ASSERT_EQUAL(chunk->size, zone->size - CHUNK_METADATA_SIZE);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    TEST_ASSERT_EQUAL(chunk->free, 1);
    TEST_ASSERT_EQUAL(chunk->data, (void*)chunk + CHUNK_METADATA_SIZE);
    TEST_ASSERT_EQUAL((chunk->magic >> 8), chunk->data);

    last = zone;
    zone = zone_new(last, requested_size);
    TEST_ASSERT_NOT_NULL(zone);
    TEST_ASSERT_NULL(zone->next);
    TEST_ASSERT_GREATER_OR_EQUAL(chunk_size * CHUNK_PER_ZONE, zone->size);
    //Zone size should be page-aligned
    TEST_ASSERT_EQUAL(0, (zone->size + ZONE_METADATA_SIZE) % page_size);
    TEST_ASSERT_EQUAL(last->next, zone);

    chunk = zone_get_chunk(zone);
    TEST_ASSERT_EQUAL(chunk->size, zone->size - CHUNK_METADATA_SIZE);
    TEST_ASSERT_NULL(chunk->next);
    TEST_ASSERT_NULL(chunk->prev);
    TEST_ASSERT_EQUAL(chunk->free, 1);
    TEST_ASSERT_EQUAL(chunk->data, (void*)chunk + CHUNK_METADATA_SIZE);
    TEST_ASSERT_EQUAL((chunk->magic >> 8), chunk->data);
}