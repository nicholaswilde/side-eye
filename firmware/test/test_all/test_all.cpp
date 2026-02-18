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
#include "../../src/BLEPresenceManager.cpp"
#endif

#include "HistoryBuffer.h"
#include "InputHandler.h"
#include "DisplayManager.h"
#include "SyncManager.h"
#include "NetworkManager.h"

DisplayManager* DisplayManager::_instance = nullptr;

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
    
    display.begin(state);
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
    
    display.begin(state);
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
    
    display.begin(state);
    display.drawSDPage(state, true);
    display.drawSDPage(state, false);
    
    display.drawStaticUI(state, PAGE_SD, "1.0.0");
    display.updateDynamicValues(state, PAGE_SD, true, false, "1.0.0");
}

void test_display_draw_update() {
    DisplayManager display;
    display.drawUpdateScreen(50, "Downloading...");
}

void test_display_manager_extended() {
    DisplayManager display;
    SystemState state;
    state.connected = true;
    
    display.begin(state);
    
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

void test_display_draw_jpg() {
#ifdef NATIVE
    DisplayManager display;
    SystemState state;
    display.begin(state);
    
    // File doesn't exist
    TEST_ASSERT_FALSE(display.drawJpg("/test.jpg", 0, 0));
    
    // File exists
    _mock_sd_files["/test.jpg"] = "fake-jpg-data";
    TEST_ASSERT_TRUE(display.drawJpg("/test.jpg", 0, 0));
#endif
}

void test_display_background_fallback() {
#ifdef NATIVE
    DisplayManager display;
    SystemState state;
    display.begin(state);
    
    // Background doesn't exist - draw count should not increase (beyond initial if any)
    _mock_tjpg_draw_count = 0;
    display.drawStaticUI(state, PAGE_IDENTITY, "1.0.0");
    TEST_ASSERT_EQUAL(0, _mock_tjpg_draw_count);
    
    // Background exists
    _mock_sd_files["/themes/active/identity.jpg"] = "fake-jpg";
    display.drawStaticUI(state, PAGE_IDENTITY, "1.0.0");
    TEST_ASSERT_EQUAL(1, _mock_tjpg_draw_count);
#endif
}

void test_display_theme_json_parser() {
#ifdef NATIVE
    DisplayManager display;
    
    // Theme JSON doesn't exist - should stay default
    TEST_ASSERT_FALSE(display.loadTheme("/no-theme"));
    
    // Theme JSON exists
    _mock_sd_files["/themes/test/theme.json"] = "{\"colors\":{\"base\":\"0xFF00\",\"text\":\"0x00FF\"}}";
    TEST_ASSERT_TRUE(display.loadTheme("/themes/test"));
    TEST_ASSERT_EQUAL(0xFF00, display.active_theme.base);
    TEST_ASSERT_EQUAL(0x00FF, display.active_theme.text);
#endif
}

void test_theme_switching() {
#ifdef NATIVE
    DisplayManager display;
    SystemState state;
    SideEyeNetworkManager nm;
    
    // Initial theme
    state.theme_path = "/themes/old";
    _mock_sd_files["/themes/old/theme.json"] = "{\"colors\":{\"base\":\"0x1111\"}}";
    display.loadTheme(state.theme_path);
    TEST_ASSERT_EQUAL(0x1111, display.active_theme.base);
    
    // Update theme path
    state.theme_path = "/themes/new";
    _mock_sd_files["/themes/new/theme.json"] = "{\"colors\":{\"base\":\"0x2222\"}}";
    display.loadTheme(state.theme_path);
    TEST_ASSERT_EQUAL(0x2222, display.active_theme.base);
    
    // Verify save/load from config
    nm.saveConfig(state, true);
    TEST_ASSERT_TRUE(LittleFS.exists("/config.json"));
    
    SystemState state2;
    // nm.begin would load it, but we can just check if theme_path is in the JSON
    File f = LittleFS.open("/config.json", "r");
    String content = "";
    while(f.available()) content += (char)f.read();
    f.close();
    TEST_ASSERT_NOT_EQUAL(std::string::npos, content.find("/themes/new"));
#endif
}

void test_display_boot_screen_custom() {
#ifdef NATIVE
    DisplayManager display;
    
    // Default boot screen (no /boot.jpg)
    _mock_tjpg_draw_count = 0;
    display.drawBootScreen("1.0.0");
    TEST_ASSERT_EQUAL(0, _mock_tjpg_draw_count);
    
    // Custom boot screen (/boot.jpg exists)
    _mock_sd_files["/boot.jpg"] = "fake-boot-jpg";
    display.drawBootScreen("1.0.0");
    TEST_ASSERT_EQUAL(1, _mock_tjpg_draw_count);
#endif
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

void test_sync_manager_single_file() {
    SyncManager sync;
    sync.begin();
    
    JsonDocument doc;
    doc["path"] = "/single.txt";
    doc["offset"] = 0;
    doc["data"] = "SGVsbG8="; // "Hello"
    
    bool success = sync.handleWriteChunk(doc.as<JsonObject>());
    TEST_ASSERT_TRUE(success);
    
    // Verify content (Mock SD required to support this)
    TEST_ASSERT_TRUE(SD.exists("/single.txt"));
    File f = SD.open("/single.txt");
    TEST_ASSERT_TRUE(f);
    TEST_ASSERT_EQUAL(5, f.size());
    // We would read content here if mock supported it
    f.close();
}

void test_sync_manager_multi_chunk() {
    SyncManager sync;
    sync.begin();
    
    // Chunk 1: "He"
    JsonDocument doc1;
    doc1["path"] = "/multi.txt";
    doc1["offset"] = 0;
    doc1["data"] = "SGU="; // "He"
    TEST_ASSERT_TRUE(sync.handleWriteChunk(doc1.as<JsonObject>()));
    
    // Chunk 2: "llo"
    JsonDocument doc2;
    doc2["path"] = "/multi.txt";
    doc2["offset"] = 2;
    doc2["data"] = "bGxv"; // "llo"
    TEST_ASSERT_TRUE(sync.handleWriteChunk(doc2.as<JsonObject>()));
    
    TEST_ASSERT_TRUE(SD.exists("/multi.txt"));
    File f = SD.open("/multi.txt");
    TEST_ASSERT_EQUAL(5, f.size());
    f.close();
}

void test_sync_manager_nested_dir() {
    SyncManager sync;
    sync.begin();
    
    JsonDocument doc;
    doc["path"] = "/nested/dir/test.txt";
    doc["offset"] = 0;
    doc["data"] = "SGVsbG8="; // "Hello"
    
    bool success = sync.handleWriteChunk(doc.as<JsonObject>());
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(SD.exists("/nested/dir/test.txt"));
}

void dummy_callback() {}
void dummy_config_callback(WiFiManager* wm) {}

class NetworkManagerTest {
public:
    static void test() {
        SideEyeNetworkManager nm;
        BLEPresenceManager ble;
        SystemState state;
        unsigned long retry = 0;
        
        // 1. Initial state (disconnected)
        nm.update(retry); 
        nm.publishState(state, ble);
        
        // 2. Setup with config
        LittleFS._setFile("/config.json", "{\"mqtt_server\":\"localhost\",\"mqtt_port\":1883,\"mqtt_user\":\"user\",\"mqtt_pass\":\"pass\"}");
        nm.begin("DEV1", "1.0.0", state, dummy_callback, dummy_config_callback, dummy_callback);
        
        // 3. Trigger reconnect
        nm._mqttClient._setConnected(false);
        nm.update(retry);
        
        // 4. Publish while connected
        nm._mqttClient._setConnected(true);
        nm.publishState(state, ble);
        
        // 5. Discovery
        nm.publishHADiscovery();
        
        // 6. Setup without config
        SideEyeNetworkManager nm2;
        nm2.begin("DEV2", "1.0.0", state, dummy_callback, dummy_config_callback, dummy_callback);
        nm2._mqttClient._setConnected(false);
        nm2.reconnectMQTT(); // Test anonymous MQTT connect
        
        // 7. Save config paths
        nm.saveConfig(state, true);
        LittleFS._setFailNextOpen(true);
        nm.saveConfig(state, true);

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
    SystemState state;
    delay(2000); // Wait for serial to connect
    UNITY_BEGIN();
    RUN_TEST(test_history_push_and_get);
    RUN_TEST(test_history_max);
    RUN_TEST(test_display_draw_identity);
    RUN_TEST(test_display_format_speed);
    RUN_TEST(test_display_draw_smoke);
    RUN_TEST(test_display_sd_disconnected);
    RUN_TEST(test_display_draw_update);
    RUN_TEST(test_display_manager_extended);
    UNITY_END();
}

void loop() {
}
#else
void test_display_backlight_pwm() {
#ifdef NATIVE
    DisplayManager display;
    SystemState state;
    _mock_analogWrite_val = -1;
    display.setBacklight(state, true);
    TEST_ASSERT_EQUAL(255, _mock_analogWrite_val);
    display.setBacklight(state, false);
    TEST_ASSERT_EQUAL(0, _mock_analogWrite_val);
    
    // Test fade
    display.fadeBacklight(255, 100);
    TEST_ASSERT_EQUAL(255, _mock_analogWrite_val);
    display.fadeBacklight(0, 100);
    TEST_ASSERT_EQUAL(0, _mock_analogWrite_val);
#endif
}

void test_sync_manager_frequency() {
#ifdef NATIVE
    SyncManager sync;
    _mock_sd_frequency = 0;
    sync.begin();
    TEST_ASSERT_EQUAL(4000000, _mock_sd_frequency);
#endif
}

void test_ble_presence_logic() {
#ifdef NATIVE
    SideEyeNetworkManager network;
    SystemState state;
    BLEPresenceManager ble;
    ble.begin("TEST");
    ble.setEnabled(true);
    ble.setTargetMac("AA:BB:CC:DD:EE:FF");

    // 1. Initial state
    TEST_ASSERT_FALSE(ble.isPresent());
    TEST_ASSERT_EQUAL_STRING("Scanning", ble.getStatusString());

    // 2. Detect target
    BLEAdvertisedDevice dev; // Mock returns AA:BB:CC:DD:EE:FF by default
    ble.onResult(dev);
    TEST_ASSERT_TRUE(ble.isPresent());
    TEST_ASSERT_EQUAL_STRING("Present", ble.getStatusString());

    // 3. Signal drop (within timeout)
    _mock_millis += 5000;
    ble.update(network, state);
    TEST_ASSERT_TRUE(ble.isPresent());

    // 4. Timeout (10s)
    _mock_millis += 6000;
    ble.update(network, state);
    TEST_ASSERT_FALSE(ble.isPresent());
    TEST_ASSERT_EQUAL_STRING("Scanning", ble.getStatusString());

    // 5. Wrong target (ignored)
    // Note: Our current mock ALWAYS returns AA:BB:CC:DD:EE:FF. 
    // To test filtering properly we'd need a more advanced mock, but 
    // the logic in BLEPresenceManager::onResult is straightforward.
#endif
}

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
    RUN_TEST(test_display_draw_update);
    RUN_TEST(test_display_draw_jpg);
    RUN_TEST(test_display_background_fallback);
    RUN_TEST(test_display_theme_json_parser);
    RUN_TEST(test_theme_switching);
    RUN_TEST(test_display_boot_screen_custom);
    RUN_TEST(test_display_backlight_pwm);
    RUN_TEST(test_sync_manager_full);
    RUN_TEST(test_sync_manager_single_file);
    RUN_TEST(test_sync_manager_multi_chunk);
    RUN_TEST(test_sync_manager_nested_dir);
    RUN_TEST(test_sync_manager_frequency);
    RUN_TEST(test_network_manager_full);
    RUN_TEST(test_input_handler_extended);
    RUN_TEST(test_display_manager_extended);
    RUN_TEST(test_ble_presence_logic);
    UNITY_END();
    return 0;
}
#endif