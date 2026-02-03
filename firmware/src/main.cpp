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

/* 
 * Waveshare ESP32-C6-GEEK Configuration
 * Updated with correct offsets for the 1.14" 240x135 LCD.
 */
#define LCD_SCK 1
#define LCD_MOSI 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

// Added offsets (40, 53) to center the 240x135 area on the ST7789 controller
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, LCD_RST, 1 /* rotation */, true /* IPS */, 
    240 /* width */, 135 /* height */, 
    40 /* col offset 1 */, 53 /* row offset 1 */, 
    40 /* col offset 2 */, 52 /* row offset 2 */
);

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
    gfx->setRotation(1); 
    
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
}

String inputBuffer = "";

void updateDisplay(String hostname, String ip, String mac) {
    gfx->fillScreen(BLACK);
    
    // Header
    gfx->setTextSize(2);
    gfx->setCursor(15, 10);
    gfx->setTextColor(ORANGE);
    gfx->println("SideEye Info");
    
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

    // MAC Info (Smaller text to ensure it fits)
    gfx->setCursor(15, 100);
    gfx->setTextColor(MAGENTA);
    gfx->setTextSize(1);
    gfx->print("MAC:  ");
    gfx->setTextColor(DARKGREY);
    gfx->println(mac);
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            int firstPipe = inputBuffer.indexOf('|');
            int secondPipe = inputBuffer.indexOf('|', firstPipe + 1);

            if (firstPipe > 0 && secondPipe > 0) {
                String hostname = inputBuffer.substring(0, firstPipe);
                String ip = inputBuffer.substring(firstPipe + 1, secondPipe);
                String mac = inputBuffer.substring(secondPipe + 1);
                
                mac.trim(); 
                updateDisplay(hostname, ip, mac);
            }
            
            inputBuffer = "";
        } else {
            inputBuffer += c;
        }
    }
}