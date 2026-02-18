#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <TJpg_Decoder.h>
#include <ArduinoJson.h>
#include "catppuccin_colors.h"
#include "HistoryBuffer.h"
#include "status_icons.h"
#include <SD.h>

/* 
 * Waveshare ESP32-C6-GEEK Configuration
 */
#define LCD_SCK 1
#define LCD_MOSI 2
#define LCD_MISO 0
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

enum Page {
    PAGE_IDENTITY,
    PAGE_RESOURCES,
    PAGE_STATUS,
    PAGE_SD,
    PAGE_THERMAL,
    PAGE_NETWORK,
    NUM_PAGES
};

struct SystemState {
    String hostname = "Unknown";
    String ip = "No IP";
    String mac = "No MAC";
    String os = "Unknown";
    String user = "Unknown";
    float cpu_percent = 0;
    uint64_t ram_used = 0;
    uint64_t ram_total = 0;
    uint64_t disk_used = 0;
    uint64_t disk_total = 0;
    uint64_t net_up = 0;
    uint64_t net_down = 0;
    HistoryBuffer<uint64_t, 60> net_up_history;
    HistoryBuffer<uint64_t, 60> net_down_history;
    uint64_t uptime = 0;
    uint64_t sd_used = 0;
    uint64_t sd_total = 0;
    String sd_sync_status = "Idle";
    float thermal_c = 0;
    float gpu_percent = 0;
    uint8_t alert_level = 0;
    bool has_data = false;
    bool connected = false;
    bool mqtt_connected = false;
    String ble_status = "Disabled";
    bool ble_present = false;
    bool post_update_check = false;

    // Configurable Settings
    bool ble_enabled = false;
    String ble_target = "";
    uint8_t brightness = 255;
    int rotation = 1;
    unsigned long cycle_duration = 5000;
    uint8_t cpu_warning = 50;
    uint8_t cpu_critical = 80;
    uint8_t ram_warning = 50;
    uint8_t ram_critical = 80;
    String theme_path = "/themes/active";
};

class DisplayManager {
public:
    static bool tjpg_callback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
        if (_instance) {
            _instance->gfx.draw16bitRGBBitmap(x, y, bitmap, w, h);
        }
        return true;
    }

    DisplayManager() : 
        bus(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, LCD_MISO),
        gfx(&bus, LCD_RST, 0 /* rotation */, true /* IPS */,
            135 /* width */, 240 /* height */,
            52 /* col offset 1 */, 40 /* row offset 1 */,
            53 /* col offset 2 */, 40 /* row offset 2 */)
    {
        _instance = this;
    }

    ~DisplayManager() {
        if (_instance == this) _instance = nullptr;
    }

    // Disable copy and assignment
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    void begin(const SystemState& state) {
        if (LCD_BL >= 0) {
            pinMode(LCD_BL, OUTPUT);
            setBacklight(state, true);
        }
        gfx.begin();
        gfx.setRotation(currentRotation);
        gfx.fillScreen(active_theme.base);

        TJpgDec.setCallback(tjpg_callback);
        TJpgDec.setJpgScale(1);
        TJpgDec.setSwapBytes(true);
    }

    bool drawJpg(const char* path, int x, int y) {
        if (!SD.exists(path)) return false;
        TJpgDec.drawSdJpg(x, y, path);
        return true;
    }

    void drawStatusIcon(int x, int y, const uint8_t* bitmap, uint16_t color) {
        gfx.drawBitmap(x, y, bitmap, 8, 8, color, active_theme.base);
    }

    bool loadTheme(const String& themePath) {
        String jsonPath = themePath + "/theme.json";
        if (!SD.exists(jsonPath.c_str())) {
            // Reset to defaults
            active_theme = ThemeColors();
            return false;
        }

        File file = SD.open(jsonPath.c_str());
        if (!file) return false;

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) return false;

        if (doc["colors"].is<JsonObject>()) {
            JsonObject colors = doc["colors"];
            if (colors["base"].is<const char*>()) active_theme.base = (uint16_t)strtol(colors["base"], nullptr, 16);
            if (colors["text"].is<const char*>()) active_theme.text = (uint16_t)strtol(colors["text"], nullptr, 16);
            if (colors["subtext"].is<const char*>()) active_theme.subtext = (uint16_t)strtol(colors["subtext"], nullptr, 16);
            if (colors["overlay"].is<const char*>()) active_theme.overlay = (uint16_t)strtol(colors["overlay"], nullptr, 16);
            if (colors["accent"].is<const char*>()) active_theme.accent = (uint16_t)strtol(colors["accent"], nullptr, 16);
            if (colors["green"].is<const char*>()) active_theme.green = (uint16_t)strtol(colors["green"], nullptr, 16);
            if (colors["yellow"].is<const char*>()) active_theme.yellow = (uint16_t)strtol(colors["yellow"], nullptr, 16);
            if (colors["red"].is<const char*>()) active_theme.red = (uint16_t)strtol(colors["red"], nullptr, 16);
            if (colors["blue"].is<const char*>()) active_theme.blue = (uint16_t)strtol(colors["blue"], nullptr, 16);
            if (colors["peach"].is<const char*>()) active_theme.peach = (uint16_t)strtol(colors["peach"], nullptr, 16);
            if (colors["sapphire"].is<const char*>()) active_theme.sapphire = (uint16_t)strtol(colors["sapphire"], nullptr, 16);
            if (colors["teal"].is<const char*>()) active_theme.teal = (uint16_t)strtol(colors["teal"], nullptr, 16);
            if (colors["flamingo"].is<const char*>()) active_theme.flamingo = (uint16_t)strtol(colors["flamingo"], nullptr, 16);
        }
        return true;
    }

    void setRotation(int rotation) {
        currentRotation = rotation;
        gfx.setRotation(currentRotation);
    }

    int getRotation() {
        return currentRotation;
    }

    void setBacklight(const SystemState& state, bool on) {
        analogWrite(LCD_BL, on ? state.brightness : 0);
    }

    void fadeBacklight(uint8_t target, uint16_t duration_ms) {
        static uint8_t current_brightness = 255;
        int steps = 20;
        int delay_step = duration_ms / steps;
        int delta = (target - current_brightness) / steps;

        for (int i = 0; i < steps; i++) {
            current_brightness += delta;
            analogWrite(LCD_BL, current_brightness);
            delay(delay_step);
        }
        current_brightness = target;
        analogWrite(LCD_BL, current_brightness);
    }

    void fillScreen(uint16_t color) {
        gfx.fillScreen(color);
    }

    void drawBanner(const char* title, uint8_t alert_level = 0) {
        uint16_t bg_color = active_theme.accent;
        if (alert_level == 1) {
            bg_color = active_theme.yellow;
        } else if (alert_level >= 2) {
            // Flash red/base if critical
            bg_color = ((millis() / 500) % 2 == 0) ? active_theme.red : active_theme.base;
        }

        gfx.fillRect(0, 0, 240, 20, bg_color);
        gfx.setTextColor(bg_color == active_theme.base ? active_theme.red : CATPPUCCIN_CRUST);
        gfx.setTextSize(1);
        
        int16_t x1, y1;
        uint16_t w, h;
        gfx.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((240 - w) / 2, 6);
        gfx.println(title);
    }

    void drawWiFiStatus() {
        int x = (currentRotation == 1) ? 225 : 15;
        int y = 10;
        if (WiFi.status() == WL_CONNECTED) {
            gfx.fillCircle(x, y, 3, active_theme.green);
        } else {
            gfx.fillCircle(x, y, 3, active_theme.red);
        }
    }

    void drawProgressBar(int x, int y, int w, int h, float percent, uint16_t color) {
        gfx.drawRect(x, y, w, h, active_theme.overlay);
        int fill_w = (int)((w - 2) * (percent / 100.0));
        if (fill_w < 0) fill_w = 0;
        if (fill_w > w - 2) fill_w = w - 2;
        gfx.fillRect(x + 1, y + 1, w - 2, h - 2, active_theme.base);
        gfx.fillRect(x + 1, y + 1, fill_w, h - 2, color);
    }

    template <typename T, size_t Size>
    void drawSparkline(int x, int y, int w, int h, const HistoryBuffer<T, Size>& buffer, uint16_t color) {
        gfx.drawRect(x, y, w, h, active_theme.overlay);
        gfx.fillRect(x + 1, y + 1, w - 2, h - 2, active_theme.base);

        size_t count = buffer.count();
        if (count < 2) return;

        T max_val = buffer.max();
        if (max_val == 0) max_val = 1; // Avoid division by zero

        int prev_x = -1;
        int prev_y = -1;

        for (size_t i = 0; i < count; i++) {
            int cur_x = x + 1 + (int)(i * (w - 2) / (Size - 1));
            T val = buffer.get(i);
            int cur_y = y + h - 1 - (int)(val * (h - 2) / max_val);

            if (prev_x != -1) {
                gfx.drawLine(prev_x, prev_y, cur_x, cur_y, color);
            }
            prev_x = cur_x;
            prev_y = cur_y;
        }
    }

    void drawIdentityPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setTextColor(active_theme.blue);
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.print("Host: ");

            gfx.setCursor(start_x, start_y + line_h * 2.5);
            gfx.setTextColor(active_theme.green);
            gfx.print("IP:   ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(active_theme.flamingo);
            gfx.print("MAC:  ");
        } else {
            gfx.setTextColor(active_theme.text);
            
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.println(state.hostname);

            gfx.fillRect(value_x, (int)(start_y + line_h * 2.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 2.5));
            gfx.println(state.ip);

            gfx.fillRect(value_x, (int)(start_y + line_h * 3.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 3.5));
            gfx.println(state.mac);
        }
    }

    void drawResourcesPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(active_theme.peach);
            gfx.print("CPU:  ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(active_theme.sapphire);
            gfx.print("RAM:  ");
        } else {
            gfx.setTextColor(active_theme.text);

            // CPU
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 100, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.print(state.cpu_percent, 1);
            gfx.println("%");
            uint16_t cpu_col = (state.cpu_percent > 80) ? active_theme.red : (state.cpu_percent > 50) ? active_theme.yellow : active_theme.green;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, state.cpu_percent, cpu_col);

            // RAM
            gfx.fillRect(value_x, (int)(start_y + line_h * 3.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 3.5));
            gfx.printf("%llu / %llu MB", state.ram_used / 1024 / 1024, state.ram_total / 1024 / 1024);
            float ram_p = (state.ram_total > 0) ? (float)state.ram_used / state.ram_total * 100.0 : 0;
            uint16_t ram_col = (ram_p > 80) ? active_theme.red : (ram_p > 50) ? active_theme.yellow : active_theme.green;
            drawProgressBar(start_x, start_y + line_h * 4.5, 220, 8, ram_p, ram_col);
        }
    }

    void drawStatusPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(active_theme.teal);
            gfx.print("Disk: ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(active_theme.subtext);
            gfx.print("Uptime: ");
        } else {
            gfx.setTextColor(active_theme.text);

            // Disk
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.printf("%llu / %llu GB", state.disk_used / 1024 / 1024 / 1024, state.disk_total / 1024 / 1024 / 1024);
            float disk_p = (state.disk_total > 0) ? (float)state.disk_used / state.disk_total * 100.0 : 0;
            uint16_t disk_col = (disk_p > 80) ? active_theme.red : (disk_p > 50) ? active_theme.yellow : active_theme.green;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, disk_p, disk_col);

            // Uptime
            gfx.fillRect(value_x + 20, (int)(start_y + line_h * 3.5), 160, 8, active_theme.base);
            uint32_t h_up = state.uptime / 3600;
            uint32_t m_up = (state.uptime % 3600) / 60;
            gfx.setCursor(value_x + 20, (int)(start_y + line_h * 3.5));
            gfx.printf("%luh %lum", (unsigned long)h_up, (unsigned long)m_up);
        }
    }

    void drawSDPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(active_theme.accent);
            gfx.print("SD Card:");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(active_theme.yellow);
            gfx.print("Sync:");
        } else {
            gfx.setTextColor(active_theme.text);

            // SD Storage
            uint64_t total = SD.totalBytes();
            uint64_t used = SD.usedBytes();
            gfx.fillRect(value_x + 20, (int)(start_y + line_h * 1.5), 160, 8, active_theme.base);
            gfx.setCursor(value_x + 20, (int)(start_y + line_h * 1.5));
            gfx.printf("%llu / %llu MB", used / 1024 / 1024, total / 1024 / 1024);
            // cppcheck-suppress knownConditionTrueFalse
            float sd_p = (total > 0) ? (float)used / total * 100.0 : 0;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, sd_p, active_theme.accent);

            // Sync Status
            gfx.fillRect(value_x + 10, (int)(start_y + line_h * 3.5), 170, 8, active_theme.base);
            gfx.setCursor(value_x + 10, (int)(start_y + line_h * 3.5));
            if (state.connected) {
                gfx.print(state.sd_sync_status);
            } else {
                gfx.print("Disconnected");
            }
        }
    }

    void drawThermalPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(active_theme.red);
            gfx.print("Temp: ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(active_theme.green);
            gfx.print("GPU:  ");
        } else {
            gfx.setTextColor(active_theme.text);

            // Thermal
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 100, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.print(state.thermal_c, 1);
            gfx.println(" C");
            uint16_t temp_col = (state.thermal_c > 80) ? active_theme.red : (state.thermal_c > 65) ? active_theme.yellow : active_theme.green;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, state.thermal_c, temp_col);

            // GPU
            gfx.fillRect(value_x, (int)(start_y + line_h * 3.5), 100, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 3.5));
            gfx.print(state.gpu_percent, 1);
            gfx.println("%");
            uint16_t gpu_col = (state.gpu_percent > 80) ? active_theme.red : (state.gpu_percent > 50) ? active_theme.yellow : active_theme.green;
            drawProgressBar(start_x, start_y + line_h * 4.5, 220, 8, state.gpu_percent, gpu_col);
        }
    }

    void drawNetworkPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(active_theme.green);
            gfx.print("Down:");

            gfx.setCursor(start_x, start_y + line_h * 4.5);
            gfx.setTextColor(active_theme.accent);
            gfx.print("Up:");
        } else {
            gfx.setTextColor(active_theme.text);

            // Download
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.print(formatSpeed(state.net_down));
            drawSparkline(start_x, start_y + line_h * 2.5, 220, 20, state.net_down_history, active_theme.green);

            // Upload
            gfx.fillRect(value_x, (int)(start_y + line_h * 4.5), 180, 8, active_theme.base);
            gfx.setCursor(value_x, (int)(start_y + line_h * 4.5));
            gfx.print(formatSpeed(state.net_up));
            drawSparkline(start_x, start_y + line_h * 5.5, 220, 20, state.net_up_history, active_theme.accent);
        }
    }

    String formatSpeed(uint64_t bytesPerSec) {
        if (bytesPerSec < 1024) return String(bytesPerSec) + " B/s";
        if (bytesPerSec < 1024 * 1024) return String(bytesPerSec / 1024.0, 1) + " KB/s";
        return String(bytesPerSec / (1024.0 * 1024.0), 1) + " MB/s";
    }

    void drawStaticUI(const SystemState& state, Page currentPage, const char* version) {
        // Try to draw background image from SD
        String path = state.theme_path + "/";
        switch (currentPage) {
            case PAGE_IDENTITY:  path += "identity.jpg"; break;
            case PAGE_RESOURCES: path += "resources.jpg"; break;
            case PAGE_STATUS:    path += "status.jpg"; break;
            case PAGE_SD:        path += "sd.jpg"; break;
            case PAGE_THERMAL:   path += "thermal.jpg"; break;
            case PAGE_NETWORK:   path += "network.jpg"; break;
            default:             path += "background.jpg"; break;
        }

        if (!drawJpg(path.c_str(), 0, 0)) {
            // Fallback to solid color if image fails or doesn't exist
            gfx.fillScreen(active_theme.base);
        }

        drawBanner("SIDEEYE MONITOR", state.alert_level);
        drawWiFiStatus();

        gfx.setTextSize(1);
        
        gfx.setTextColor(active_theme.yellow);
        gfx.setCursor(start_x, start_y);
        gfx.print("Status:");

        if (state.connected || currentPage == PAGE_SD) {
            switch (currentPage) {
                case PAGE_IDENTITY: if (state.connected) drawIdentityPage(state, true); break;
                case PAGE_RESOURCES: if (state.connected) drawResourcesPage(state, true); break;
                case PAGE_STATUS: if (state.connected) drawStatusPage(state, true); break;
                case PAGE_SD: drawSDPage(state, true); break;
                case PAGE_THERMAL: if (state.connected) drawThermalPage(state, true); break;
                case PAGE_NETWORK: if (state.connected) drawNetworkPage(state, true); break;
                default: break;
            }
        }

        // Version back in bottom right corner
        gfx.setTextColor(CATPPUCCIN_SURFACE1);
        gfx.setCursor(200, 120);
        gfx.print(version);
    }

    void updateDynamicValues(const SystemState& state, Page currentPage, bool forceRedraw, bool waitingMessageActive, const char* version) {
        if (forceRedraw || waitingMessageActive) {
            drawStaticUI(state, currentPage, version);
        }

        gfx.setTextSize(1);

        // Status value
        gfx.fillRect(value_x, start_y, 140, 8, active_theme.base);
        gfx.setCursor(value_x, start_y);
        if (state.connected) {
            gfx.setTextColor(active_theme.green);
            gfx.println("Connected");
        } else {
            gfx.setTextColor(active_theme.peach);
            gfx.println("Waiting...");
        }

        if (state.connected || currentPage == PAGE_SD) {
            switch (currentPage) {
                case PAGE_IDENTITY: if (state.connected) drawIdentityPage(state, false); break;
                case PAGE_RESOURCES: if (state.connected) drawResourcesPage(state, false); break;
                case PAGE_STATUS: if (state.connected) drawStatusPage(state, false); break;
                case PAGE_SD: drawSDPage(state, false); break;
                case PAGE_THERMAL: if (state.connected) drawThermalPage(state, false); break;
                case PAGE_NETWORK: if (state.connected) drawNetworkPage(state, false); break;
                default: break;
            }
        }
    }

    void drawBootScreen(const char* version) {
        if (drawJpg("/boot.jpg", 0, 0)) {
            // If boot.jpg exists and was drawn, we can still overlay the version if desired,
            // or just return. Let's overlay the version in a corner.
            gfx.setTextSize(1);
            gfx.setTextColor(active_theme.subtext);
            gfx.setCursor(200, 120);
            gfx.print(version);
            return;
        }

        gfx.fillScreen(active_theme.base);
        drawBanner("BOOTING...");
        
        int16_t x1, y1;
        uint16_t w, h;
        int screen_w = 240; // Landscape width
        
        // Draw SideEye name
        gfx.setTextSize(2);
        gfx.setTextColor(active_theme.accent);
        const char* name = "SideEye";
        gfx.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((screen_w - w) / 2, 55);
        gfx.println(name);
        
        // Draw Version
        gfx.setTextSize(1);
        gfx.setTextColor(active_theme.subtext);
        char v_str[32];
        snprintf(v_str, sizeof(v_str), "v%s", version);
        gfx.getTextBounds(v_str, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((screen_w - w) / 2, 85);
        gfx.println(v_str);
    }

    void drawConfigMode(const char* apName, const String& ip) {
        gfx.fillScreen(active_theme.base);
        drawBanner("SETUP MODE", 1);
        
        gfx.setTextColor(active_theme.text);
        gfx.setTextSize(1);
        gfx.setCursor(15, 45);
        gfx.println("Connect to WiFi AP:");
        
        gfx.setTextColor(active_theme.yellow);
        gfx.setCursor(15, 60);
        gfx.println(apName);
        
        gfx.setTextColor(active_theme.text);
        gfx.setCursor(15, 90);
        gfx.print("Then visit:");
        
        gfx.setTextColor(active_theme.green);
        gfx.setCursor(90, 90);
        gfx.println(ip);
    }

    void drawWiFiOnline() {
        gfx.fillScreen(active_theme.base);
        drawBanner("CONNECTED");
        gfx.setCursor(15, start_y);
        gfx.setTextColor(active_theme.green);
        gfx.println("WiFi Online!");
    }

    void showNotification(const char* message) {
        // Simple overlay notification
        uint16_t box_w = 200;
        uint16_t box_h = 40;
        uint16_t box_x = (240 - box_w) / 2;
        uint16_t box_y = (135 - box_h) / 2;

        gfx.fillRect(box_x, box_y, box_w, box_h, active_theme.overlay);
        gfx.drawRect(box_x, box_y, box_w, box_h, active_theme.accent);
        
        gfx.setTextColor(active_theme.text);
        gfx.setTextSize(1);
        
        int16_t x1, y1;
        uint16_t w, h;
        gfx.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor(box_x + (box_w - w) / 2, box_y + (box_h - h) / 2);
        gfx.println(message);
        
        delay(1500); // Hold for 1.5s
    }

    void drawResetScreen(int secondsRemaining, bool forceRedraw = false) {
        if (forceRedraw) {
            gfx.fillScreen(active_theme.base);
            drawBanner("FACTORY RESET", 0); // Use alert 0 to avoid banner flashing
            
            gfx.setTextColor(active_theme.text);
            gfx.setTextSize(1);
            gfx.setCursor(15, 50);
            gfx.println("Resetting in:");
            
            gfx.setTextColor(active_theme.subtext);
            gfx.setTextSize(1);
            gfx.setCursor(15, 115);
            gfx.println("Release to cancel");
        }
        
        // Clear and update only the number area
        gfx.fillRect(100, 75, 40, 25, active_theme.base);
        gfx.setTextColor(active_theme.red);
        gfx.setTextSize(3);
        gfx.setCursor(100, 75);
        gfx.println(secondsRemaining);
    }

    void drawUpdateScreen(int progress, const char* status) {
        gfx.fillScreen(active_theme.base);
        drawBanner("FIRMWARE UPDATE", 1);
        
        gfx.setTextColor(active_theme.text);
        gfx.setTextSize(1);
        
        int16_t x1, y1;
        uint16_t w, h;
        gfx.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((240 - w) / 2, 50);
        gfx.println(status);
        
        drawProgressBar(20, 80, 200, 20, progress, active_theme.green);
        
        char p_str[10];
        snprintf(p_str, sizeof(p_str), "%d%%", progress);
        gfx.getTextBounds(p_str, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((240 - w) / 2, 110);
        gfx.setTextColor(active_theme.subtext);
        gfx.println(p_str);
    }

    struct ThemeColors {
        uint16_t base = CATPPUCCIN_BASE;
        uint16_t text = CATPPUCCIN_TEXT;
        uint16_t subtext = CATPPUCCIN_SUBTEXT0;
        uint16_t overlay = CATPPUCCIN_SURFACE0;
        uint16_t accent = CATPPUCCIN_MAUVE;
        uint16_t green = CATPPUCCIN_GREEN;
        uint16_t yellow = CATPPUCCIN_YELLOW;
        uint16_t red = CATPPUCCIN_RED;
        uint16_t blue = CATPPUCCIN_BLUE;
        uint16_t peach = CATPPUCCIN_PEACH;
        uint16_t sapphire = CATPPUCCIN_SAPPHIRE;
        uint16_t teal = CATPPUCCIN_TEAL;
        uint16_t flamingo = CATPPUCCIN_FLAMINGO;
    } active_theme;

private:
    static DisplayManager* _instance;
    Arduino_HWSPI bus;
    Arduino_ST7789 gfx;
    int currentRotation = 1;
    const int start_x = 10;
    const int start_y = 30;
    const int line_h = 12;
    const int value_x = 55;
};

#endif
