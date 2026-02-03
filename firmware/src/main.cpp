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
 * Updated based on manufacturer documentation for the GEEK dongle version.
 */
#define LCD_SCK 1
#define LCD_MOSI 2
#define LCD_MISO -1
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */);

void configModeCallback (WiFiManager *myWiFiManager) {
    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 10);
    gfx->setTextColor(ORANGE);
    gfx->setTextSize(2);
    gfx->println(" SideEye Setup");
    
    gfx->setTextColor(WHITE);
    gfx->setCursor(0, 40);
    gfx->println("Connect to AP:");
    
    gfx->setTextColor(CYAN);
    gfx->println("SideEye-Setup");
    
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(0, 90);
    gfx->println("Then visit:");
    gfx->println(WiFi.softAPIP().toString());
}

void setup() {
    // Initialize Serial (USB CDC)
    Serial.begin(115200);

    // Initialize Backlight early
    if (LCD_BL >= 0) {
        pinMode(LCD_BL, OUTPUT);
        digitalWrite(LCD_BL, HIGH);
    }

    // Initialize Display
    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    
    gfx->fillScreen(BLACK);
    gfx->setRotation(1); // Landscape
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    
    // WiFi Manager Setup
    WiFiManager wm;
    wm.setAPCallback(configModeCallback);
    
    // Show initial message
    gfx->setCursor(10, 10);
    gfx->println("SideEye Boot...");
    Serial.println("SideEye Booting...");
    
    // AutoConnect
    if (!wm.autoConnect("SideEye-Setup")) {
        Serial.println("failed to connect and hit timeout");
        ESP.restart();
        delay(1000);
    }
    
    // If we get here, we are connected
    gfx->fillScreen(BLACK);
    gfx->setCursor(10, 10);
    gfx->setTextColor(GREEN);
    gfx->println("WiFi Connected!");
    Serial.println("WiFi Connected!");
    delay(1000);

    // Default Waiting Screen
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setCursor(10, 10);
    gfx->println("SideEye Waiting...");
}

String inputBuffer = "";

void updateDisplay(String hostname, String ip, String mac) {
    gfx->fillScreen(BLACK);
    
    gfx->setCursor(0, 10);
    gfx->setTextColor(ORANGE);
    gfx->setTextSize(2);
    gfx->println(" SideEye Info");
    
    gfx->drawFastHLine(0, 30, 240, DARKGREY);

    gfx->setTextColor(CYAN);
    gfx->setTextSize(2);
    gfx->setCursor(0, 45);
    gfx->print(" Host: ");
    gfx->setTextColor(WHITE);
    gfx->println(hostname);

    gfx->setTextColor(GREEN);
    gfx->setCursor(0, 75);
    gfx->print(" IP:   ");
    gfx->setTextColor(WHITE);
    gfx->println(ip);

    gfx->setTextColor(MAGENTA);
    gfx->setCursor(0, 105);
    gfx->print(" MAC:  ");
    gfx->setTextColor(WHITE);
    gfx->setTextSize(1); // Smaller font for MAC to fit
    gfx->println(mac);
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            // Parse payload: "HOSTNAME|IP|MAC"
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
