// src/Button.cpp
#include "ButtonHandler.h"

void ButtonHandler::isrArg(void *arg) {
    if (!arg) return;
    static_cast<ButtonHandler*>(arg)->handleInterrupt();
}

ButtonHandler::ButtonHandler(uint8_t pin)
: _pin(pin)
{
}

ButtonHandler::~ButtonHandler() {
    detachInterrupt(digitalPinToInterrupt(_pin));
}

void ButtonHandler::begin() {
    pinMode(_pin, INPUT);
    attachInterruptArg(digitalPinToInterrupt(_pin), ButtonHandler::isrArg, this, CHANGE);
}

void ButtonHandler::setShortPressHandler(Callback cb) {
    _shortCb = cb;
}

void ButtonHandler::setLongPressHandler(Callback cb) {
    _longCb = cb;
}

void IRAM_ATTR ButtonHandler::handleInterrupt() {
    const unsigned long now = millis();

    if (now - _lastTriggerTime < DEBOUNCE_TIME) return;
    _lastTriggerTime = now;

    int state = digitalRead(_pin);

    if (state == LOW) {     // gedrückt
        _pressStartTime = now;
        _longPressTriggered = false;
    } else {                // losgelassen
        _buttonReleased = true;
    }
}

void ButtonHandler::check() {
    // Prüfung auf langen Tastendruck (während noch gedrückt)
    if (digitalRead(_pin) == LOW && !_longPressTriggered) {
        if (millis() - _pressStartTime > LONG_PRESS_TIME && _pressStartTime != 0) {
            _longPressTriggered = true;
            if (_longCb) _longCb();
        }
    }

    // Prüfung auf kurzen Tastendruck (nach Loslassen)
    if (_buttonReleased) {
        const unsigned long duration = millis() - _pressStartTime;
        _buttonReleased = false;

        if (duration < LONG_PRESS_TIME && !_longPressTriggered) {
            if (_shortCb) _shortCb();
        }
        _pressStartTime = 0;
    }
}
