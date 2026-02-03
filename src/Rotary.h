//
// Created by Alexander Christof on 03.02.26.
//

#ifndef HANDYGARAGE_ESP_ROTARY_H
#define HANDYGARAGE_ESP_ROTARY_H

#include <Arduino.h>

// Pins für KY-040
#define ENCODER_CLK 21
#define ENCODER_DT  20
#define ENCODER_SW  15

// Schwellenwert für langen Tastendruck (ms)
#define LONG_PRESS_TIME 800

// Event-Typen
enum RotaryEvent { NONE, ROTARY_CW, ROTARY_CCW, SHORT_PRESS, LONG_PRESS };

// Globales Queue-Handle
QueueHandle_t RotaryEventQueue;

// --- ISRs ---

void IRAM_ATTR handleRotary() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  if (interruptTime - lastInterruptTime > 5) { // Minimales Software-Debounce
    RotaryEvent event = (digitalRead(ENCODER_DT)) ? ROTARY_CW : ROTARY_CCW;
    xQueueSendFromISR(RotaryEventQueue, &event, NULL);
  }
  lastInterruptTime = interruptTime;
}

void IRAM_ATTR handleButton() {
  static unsigned long pressStartTime = 0;
  unsigned long now = millis();

  // Taster am KY-040 ist Low-Active
  if (digitalRead(ENCODER_SW) == LOW) {
    pressStartTime = now; // Zeit beim Drücken merken
  } else {
    // Beim Loslassen Zeit messen
    unsigned long duration = now - pressStartTime;
    if (duration > 50) { // Entprellen
      RotaryEvent event = (duration > LONG_PRESS_TIME) ? LONG_PRESS : SHORT_PRESS;
      xQueueSendFromISR(RotaryEventQueue, &event, NULL);
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