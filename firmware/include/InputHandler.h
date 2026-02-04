#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "DisplayManager.h"

class Button {
public:
    enum Event { NONE, CLICK, DOUBLE_CLICK, HOLD };

    Button(int pin) : _pin(pin) {}

    void begin() {
        pinMode(_pin, INPUT_PULLUP);
        _lastState = digitalRead(_pin);
    }

    Event update() {
        int currentState = digitalRead(_pin);
        unsigned long now = millis();
        Event event = NONE;

        if (currentState != _lastState) {
            _lastDebounceTime = now;
        }

        if ((now - _lastDebounceTime) > _debounceDelay) {
            if (currentState != _stableState) {
                _stableState = currentState;
                if (_stableState == LOW) { // Pressed
                    _pressStartTime = now;
                    _holdReported = false;
                    if (now - _lastClickTime < _doubleClickDelay) {
                        _pendingClick = false;
                        event = DOUBLE_CLICK;
                    }
                } else { // Released
                    if (!_holdReported && event != DOUBLE_CLICK) {
                        _pendingClick = true;
                        _lastClickTime = now;
                    }
                }
            }
        }

        if (_stableState == LOW && !_holdReported) {
            if (now - _pressStartTime > _holdDelay) {
                event = HOLD;
                _holdReported = true;
                _pendingClick = false;
            }
        }

        if (_pendingClick && (now - _lastClickTime > _doubleClickDelay)) {
            _pendingClick = false;
            event = CLICK;
        }

        _lastState = currentState;
        return event;
    }

private:
    int _pin;
    int _lastState = HIGH;
    int _stableState = HIGH;
    unsigned long _lastDebounceTime = 0;
    unsigned long _debounceDelay = 50;
    unsigned long _pressStartTime = 0;
    unsigned long _holdDelay = 800;
    unsigned long _lastClickTime = 0;
    unsigned long _doubleClickDelay = 300;
    bool _holdReported = false;
    bool _pendingClick = false;
};

class InputHandler {
public:
    InputHandler(int buttonPin, DisplayManager& display) : _button(buttonPin), _display(display) {}

    void begin() {
        _button.begin();
        _lastActivityTime = millis();
    }

    void update(SystemState& state, Page& currentPage, unsigned long& lastPageChange, bool& needsStaticDraw, const char* version) {
        Button::Event ev = _button.update();
        
        if (ev != Button::NONE) {
            _lastActivityTime = millis();
        }

        if (ev == Button::CLICK) {
            if (!_isScreenOn) {
                setScreenOn(true);
            } else {
                // Next page
                currentPage = static_cast<Page>((currentPage + 1) % NUM_PAGES);
                lastPageChange = millis();
                needsStaticDraw = true;
                _display.updateDynamicValues(state, currentPage, needsStaticDraw, false, version);
            }
        } else if (ev == Button::DOUBLE_CLICK) {
            int newRotation = (_display.getRotation() == 1) ? 3 : 1;
            _display.setRotation(newRotation);
            needsStaticDraw = true;
            _display.updateDynamicValues(state, currentPage, needsStaticDraw, false, version);
        } else if (ev == Button::HOLD) {
            setScreenOn(!_isScreenOn);
        }

        // Auto-off for screen
        if (_isScreenOn && (millis() - _lastActivityTime > _autoOffDelay)) {
            setScreenOn(false);
        }
    }

    void notifyActivity() {
        _lastActivityTime = millis();
        if (!_isScreenOn) {
            setScreenOn(true);
        }
    }

    bool isScreenOn() const { return _isScreenOn; }

private:
    void setScreenOn(bool on) {
        _isScreenOn = on;
        _display.setBacklight(on);
    }

    Button _button;
    DisplayManager& _display;
    bool _isScreenOn = true;
    unsigned long _lastActivityTime = 0;
    const unsigned long _autoOffDelay = 60000; // 1 minute
};

#endif
