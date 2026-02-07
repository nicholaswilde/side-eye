#pragma once
#include <stdint.h>
#include <iostream>
#include <string>
#include <cstdio>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

extern unsigned long _mock_millis;
inline unsigned long millis() { return _mock_millis; }
inline void delay(unsigned long ms) { _mock_millis += ms; }

extern int _mock_digitalRead_val;
inline int digitalRead(uint8_t pin) { return _mock_digitalRead_val; }
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline void pinMode(uint8_t pin, uint8_t mode) {}

class String : public std::string {
public:
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    String() : std::string() {}
    explicit String(int i) : std::string(std::to_string(i)) {}
    explicit String(unsigned long i) : std::string(std::to_string(i)) {}
    explicit String(long i) : std::string(std::to_string(i)) {}
    String(double d, int precision) {
        char buf[32];
        sprintf(buf, "%.*f", precision, d);
        assign(buf);
    }
    const char* c_str() const { return std::string::c_str(); }
    size_t write(uint8_t c) { push_back(c); return 1; }
    size_t write(const uint8_t *s, size_t n) { append((const char*)s, n); return n; }
};

class SerialMock {
public:
    void begin(unsigned long baud) {}
    void println(const char* s) { std::cout << s << std::endl; }
    void println(String s) { std::cout << s.c_str() << std::endl; }
    void print(const char* s) { std::cout << s; }
    void print(String s) { std::cout << s.c_str(); }
    void print(float f, int p=2) { std::cout << f; }
    void printf(const char* format, ...) {}
};
extern SerialMock Serial;