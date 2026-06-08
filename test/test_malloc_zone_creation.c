#include "unity.h"

#include <string.h>

#include "malloc.h"
#include "zone.h"
#include "chunk.h"

#define LARGE_CHUNK_SIZE (SMALL_CHUNK_SIZE * 8)

static void test_malloc_zone_creation_tiny(void);
static void test_malloc_zone_creation_large(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_malloc_zone_creation_tiny);
    RUN_TEST(test_malloc_zone_creation_large);

    return UNITY_END();
}

static void test_malloc_zone_creation_tiny(void) {
    const size_t CHUNK_SIZE = TINY_CHUNK_SIZE;

    void *addr = ft_malloc(CHUNK_SIZE);
    zone_t zone1 = addr - CHUNK_METADATA_SIZE - ZONE_METADATA_SIZE;
    TEST_ASSERT_NULL(zone1->next);
    TEST_ASSERT_GREATER_OR_EQUAL(TINY_CHUNK_SIZE * CHUNK_PER_ZONE, zone1->size);
    TEST_ASSERT_LESS_THAN(SMALL_CHUNK_SIZE * CHUNK_PER_ZONE, zone1->size);

    //Now we need to fill the zone with chunks to create a new one
    size_t chunk_nb = zone1->size / (CHUNK_SIZE + CHUNK_METADATA_SIZE) - 1;
    for (size_t i = 0; i < chunk_nb; i++) {
        ft_malloc(CHUNK_SIZE);
    }
    TEST_ASSERT_NULL(zone1->next);
    ft_malloc(CHUNK_SIZE);
    TEST_ASSERT_NOT_NULL(zone1->next);

    zone_t zone2 = zone1->next;
    TEST_ASSERT_NULL(zone2->next);
    TEST_ASSERT_GREATER_OR_EQUAL(TINY_CHUNK_SIZE * CHUNK_PER_ZONE, zone2->size);
    TEST_ASSERT_LESS_THAN(SMALL_CHUNK_SIZE * CHUNK_PER_ZONE, zone2->size);
}

static void test_malloc_zone_creation_large(void) {
    const size_t CHUNK_SIZE = SMALL_CHUNK_SIZE;

    void *addr = ft_malloc(CHUNK_SIZE);
    zone_t zone1 = addr - CHUNK_METADATA_SIZE - ZONE_METADATA_SIZE;
    TEST_ASSERT_NULL(zone1->next);
    TEST_ASSERT_GREATER_OR_EQUAL(SMALL_CHUNK_SIZE * CHUNK_PER_ZONE, zone1->size);

    //Now we need to fill the zone with chunks to create a new one
    size_t chunk_nb = zone1->size / (CHUNK_SIZE + CHUNK_METADATA_SIZE) - 1;
    for (size_t i = 0; i < chunk_nb; i++) {
        ft_malloc(CHUNK_SIZE);
    }
    TEST_ASSERT_NULL(zone1->next);
    ft_malloc(CHUNK_SIZE);
    TEST_ASSERT_NOT_NULL(zone1->next);

    zone_t zone2 = zone1->next;
    TEST_ASSERT_NULL(zone2->next);
    TEST_ASSERT_GREATER_OR_EQUAL(SMALL_CHUNK_SIZE * CHUNK_PER_ZONE, zone2->size);
}
