#include <unity.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <LittleFS.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <esp_mac.h>

#ifdef NATIVE
#include "../../test/mocks/mocks.cpp"
#endif

#include "HistoryBuffer.h"
#include "InputHandler.h"
#include "DisplayManager.h"
#include "SyncManager.h"
#include "NetworkManager.h"

void setUp(void) {
#ifdef NATIVE
    _mock_millis = 0;
    _mock_digitalRead_val = HIGH;
#endif
}

void tearDown(void) {
}

// --- Shared Tests (Hardware and Native) ---

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

void test_display_sd_disconnected() {
    DisplayManager display;
    SystemState state;
    state.connected = false;
    
    display.begin();
    display.drawSDPage(state, true);
    display.drawSDPage(state, false);
    
    display.drawStaticUI(state, PAGE_SD, "1.0.0");
    display.updateDynamicValues(state, PAGE_SD, true, false, "1.0.0");
}

void test_display_manager_extended() {
    DisplayManager display;
    SystemState state;
    state.connected = true;
    
    display.begin();
    
    // Test all alert levels in banner
    display.drawBanner("Alert 0", 0);
#ifdef NATIVE
    _mock_millis += 1000;
#else
    delay(1000);
#endif
    display.drawBanner("Alert 1", 1);
    display.drawBanner("Alert 2", 2);
    
    // Test various states
    state.cpu_percent = 90; // Red progress bar
    display.drawResourcesPage(state, false);
    state.cpu_percent = 60; // Yellow progress bar
    display.drawResourcesPage(state, false);
    
    // Test Sparkline with 0 max
    HistoryBuffer<uint64_t, 60> emptyBuffer;
    display.drawSparkline(0, 0, 100, 20, emptyBuffer, 0xFFFF);
    emptyBuffer.push(0);
    emptyBuffer.push(0);
    display.drawSparkline(0, 0, 100, 20, emptyBuffer, 0xFFFF);
    
    // Test other screens
    display.drawBootScreen("1.0.0");
    display.drawConfigMode("AP", "1.2.3.4");
    display.drawWiFiOnline();
    display.drawResetScreen(5, true);
    display.drawResetScreen(4, false);
    
    // Test updateDynamicValues with disconnected state
    state.connected = false;
    display.updateDynamicValues(state, PAGE_IDENTITY, true, false, "1.0.0");
}

// --- Native-Only Tests (Require Mocks) ---

#ifdef NATIVE
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

void test_sync_manager_full() {
    SyncManager sync;
    sync.begin();
    
    // Test listFiles with no files (mock returns empty list)
    String list = sync.listFiles("/");
    // Mock SD.open returns a valid file by default now, so it should find one file named "test"
    TEST_ASSERT_EQUAL_STRING("[{\"n\":\"test\",\"s\":4,\"d\":false}]", list.c_str());
    
    // Test listFiles with invalid dir
    TEST_ASSERT_EQUAL_STRING("[]", sync.listFiles("invalid").c_str());
    
    // Test handleWriteChunk
    JsonDocument doc;
    doc["path"] = "test.txt";
    doc["offset"] = 0;
    doc["data"] = "SGVsbG8="; // "Hello"
    bool success = sync.handleWriteChunk(doc.as<JsonObject>());
    TEST_ASSERT_TRUE(success);
}

void dummy_callback() {}
void dummy_config_callback(WiFiManager* wm) {}

class NetworkManagerTest {
public:
    static void test() {
        SideEyeNetworkManager nm;
        SystemState state;
        unsigned long retry = 0;
        
        // 1. Initial state (disconnected)
        nm.update(retry); 
        nm.publishState(state);
        
        // 2. Setup with config
        LittleFS._setFile("/config.json", "{\"mqtt_server\":\"localhost\",\"mqtt_port\":1883,\"mqtt_user\":\"user\",\"mqtt_pass\":\"pass\"}");
        nm.begin("DEV1", "1.0.0", dummy_callback, dummy_config_callback, dummy_callback);
        
        // 3. Trigger reconnect
        nm._mqttClient._setConnected(false);
        nm.update(retry);
        
        // 4. Publish while connected
        nm._mqttClient._setConnected(true);
        nm.publishState(state);
        
        // 5. Discovery
        nm.publishHADiscovery();
        
        // 6. Setup without config
        SideEyeNetworkManager nm2;
        nm2.begin("DEV2", "1.0.0", dummy_callback, dummy_config_callback, dummy_callback);
        nm2._mqttClient._setConnected(false);
        nm2.reconnectMQTT(); // Test anonymous MQTT connect
        
        // 7. Save config paths
        nm.saveConfig(true);
        LittleFS._setFailNextOpen(true);
        nm.saveConfig(true);

        // 8. Reset settings
        ESP._restarted = false;
        nm.resetSettings();
        TEST_ASSERT_TRUE(ESP._restarted);
    }
    
    static void testFailure() {
        SideEyeNetworkManager nm;
        nm._mqttClient._setConnected(false);
        nm._mqttClient._setState(-1);
        nm.reconnectMQTT();
    }
};

void test_network_manager_full() {
    NetworkManagerTest::test();
    NetworkManagerTest::testFailure();
}

void test_input_handler_extended() {
    SystemState state;
    Page page = PAGE_IDENTITY;
    unsigned long lastPageChange = 0;
    bool needsStaticDraw = false;

    _mock_digitalRead_val = HIGH;
    _mock_millis = 0;
    DisplayManager display;
    InputHandler input(9, display);
    input.begin();
    input.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register HIGH
    
    // Test Hold (toggle screen)
    _mock_digitalRead_val = LOW;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Start press
    _mock_millis += 100;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register press
    _mock_millis += 1000;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register HOLD
    _mock_digitalRead_val = HIGH;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Release
    _mock_millis += 100;
    input.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register Release
    TEST_ASSERT_FALSE(input.isScreenOn());
    
    // Test Long Hold (Reset)
    _mock_digitalRead_val = HIGH;
    _mock_millis += 100;
    InputHandler input2(9, display);
    input2.begin();
    input2.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register HIGH
    
    _mock_digitalRead_val = LOW;
    input2.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Start press
    _mock_millis += 100;
    input2.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register press
    
    bool reset = false;
    for(int i=0; i<30; i++) {
        _mock_millis += 500;
        if (input2.update(state, page, lastPageChange, needsStaticDraw, "v1")) {
            reset = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(reset);
    TEST_ASSERT_TRUE(input2.isResetActive());
    _mock_digitalRead_val = HIGH;
    input2.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Release
    _mock_millis += 100;
    input2.update(state, page, lastPageChange, needsStaticDraw, "v1"); // Register Release
    TEST_ASSERT_FALSE(input2.isResetActive());
}

void test_input_click_disconnected(void) {
    DisplayManager display;
    InputHandler input(9, display);
    SystemState state;
    state.connected = false;
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
#endif

// --- Test Runners ---

#ifndef NATIVE
void setup() {
    delay(2000); // Wait for serial to connect
    UNITY_BEGIN();
    RUN_TEST(test_history_push_and_get);
    RUN_TEST(test_history_max);
    RUN_TEST(test_display_draw_identity);
    RUN_TEST(test_display_format_speed);
    RUN_TEST(test_display_draw_smoke);
    RUN_TEST(test_display_sd_disconnected);
    RUN_TEST(test_display_manager_extended);
    UNITY_END();
}

void loop() {
}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_history_push_and_get);
    RUN_TEST(test_history_max);
    RUN_TEST(test_input_click);
    RUN_TEST(test_input_click_disconnected);
    RUN_TEST(test_input_double_click_and_hold);
    RUN_TEST(test_input_notify_activity);
    RUN_TEST(test_display_draw_identity);
    RUN_TEST(test_display_format_speed);
    RUN_TEST(test_display_draw_smoke);
    RUN_TEST(test_display_sd_disconnected);
    RUN_TEST(test_sync_manager_full);
    RUN_TEST(test_network_manager_full);
    RUN_TEST(test_input_handler_extended);
    RUN_TEST(test_display_manager_extended);
    UNITY_END();
    return 0;
}
#endif