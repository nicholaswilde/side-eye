#include <Arduino.h>
#include <Arduino_GFX_Library.h>

/* 
 * Waveshare ESP32-C6-GEEK Configuration
 * NOTE: Pinout based on similar Waveshare ESP32-C6 boards. 
 * Please verify against the official schematic if display does not work.
 * 
 * Common Waveshare ESP32-C6 pin mappings:
 * SCK  : 7
 * MOSI : 6
 * CS   : 14
 * DC   : 15
 * RST  : 21
 * BL   : 22
 */

#define LCD_SCK 7
#define LCD_MOSI 6
#define LCD_CS 14
#define LCD_DC 15
#define LCD_RST 21
#define LCD_BL 22

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */);

void setup() {
    // Initialize Serial (USB CDC)
    Serial.begin(115200);
    // Wait for USB Serial to be ready (optional, good for debugging)
    // while (!Serial) delay(100); 

    // Initialize Display
    if (LCD_BL >= 0) {
        pinMode(LCD_BL, OUTPUT);
        digitalWrite(LCD_BL, HIGH);
    }

    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    
    gfx->fillScreen(BLACK);
    gfx->setRotation(1); // Landscape
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
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
                
                // Trim potential CR
                mac.trim(); 

                updateDisplay(hostname, ip, mac);
            }
            
            inputBuffer = "";
        } else {
            inputBuffer += c;
        }
    }
}
