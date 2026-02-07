#include <unity.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <LittleFS.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <esp_mac.h>
#include "../../test/mocks/mocks.cpp"
#include "HistoryBuffer.h"
#include "InputHandler.h"
#include "DisplayManager.h"
#include "SyncManager.h"
#include "NetworkManager.h"

void setUp(void) {
    _mock_millis = 0;
    _mock_digitalRead_val = HIGH;
}

void tearDown(void) {
}

// History tests
void test_history_push_and_get(void) {
    HistoryBuffer<int, 5> buffer;
    TEST_ASSERT_EQUAL(0, buffer.count());
    buffer.push(10);
    TEST_ASSERT_EQUAL(1, buffer.count());
    TEST_ASSERT_EQUAL(10, buffer.get(0));
}

void test_history_max(void) {
    HistoryBuffer<int, 5> buffer;
    buffer.push(10);
    buffer.push(50);
    buffer.push(20);
    TEST_ASSERT_EQUAL(50, buffer.max());
}

// Input tests
void test_input_click(void) {
    DisplayManager display;
    InputHandler input(9, display);
    SystemState state;
    Page page = PAGE_IDENTITY;
    unsigned long lastPageChange = 0;
    bool needsStaticDraw = false;

    input.begin();
    _mock_digitalRead_val = LOW;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_millis += 60;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_digitalRead_val = HIGH;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_millis += 60;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_millis += 400;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");

    TEST_ASSERT_TRUE(needsStaticDraw);
    TEST_ASSERT_EQUAL(PAGE_RESOURCES, page);
}

void test_input_double_click_and_hold(void) {
    DisplayManager display;
    InputHandler input(9, display);
    SystemState state;
    Page page = PAGE_IDENTITY;
    unsigned long lastPageChange = 0;
    bool needsStaticDraw = false;

    input.begin();
    
    // 1. Click 1
    _mock_digitalRead_val = LOW; _mock_millis += 60; input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_digitalRead_val = HIGH; _mock_millis += 60; input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    // 2. Click 2 (within double click window)
    _mock_digitalRead_val = LOW; _mock_millis += 60; input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    // Release
    _mock_digitalRead_val = HIGH; _mock_millis += 60; input.update(state, page, lastPageChange, needsStaticDraw, "v1");

    // Hold (for screen toggle)
    _mock_digitalRead_val = LOW; _mock_millis += 60; input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_millis += 1000;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    _mock_digitalRead_val = HIGH; _mock_millis += 60; input.update(state, page, lastPageChange, needsStaticDraw, "v1");
    
    // Auto-off simulation
    input.notifyActivity();
    _mock_millis += 70000;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1");
}

void test_input_notify_activity(void) {
    DisplayManager display;
    InputHandler input(9, display);
    input.notifyActivity();
    TEST_ASSERT_TRUE(input.isScreenOn());
}

void test_display_draw_identity() {
    DisplayManager display;
    SystemState state;
    state.hostname = "test-host";
    state.ip = "1.2.3.4";
    state.mac = "AA:BB:CC:DD:EE:FF";
    
    display.begin();
    display.drawIdentityPage(state, true);
    display.drawIdentityPage(state, false);
}

// Display tests
void test_display_format_speed() {
    DisplayManager display;
    TEST_ASSERT_EQUAL_STRING("500 B/s", display.formatSpeed(500).c_str());
    TEST_ASSERT_EQUAL_STRING("1.0 KB/s", display.formatSpeed(1024).c_str());
    TEST_ASSERT_EQUAL_STRING("1.5 MB/s", display.formatSpeed(1.5 * 1024 * 1024).c_str());
}

void test_display_draw_smoke() {
    DisplayManager display;
    SystemState state;
    state.connected = true;
    state.alert_level = 2; // Critical
    state.uptime = 3661;
    
    display.begin();
    display.drawBanner("Test", 2);
    display.drawWiFiStatus();
    display.drawProgressBar(0, 0, 100, 10, 50.0, 0xFFFF);
    
    HistoryBuffer<uint64_t, 60> buffer;
    buffer.push(10);
    display.drawSparkline(0, 0, 100, 20, buffer, 0xFFFF);
    
    display.drawResourcesPage(state, true);
    display.drawResourcesPage(state, false);
    display.drawStatusPage(state, true);
    display.drawStatusPage(state, false);
    display.drawSDPage(state, true);
    display.drawSDPage(state, false);
    display.drawThermalPage(state, true);
    display.drawThermalPage(state, false);
    display.drawNetworkPage(state, true);
    display.drawNetworkPage(state, false);
    
    display.drawStaticUI(state, PAGE_IDENTITY, "1.0.0");
    display.updateDynamicValues(state, PAGE_RESOURCES, true, false, "1.0.0");
}

void test_sync_manager_smoke() {
    SyncManager sync;
    sync.begin();
    sync.listFiles("/");
    
    JsonDocument doc;
    doc["path"] = "test.txt";
    doc["offset"] = 0;
    doc["data"] = "SGVsbG8="; // "Hello"
    sync.handleWriteChunk(doc.as<JsonObject>());
}

void test_network_manager_smoke() {
    SideEyeNetworkManager network;
    SystemState state;
    unsigned long retry = 0;
    
    network.saveConfig(false);
    network.update(retry);
    network.publishState(state);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_history_push_and_get);
    RUN_TEST(test_history_max);
    RUN_TEST(test_input_click);
    RUN_TEST(test_input_double_click_and_hold);
    RUN_TEST(test_input_notify_activity);
    RUN_TEST(test_display_draw_identity);
    RUN_TEST(test_display_format_speed);
    RUN_TEST(test_display_draw_smoke);
    RUN_TEST(test_sync_manager_smoke);
    RUN_TEST(test_network_manager_smoke);
    UNITY_END();
    return 0;
}
