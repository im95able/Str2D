#define SEG_TEST

#ifdef SEG_TEST

#define GTEST_LANG_CXX11 1 // Must be defined, otherwise Gtest complains

#define SEG_CONTAINER_FLAT_INSERT_TEST
#define SEG_CONTAINER_FLAT_ERASE_TEST
#define SEG_CONTAINER_SEGMENT_SLIDE_TEST
#define SEG_CONTAINER_SEGMENT_MOVE_TEST
#define SEG_CONTAINER_INDEX_TEST
#define SEG_CONTAINER_SEGMENTED_INSERT_TEST
#define SEG_CONTAINER_SEGMENTED_ERASE_TEST

#endif

#define BIG_HEADER

#define CHUNKED_ALLOCATOR_TEST

//#define SEG_POD_TEST