//
// Created by Alexander Christof on 30.01.26.
//

#ifndef HANDYGARAGE_ESP_BUTTON_H
#define HANDYGARAGE_ESP_BUTTON_H

// ButtonHandler.h
#pragma once
#include <Arduino.h>
#include <functional>

#ifndef DEBOUNCE_TIME
#define DEBOUNCE_TIME 50
#endif

#ifndef LONG_PRESS_TIME
#define LONG_PRESS_TIME 1000
#endif

class ButtonHandler {
public:
    using Callback = void(*)();

    explicit ButtonHandler(uint8_t pin);
    ~ButtonHandler();

    void begin();                 // Interrupt aktivieren
    void check();                 // periodisch aufrufen (z.B. in loop)

    void setShortPressHandler(Callback cb);
    void setLongPressHandler(Callback cb);

    // ISR-Wrapper als static Memberfunktion (kann private Member aufrufen)
    static void isrArg(void *arg);

private:
    int _pin;
    Callback _shortCb = nullptr;
    Callback _longCb = nullptr;

    volatile unsigned long _lastTriggerTime = 0;
    volatile unsigned long _pressStartTime = 0;
    volatile bool _buttonReleased = false;
    volatile bool _longPressTriggered = false;

    static ButtonHandler* s_instance;

    void handleInterrupt();
};


#endif //HANDYGARAGE_ESP_BUTTON_H