#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFiManager.h>

#define BLACK RGB565_BLACK
#define WHITE RGB565_WHITE
#define ORANGE RGB565_ORANGE
#define DARKGREY RGB565_DARKGREY
#define CYAN RGB565_CYAN
#define GREEN RGB565_GREEN
#define MAGENTA RGB565_MAGENTA
#define RED RGB565_RED

/* 
 * Waveshare ESP32-C6-GEEK Configuration
 */
#define LCD_SCK 1
#define LCD_MOSI 2
#define LCD_MISO -1
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

// Centering 135x240 Portrait area in 240x320 RAM, then rotating to Landscape
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, LCD_RST, 0 /* rotation */, true /* IPS */,
    135 /* width */, 240 /* height */,
    52 /* col offset 1 */, 40 /* row offset 1 */,
    53 /* col offset 2 */, 40 /* row offset 2 */
);

void drawWiFiStatus() {
    int x = 220;
    int y = 10;
    if (WiFi.status() == WL_CONNECTED) {
        gfx->fillCircle(x, y, 4, GREEN);
    } else {
        gfx->fillCircle(x, y, 4, RED);
    }
}

void configModeCallback (WiFiManager *myWiFiManager) {
    gfx->fillScreen(BLACK);
    gfx->setTextColor(ORANGE);
    gfx->setTextSize(2);
    gfx->setCursor(15, 15);
    gfx->println("SideEye Setup");
    
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(15, 45);
    gfx->println("Connect to WiFi AP:");
    
    gfx->setTextColor(CYAN);
    gfx->setTextSize(2);
    gfx->setCursor(15, 60);
    gfx->println("SideEye-Setup");
    
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(15, 95);
    gfx->println("Then visit:");
    gfx->setTextColor(GREEN);
    gfx->println(WiFi.softAPIP().toString());
}

void setup() {
    Serial.begin(115200);

    if (LCD_BL >= 0) {
        pinMode(LCD_BL, OUTPUT);
        digitalWrite(LCD_BL, HIGH);
    }

    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    
    gfx->fillScreen(BLACK);
    gfx->setRotation(1); // Set to Landscape
    
    WiFiManager wm;
    wm.setAPCallback(configModeCallback);
    
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(15, 40);
    gfx->println("SideEye Boot...");
    
    if (!wm.autoConnect("SideEye-Setup")) {
        ESP.restart();
        delay(1000);
    }
    
    gfx->fillScreen(BLACK);
    gfx->setCursor(15, 40);
    gfx->setTextColor(GREEN);
    gfx->println("WiFi Connected!");
    delay(1000);

    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(15, 40);
    gfx->println("Waiting for Host...");
    drawWiFiStatus();
}

String inputBuffer = "";

void updateDisplay(String hostname, String ip, String mac) {
    gfx->fillScreen(BLACK);
    
    // Header
    gfx->setTextSize(2);
    gfx->setCursor(15, 10);
    gfx->setTextColor(ORANGE);
    gfx->println("SideEye Info");
    
    drawWiFiStatus();
    
    gfx->drawFastHLine(0, 32, 240, DARKGREY);

    // Host Info
    gfx->setCursor(15, 45);
    gfx->setTextColor(CYAN);
    gfx->print("Host: ");
    gfx->setTextColor(WHITE);
    gfx->println(hostname);

    // IP Info
    gfx->setCursor(15, 70);
    gfx->setTextColor(GREEN);
    gfx->print("IP:   ");
    gfx->setTextColor(WHITE);
    gfx->println(ip);

    // MAC Info
    gfx->setCursor(15, 100);
    gfx->setTextColor(MAGENTA);
    gfx->setTextSize(1);
    gfx->print("MAC:  ");
    gfx->setTextColor(DARKGREY);
    gfx->println(mac);
}

void loop() {
    static unsigned long lastWiFiCheck = 0;
    if (millis() - lastWiFiCheck > 5000) {
        drawWiFiStatus();
        lastWiFiCheck = millis();
    }

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            if (inputBuffer.length() > 0 && inputBuffer.length() < 128) {
                int firstPipe = inputBuffer.indexOf('|');
                int secondPipe = inputBuffer.indexOf('|', firstPipe + 1);

                if (firstPipe > 0 && secondPipe > (firstPipe + 1)) {
                    String hostname = inputBuffer.substring(0, firstPipe);
                    String ip = inputBuffer.substring(firstPipe + 1, secondPipe);
                    String mac = inputBuffer.substring(secondPipe + 1);
                    
                    hostname.trim();
                    ip.trim();
                    mac.trim(); 

                    if (hostname.length() > 0 && ip.length() > 0) {
                        updateDisplay(hostname, ip, mac);
                    }
                }
            }
            inputBuffer = "";
        } else {
            if (inputBuffer.length() < 128) {
                inputBuffer += c;
            }
        }
    }
}