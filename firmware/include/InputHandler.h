#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "DisplayManager.h"

class Button {
public:
    enum Event { NONE, CLICK, DOUBLE_CLICK, HOLD, LONG_HOLD };

    explicit Button(int pin) : _pin(pin) {}

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
                    _longHoldReported = false;
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

        if (_stableState == LOW) {
            if (!_holdReported && (now - _pressStartTime > _holdDelay)) {
                event = HOLD;
                _holdReported = true;
                _pendingClick = false;
            }
            if (!_longHoldReported && (now - _pressStartTime > _longHoldDelay)) {
                event = LONG_HOLD;
                _longHoldReported = true;
            }
        }

        if (_pendingClick && (now - _lastClickTime > _doubleClickDelay)) {
            _pendingClick = false;
            event = CLICK;
        }

        _lastState = currentState;
        return event;
    }

    bool isPressed() const { return _stableState == LOW; }
    unsigned long getPressDuration() const { return isPressed() ? (millis() - _pressStartTime) : 0; }

private:
    int _pin;
    int _lastState = HIGH;
    int _stableState = HIGH;
    unsigned long _lastDebounceTime = 0;
    unsigned long _debounceDelay = 50;
    unsigned long _pressStartTime = 0;
    unsigned long _holdDelay = 800;
    unsigned long _longHoldDelay = 10000;
    unsigned long _lastClickTime = 0;
    unsigned long _doubleClickDelay = 300;
    bool _holdReported = false;
    bool _longHoldReported = false;
    bool _pendingClick = false;
};

class InputHandler {
public:
    InputHandler(int buttonPin, DisplayManager& display) : _button(buttonPin), _display(display) {}

    void begin() {
        _button.begin();
        _lastActivityTime = millis();
    }

    bool update(SystemState& state, Page& currentPage, unsigned long& lastPageChange, bool& needsStaticDraw, const char* version) {
        Button::Event ev = _button.update();
        bool resetTriggered = false;
        
        if (ev != Button::NONE) {
            _lastActivityTime = millis();
        }

        if (ev == Button::CLICK) {
            if (!_isScreenOn) {
                setScreenOn(true);
                needsStaticDraw = true; // Force redraw on wake
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
            if (_isScreenOn) needsStaticDraw = true; // Force redraw on wake
        } else if (ev == Button::LONG_HOLD) {
            resetTriggered = true;
        }

        // Reset countdown visual feedback
        if (_button.isPressed() && _isScreenOn) {
            unsigned long duration = _button.getPressDuration();
            if (duration > 2000) { // Show reset warning after 2s of holding
                int remaining = (10000 - duration) / 1000;
                if (remaining >= 0 && remaining != _lastRemaining) {
                    _display.drawResetScreen(remaining, !_resetScreenActive);
                    _lastRemaining = remaining;
                    _resetScreenActive = true;
                }
            }
        } else if (_resetScreenActive) {
            _resetScreenActive = false;
            _lastRemaining = -1;
            needsStaticDraw = true; // Restore normal UI when button released
        }

        // Auto-off for screen
        if (_isScreenOn && (millis() - _lastActivityTime > _autoOffDelay)) {
            setScreenOn(false);
        }

        return resetTriggered;
    }

    void notifyActivity() {
        _lastActivityTime = millis();
        if (!_isScreenOn) {
            setScreenOn(true);
            // We don't set needsStaticDraw here because main.cpp loop or handleJson will usually follow up with one
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
    bool _resetScreenActive = false;
    int _lastRemaining = -1;
    unsigned long _lastActivityTime = 0;
    const unsigned long _autoOffDelay = 60000; // 1 minute
};

#endif
