//
// Created by Alexander Christof on 03.02.26.
//

#ifndef HANDYGARAGE_ESP_ROTARY_H
#define HANDYGARAGE_ESP_ROTARY_H

#include <Arduino.h>
#include <Ticker.h>

// Pins für KY-040
#define ENCODER_CLK 23
#define ENCODER_DT  22
#define ENCODER_SW  15

#define DEBOUNCE_TIME 100

// Schwellenwert für langen Tastendruck (ms)
#define LONG_PRESS_TIME 800

// Event-Typen
enum RotaryEvent { NONE, ROTARY_CW, ROTARY_CCW, SHORT_PRESS, LONG_PRESS };
Ticker longPressTimer;
volatile unsigned long lastButtonPressTime = 0;
volatile bool isPressed = false;

// Globales Queue-Handle
QueueHandle_t RotaryEventQueue;

// --- ISRs ---

void IRAM_ATTR handleRotary() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) { // Minimales Software-Debounce
    RotaryEvent event = (digitalRead(ENCODER_DT)) ? ROTARY_CW : ROTARY_CCW;
    xQueueSendFromISR(RotaryEventQueue, &event, NULL);
  }
  lastInterruptTime = interruptTime;
}

// Diese Funktion wird vom Timer aufgerufen, wenn die Zeit abgelaufen ist
void IRAM_ATTR handleLongPressTimeout() {
  if (digitalRead(ENCODER_SW) == LOW) { // Prüfen, ob Taste immer noch gedrückt
    RotaryEvent event = LONG_PRESS;
    xQueueSendFromISR(RotaryEventQueue, &event, NULL);
    isPressed = false; // Verhindert, dass beim Loslassen ein Short Press kommt
  }
}

void IRAM_ATTR handleButton() {
  unsigned long now = millis();

  if (digitalRead(ENCODER_SW) == LOW) { // GEDRÜCKT
    if (!isPressed && (now - lastButtonPressTime > DEBOUNCE_TIME)) {
      isPressed = true;
      lastButtonPressTime = now;
      // Starte Timer: Nach 800ms handleLongPressTimeout aufrufen (einmalig)
      longPressTimer.once_ms(LONG_PRESS_TIME, handleLongPressTimeout);
    }
  }
  else { // LOSGELASSEN
    longPressTimer.detach(); // Timer stoppen, falls er noch läuft
    if (isPressed) {
      unsigned long duration = now - lastButtonPressTime;
      if (duration > DEBOUNCE_TIME) {
        RotaryEvent event = SHORT_PRESS;
        xQueueSendFromISR(RotaryEventQueue, &event, NULL);
      }
      isPressed = false;
    }
  }
}


void initializeRotary() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  // Queue für bis zu 10 Events erstellen
  RotaryEventQueue = xQueueCreate(10, sizeof(RotaryEvent));


  // Interrupts aktivieren
  attachInterrupt(ENCODER_CLK, handleRotary, FALLING);
  attachInterrupt(ENCODER_SW, handleButton, CHANGE); // CHANGE wichtig für Zeitmessung!
}

#endif //HANDYGARAGE_ESP_ROTARY_H