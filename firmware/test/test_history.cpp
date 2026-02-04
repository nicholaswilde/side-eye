#include <unity.h>
#include "HistoryBuffer.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_push_and_get(void) {
    HistoryBuffer<int, 5> buffer;
    TEST_ASSERT_EQUAL(0, buffer.count());

    buffer.push(10);
    TEST_ASSERT_EQUAL(1, buffer.count());
    TEST_ASSERT_EQUAL(10, buffer.get(0));

    buffer.push(20);
    buffer.push(30);
    buffer.push(40);
    buffer.push(50);
    TEST_ASSERT_EQUAL(5, buffer.count());
    TEST_ASSERT_EQUAL(10, buffer.get(0));
    TEST_ASSERT_EQUAL(50, buffer.get(4));

    // Test wrap around
    buffer.push(60);
    TEST_ASSERT_EQUAL(5, buffer.count());
    TEST_ASSERT_EQUAL(20, buffer.get(0));
    TEST_ASSERT_EQUAL(60, buffer.get(4));
}

void test_max(void) {
    HistoryBuffer<int, 5> buffer;
    buffer.push(10);
    buffer.push(50);
    buffer.push(20);
    TEST_ASSERT_EQUAL(50, buffer.max());

    buffer.push(100);
    TEST_ASSERT_EQUAL(100, buffer.max());

    buffer.push(5);
    buffer.push(5); // 10 should drop out
    TEST_ASSERT_EQUAL(100, buffer.max());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_push_and_get);
    RUN_TEST(test_max);
    return UNITY_END();
}
