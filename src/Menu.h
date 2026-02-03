//
// Created by Alexander Christof on 03.02.26.
//

#ifndef HANDYGARAGE_ESP_MENU_H
#define HANDYGARAGE_ESP_MENU_H

#include <Arduino.h>

// Vorhandene Menüs:
// NO_MENU: in keinem Menü
// MAIN_MENU: WIFI-Einstellung..., Garagen-Server..., Garage registrieren, zurück
// WIFI_MENU: Netzwerkscan..., Manuelle Eingabe..., zurück
// WIFI_SCAN: Liste der gefundenen Netzwerke
// WIFI_MANUAL_INPUT: Manuelle Eingabe von SSID und Passwort
enum MenuState { NO_MENU, MAIN_MENU, WIFI_MENU, WIFI_SCAN, WIFI_MANUAL_INPUT };

void drawDisplay(MenuState state, int pos) {
    if (state == MAIN_MENU)
    {

    }
    else if (state == WIFI_MENU)
    {

    }
    else if (state == WIFI_SCAN)
    {

    }
}

void menuTask(void *pvParameters) {
    RotaryEvent event;
    MenuState currentState = NO_MENU;
    int position = 0;

    drawDisplay(currentState, position);

    while (true) {
        // Warten auf Event aus der Queue (blockiert verlustfrei)
        if (xQueueReceive(RotaryEventQueue, &event, portMAX_DELAY)) {

            // 1. Navigation (Drehung)
            if (event == ROTARY_CW) position++;
            if (event == ROTARY_CCW) position--;

            // 2. Aktion (Klick)
            if (event == SHORT_PRESS) {
                if (currentState == NO_MENU)
                {
                    // QR-Code anzeigen
                    drawQRCode("https://www.szu.at");
                    delay(3000);
                    oled.invertDisplay(false);
                    showSlotscreen();
                }
                if (currentState == MAIN_MENU) {
                    if (position == 1)
                    {
                        currentState = WIFI_MENU;
                        position = 0;
                    }
                }
                if (currentState == WIFI_MENU) {

                }
                if (currentState == WIFI_SCAN)
                {

                }
                if (currentState == WIFI_MANUAL_INPUT)
                {

                }
            }

            // 3. Zurück/Spezial (Langer Druck)
            else
            {
                if (event == LONG_PRESS) {
                    if (currentState != MAIN_MENU) {
                        // Zurück zum Hauptmenü
                        currentState = MAIN_MENU;
                        position = 0;
                    }
                }
                drawDisplay(currentState, position);
            }


        }
    }
}

void initializeMenu() {
    // Task auf Kern 1 starten
    xTaskCreatePinnedToCore(menuTask, "MenuTask", 4096, NULL, 1, NULL, 1);
}

#endif //HANDYGARAGE_ESP_MENU_H