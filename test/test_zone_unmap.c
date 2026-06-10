#include "chunk.h"
#include "unity.h"

#include "zone.h"
#include "chunk.h"

void setUp(void) {}
void tearDown(void) {}

void test_zone_unmap_tiny(void);
void test_zone_unmap_small(void);

int main(void) {
    UNITY_BEGIN();

    //Basic tests, we try to malloc and free twice and see if everything is good
    RUN_TEST(test_zone_unmap_tiny);
    RUN_TEST(test_zone_unmap_small);

    return UNITY_END();
}

void test_zone_unmap_tiny(void) {
    zone_t zone_1 = zone_new(NULL, TINY_CHUNK_SIZE);
    zone_t zone_2 = zone_new(zone_1, TINY_CHUNK_SIZE);
    zone_t zone_3 = zone_new(zone_2, TINY_CHUNK_SIZE);

    chunk_t chunk = (chunk_t)zone_1->data;
    chunk->free = 0;
    TEST_ASSERT_EQUAL(zone_3, zone_2->next);
    zone_unmap(&zone_1);
    TEST_ASSERT_NULL(zone_2->next);

    zone_3 = zone_new(zone_2, TINY_CHUNK_SIZE);
    chunk->free = 1;
    chunk = (chunk_t)zone_2->data;
    chunk->free = 0;
    TEST_ASSERT_EQUAL(zone_3, zone_2->next);
    zone_unmap(&zone_1);
    TEST_ASSERT_NULL(zone_2->next);
}

void test_zone_unmap_small(void) {
    zone_t zone_1 = zone_new(NULL, SMALL_CHUNK_SIZE);
    zone_t zone_2 = zone_new(zone_1, SMALL_CHUNK_SIZE);
    zone_t zone_3 = zone_new(zone_2, SMALL_CHUNK_SIZE);

    chunk_t chunk = (chunk_t)zone_1->data;
    chunk->free = 0;
    TEST_ASSERT_EQUAL(zone_3, zone_2->next);
    zone_unmap(&zone_1);
    TEST_ASSERT_NULL(zone_2->next);

    zone_3 = zone_new(zone_2, SMALL_CHUNK_SIZE);
    chunk->free = 1;
    chunk = (chunk_t)zone_2->data;
    chunk->free = 0;
    TEST_ASSERT_EQUAL(zone_3, zone_2->next);
    zone_unmap(&zone_1);
    TEST_ASSERT_NULL(zone_2->next);
}
