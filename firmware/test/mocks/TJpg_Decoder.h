#pragma once
#include <stdint.h>
#include <Arduino.h>

extern int _mock_tjpg_draw_count;

class TJpg_Decoder {
public:
    typedef bool (*TJpg_Callback)(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

    void setCallback(TJpg_Callback cb) { _cb = cb; }
    void setJpgScale(uint8_t scale) { _scale = scale; }
    void setSwapBytes(bool swap) { _swap = swap; }

    bool drawSdJpg(int16_t x, int16_t y, const char* path) {
        _mock_tjpg_draw_count++;
        // In mock, just call the callback once with a dummy bitmap if the file exists
        if (_cb) {
            uint16_t dummy_bitmap[1] = {0xFFFF};
            _cb(x, y, 1, 1, dummy_bitmap);
        }
        return true;
    }

private:
    TJpg_Callback _cb = nullptr;
    uint8_t _scale = 1;
    bool _swap = false;
};

extern TJpg_Decoder TJpgDec;
extern int _mock_tjpg_draw_count;
