#pragma once

class Arduino_DataBus {
public:
    virtual ~Arduino_DataBus() = default;
};

class Arduino_HWSPI : public Arduino_DataBus {
public:
    Arduino_HWSPI(int dc, int cs, int sck, int mosi, int miso) {}
};

class Arduino_GFX {
public:
    virtual ~Arduino_GFX() = default;
    bool begin(int32_t speed = 0) { return true; }
    void fillScreen(uint16_t color) {}
    void setRotation(uint8_t r) {}
    void setTextColor(uint16_t c) {}
    void setTextSize(uint8_t s) {}
    void setCursor(int16_t x, int16_t y) {}
    void print(const char* s) {}
    void print(String s) {}
    void print(int i) {}
    void print(float f, int p=2) {}
    void println(const char* s) {}
    void println(String s) {}
    void println(int i) {}
    void printf(const char* format, ...) {}
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {}
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {}
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {}
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {}
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
        *x1 = 0; *y1 = 0; *w = 10; *h = 10;
    }
};

class Arduino_ST7789 : public Arduino_GFX {
public:
    Arduino_ST7789(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips, int16_t w, int16_t h, int16_t col_offset1, int16_t row_offset1, int16_t col_offset2, int16_t row_offset2) {}
};