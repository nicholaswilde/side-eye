#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include "catppuccin_colors.h"
#include "HistoryBuffer.h"
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
};

class DisplayManager {
public:
    DisplayManager() : 
        bus(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, LCD_MISO),
        gfx(&bus, LCD_RST, 0 /* rotation */, true /* IPS */,
            135 /* width */, 240 /* height */,
            52 /* col offset 1 */, 40 /* row offset 1 */,
            53 /* col offset 2 */, 40 /* row offset 2 */)
    {}

    ~DisplayManager() {}

    // Disable copy and assignment
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;

    void begin() {
        if (LCD_BL >= 0) {
            pinMode(LCD_BL, OUTPUT);
            digitalWrite(LCD_BL, HIGH);
        }
        gfx.begin();
        gfx.setRotation(currentRotation);
        gfx.fillScreen(CATPPUCCIN_BASE);
    }

    void setRotation(int rotation) {
        currentRotation = rotation;
        gfx.setRotation(currentRotation);
    }

    int getRotation() {
        return currentRotation;
    }

    void setBacklight(bool on) {
        digitalWrite(LCD_BL, on ? HIGH : LOW);
    }

    void fillScreen(uint16_t color) {
        gfx.fillScreen(color);
    }

    void drawBanner(const char* title, uint8_t alert_level = 0) {
        uint16_t bg_color = CATPPUCCIN_MAUVE;
        if (alert_level == 1) {
            bg_color = CATPPUCCIN_YELLOW;
        } else if (alert_level >= 2) {
            // Flash red/base if critical
            bg_color = ((millis() / 500) % 2 == 0) ? CATPPUCCIN_RED : CATPPUCCIN_BASE;
        }

        gfx.fillRect(0, 0, 240, 20, bg_color);
        gfx.setTextColor(bg_color == CATPPUCCIN_BASE ? CATPPUCCIN_RED : CATPPUCCIN_CRUST);
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
            gfx.fillCircle(x, y, 3, CATPPUCCIN_GREEN);
        } else {
            gfx.fillCircle(x, y, 3, CATPPUCCIN_RED);
        }
    }

    void drawProgressBar(int x, int y, int w, int h, float percent, uint16_t color) {
        gfx.drawRect(x, y, w, h, CATPPUCCIN_SURFACE0);
        int fill_w = (int)((w - 2) * (percent / 100.0));
        if (fill_w < 0) fill_w = 0;
        if (fill_w > w - 2) fill_w = w - 2;
        gfx.fillRect(x + 1, y + 1, w - 2, h - 2, CATPPUCCIN_BASE);
        gfx.fillRect(x + 1, y + 1, fill_w, h - 2, color);
    }

    template <typename T, size_t Size>
    void drawSparkline(int x, int y, int w, int h, const HistoryBuffer<T, Size>& buffer, uint16_t color) {
        gfx.drawRect(x, y, w, h, CATPPUCCIN_SURFACE0);
        gfx.fillRect(x + 1, y + 1, w - 2, h - 2, CATPPUCCIN_BASE);

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
            gfx.setTextColor(CATPPUCCIN_BLUE);
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.print("Host: ");

            gfx.setCursor(start_x, start_y + line_h * 2.5);
            gfx.setTextColor(CATPPUCCIN_GREEN);
            gfx.print("IP:   ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(CATPPUCCIN_FLAMINGO);
            gfx.print("MAC:  ");
        } else {
            gfx.setTextColor(CATPPUCCIN_TEXT);
            
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.println(state.hostname);

            gfx.fillRect(value_x, (int)(start_y + line_h * 2.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 2.5));
            gfx.println(state.ip);

            gfx.fillRect(value_x, (int)(start_y + line_h * 3.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 3.5));
            gfx.println(state.mac);
        }
    }

    void drawResourcesPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(CATPPUCCIN_PEACH);
            gfx.print("CPU:  ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(CATPPUCCIN_SAPPHIRE);
            gfx.print("RAM:  ");
        } else {
            gfx.setTextColor(CATPPUCCIN_TEXT);

            // CPU
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 100, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.print(state.cpu_percent, 1);
            gfx.println("%");
            uint16_t cpu_col = (state.cpu_percent > 80) ? CATPPUCCIN_RED : (state.cpu_percent > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, state.cpu_percent, cpu_col);

            // RAM
            gfx.fillRect(value_x, (int)(start_y + line_h * 3.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 3.5));
            gfx.printf("%llu / %llu MB", state.ram_used / 1024 / 1024, state.ram_total / 1024 / 1024);
            float ram_p = (state.ram_total > 0) ? (float)state.ram_used / state.ram_total * 100.0 : 0;
            uint16_t ram_col = (ram_p > 80) ? CATPPUCCIN_RED : (ram_p > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
            drawProgressBar(start_x, start_y + line_h * 4.5, 220, 8, ram_p, ram_col);
        }
    }

    void drawStatusPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(CATPPUCCIN_TEAL);
            gfx.print("Disk: ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(CATPPUCCIN_SUBTEXT0);
            gfx.print("Uptime: ");
        } else {
            gfx.setTextColor(CATPPUCCIN_TEXT);

            // Disk
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.printf("%llu / %llu GB", state.disk_used / 1024 / 1024 / 1024, state.disk_total / 1024 / 1024 / 1024);
            float disk_p = (state.disk_total > 0) ? (float)state.disk_used / state.disk_total * 100.0 : 0;
            uint16_t disk_col = (disk_p > 80) ? CATPPUCCIN_RED : (disk_p > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, disk_p, disk_col);

            // Uptime
            gfx.fillRect(value_x + 20, (int)(start_y + line_h * 3.5), 160, 8, CATPPUCCIN_BASE);
            uint32_t h_up = state.uptime / 3600;
            uint32_t m_up = (state.uptime % 3600) / 60;
            gfx.setCursor(value_x + 20, (int)(start_y + line_h * 3.5));
            gfx.printf("%luh %lum", (unsigned long)h_up, (unsigned long)m_up);
        }
    }

    void drawSDPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(CATPPUCCIN_MAUVE);
            gfx.print("SD Card:");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(CATPPUCCIN_YELLOW);
            gfx.print("Sync:");
        } else {
            gfx.setTextColor(CATPPUCCIN_TEXT);

            // SD Storage
            uint64_t total = SD.totalBytes();
            uint64_t used = SD.usedBytes();
            gfx.fillRect(value_x + 20, (int)(start_y + line_h * 1.5), 160, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x + 20, (int)(start_y + line_h * 1.5));
            gfx.printf("%llu / %llu MB", used / 1024 / 1024, total / 1024 / 1024);
            // cppcheck-suppress knownConditionTrueFalse
            float sd_p = (total > 0) ? (float)used / total * 100.0 : 0;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, sd_p, CATPPUCCIN_MAUVE);

            // Sync Status
            gfx.fillRect(value_x + 10, (int)(start_y + line_h * 3.5), 170, 8, CATPPUCCIN_BASE);
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
            gfx.setTextColor(CATPPUCCIN_RED);
            gfx.print("Temp: ");

            gfx.setCursor(start_x, start_y + line_h * 3.5);
            gfx.setTextColor(CATPPUCCIN_GREEN);
            gfx.print("GPU:  ");
        } else {
            gfx.setTextColor(CATPPUCCIN_TEXT);

            // Thermal
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 100, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.print(state.thermal_c, 1);
            gfx.println(" C");
            uint16_t temp_col = (state.thermal_c > 80) ? CATPPUCCIN_RED : (state.thermal_c > 65) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
            drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, state.thermal_c, temp_col);

            // GPU
            gfx.fillRect(value_x, (int)(start_y + line_h * 3.5), 100, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 3.5));
            gfx.print(state.gpu_percent, 1);
            gfx.println("%");
            uint16_t gpu_col = (state.gpu_percent > 80) ? CATPPUCCIN_RED : (state.gpu_percent > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
            drawProgressBar(start_x, start_y + line_h * 4.5, 220, 8, state.gpu_percent, gpu_col);
        }
    }

    void drawNetworkPage(const SystemState& state, bool labelsOnly) {
        if (labelsOnly) {
            gfx.setCursor(start_x, start_y + line_h * 1.5);
            gfx.setTextColor(CATPPUCCIN_GREEN);
            gfx.print("Down:");

            gfx.setCursor(start_x, start_y + line_h * 4.5);
            gfx.setTextColor(CATPPUCCIN_MAUVE);
            gfx.print("Up:");
        } else {
            gfx.setTextColor(CATPPUCCIN_TEXT);

            // Download
            gfx.fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 1.5));
            gfx.print(formatSpeed(state.net_down));
            drawSparkline(start_x, start_y + line_h * 2.5, 220, 20, state.net_down_history, CATPPUCCIN_GREEN);

            // Upload
            gfx.fillRect(value_x, (int)(start_y + line_h * 4.5), 180, 8, CATPPUCCIN_BASE);
            gfx.setCursor(value_x, (int)(start_y + line_h * 4.5));
            gfx.print(formatSpeed(state.net_up));
            drawSparkline(start_x, start_y + line_h * 5.5, 220, 20, state.net_up_history, CATPPUCCIN_MAUVE);
        }
    }

    String formatSpeed(uint64_t bytesPerSec) {
        if (bytesPerSec < 1024) return String(bytesPerSec) + " B/s";
        if (bytesPerSec < 1024 * 1024) return String(bytesPerSec / 1024.0, 1) + " KB/s";
        return String(bytesPerSec / (1024.0 * 1024.0), 1) + " MB/s";
    }

    void drawStaticUI(const SystemState& state, Page currentPage, const char* version) {
        gfx.fillScreen(CATPPUCCIN_BASE);
        drawBanner("SIDEEYE MONITOR", state.alert_level);
        drawWiFiStatus();

        gfx.setTextSize(1);
        
        gfx.setTextColor(CATPPUCCIN_YELLOW);
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
        gfx.fillRect(value_x, start_y, 140, 8, CATPPUCCIN_BASE);
        gfx.setCursor(value_x, start_y);
        if (state.connected) {
            gfx.setTextColor(CATPPUCCIN_GREEN);
            gfx.println("Connected");
        } else {
            gfx.setTextColor(CATPPUCCIN_PEACH);
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
        gfx.fillScreen(CATPPUCCIN_BASE);
        drawBanner("BOOTING...");
        
        int16_t x1, y1;
        uint16_t w, h;
        int screen_w = 240; // Landscape width
        
        // Draw SideEye name
        gfx.setTextSize(2);
        gfx.setTextColor(CATPPUCCIN_MAUVE);
        const char* name = "SideEye";
        gfx.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((screen_w - w) / 2, 55);
        gfx.println(name);
        
        // Draw Version
        gfx.setTextSize(1);
        gfx.setTextColor(CATPPUCCIN_SUBTEXT0);
        char v_str[32];
        snprintf(v_str, sizeof(v_str), "v%s", version);
        gfx.getTextBounds(v_str, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((screen_w - w) / 2, 85);
        gfx.println(v_str);
    }

    void drawConfigMode(const char* apName, const String& ip) {
        gfx.fillScreen(CATPPUCCIN_BASE);
        drawBanner("SETUP MODE", 1);
        
        gfx.setTextColor(CATPPUCCIN_TEXT);
        gfx.setTextSize(1);
        gfx.setCursor(15, 45);
        gfx.println("Connect to WiFi AP:");
        
        gfx.setTextColor(CATPPUCCIN_YELLOW);
        gfx.setCursor(15, 60);
        gfx.println(apName);
        
        gfx.setTextColor(CATPPUCCIN_TEXT);
        gfx.setCursor(15, 90);
        gfx.print("Then visit:");
        
        gfx.setTextColor(CATPPUCCIN_GREEN);
        gfx.setCursor(90, 90);
        gfx.println(ip);
    }

    void drawWiFiOnline() {
        gfx.fillScreen(CATPPUCCIN_BASE);
        drawBanner("CONNECTED");
        gfx.setCursor(15, start_y);
        gfx.setTextColor(CATPPUCCIN_GREEN);
        gfx.println("WiFi Online!");
    }

    void drawResetScreen(int secondsRemaining, bool forceRedraw = false) {
        if (forceRedraw) {
            gfx.fillScreen(CATPPUCCIN_BASE);
            drawBanner("FACTORY RESET", 0); // Use alert 0 to avoid banner flashing
            
            gfx.setTextColor(CATPPUCCIN_TEXT);
            gfx.setTextSize(1);
            gfx.setCursor(15, 50);
            gfx.println("Resetting in:");
            
            gfx.setTextColor(CATPPUCCIN_SUBTEXT0);
            gfx.setTextSize(1);
            gfx.setCursor(15, 115);
            gfx.println("Release to cancel");
        }
        
        // Clear and update only the number area
        gfx.fillRect(100, 75, 40, 25, CATPPUCCIN_BASE);
        gfx.setTextColor(CATPPUCCIN_RED);
        gfx.setTextSize(3);
        gfx.setCursor(100, 75);
        gfx.println(secondsRemaining);
    }

private:
    Arduino_HWSPI bus;
    Arduino_ST7789 gfx;
    int currentRotation = 1;
    const int start_x = 10;
    const int start_y = 30;
    const int line_h = 12;
    const int value_x = 55;
};

#endif
