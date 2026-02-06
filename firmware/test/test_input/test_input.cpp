#include <unity.h>
#include "Arduino.h" // Mock
#include "WiFi.h"    // Mock
#include "InputHandler.h"

// Define mock globals
unsigned long _mock_millis = 0;
int _mock_digitalRead_val = HIGH;
SerialMock Serial;
SDClass SD;
WiFiClass WiFi;

void setUp(void) {
    _mock_millis = 0;
    _mock_digitalRead_val = HIGH;
}

void tearDown(void) {
}

void test_click(void) {
    DisplayManager display;
    InputHandler input(9, display);
    SystemState state;
    Page page = PAGE_IDENTITY;
    unsigned long lastPageChange = 0;
    bool needsStaticDraw = false;

    input.begin();

    // 1. Press (LOW)
    _mock_digitalRead_val = LOW;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    // 2. Debounce (60ms) -> Stable LOW
    _mock_millis += 60;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    // 3. Release (HIGH)
    _mock_digitalRead_val = HIGH;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");

    // 4. Debounce (60ms) -> Stable HIGH -> Pending Click
    _mock_millis += 60;
    needsStaticDraw = false; // Reset before check
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    TEST_ASSERT_FALSE(needsStaticDraw); // Click pending, not confirmed yet
    
    // 5. Timeout (400ms) -> CLICK
    _mock_millis += 400;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");

    TEST_ASSERT_TRUE(needsStaticDraw);
    TEST_ASSERT_EQUAL(PAGE_RESOURCES, page);
}

void test_hold_reset(void) {
    DisplayManager display;
    InputHandler input(9, display);
    SystemState state;
    Page page = PAGE_IDENTITY;
    unsigned long lastPageChange = 0;
    bool needsStaticDraw = false;

    input.begin();

    // 1. Press (LOW)
    _mock_digitalRead_val = LOW;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");

    // 2. Debounce (60ms) -> Stable LOW. Press start time set here.
    _mock_millis += 60;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");

    // 3. Advance 3s (Total hold ~3s)
    // Note: at 800ms, HOLD triggers and toggles screen.
    // We assume screen starts ON. So it turns OFF.
    // Reset screen logic requires screen ON.
    // So we might miss the reset screen if we don't handle the toggle.
    // Let's force screen ON for this test or handle the toggle.
    
    _mock_millis += 3000;
    bool reset = input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    TEST_ASSERT_FALSE(reset);

    // 4. Advance to 11s total hold
    _mock_millis += 8000; 
    reset = input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    TEST_ASSERT_TRUE(reset);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_click);
    RUN_TEST(test_hold_reset);
    return UNITY_END();
}
