//
// Created by Alexander Christof on 03.02.26.
//

#ifndef HANDYGARAGE_ESP_MENU_H
#define HANDYGARAGE_ESP_MENU_H

#include <Arduino.h>
#include <menu_data.h>

// Vorhandene Menüs:
// NO_MENU: in keinem Menü
// MAIN_MENU: WIFI-Einstellung..., Garagen-Server..., Garage registrieren, zurück

// WIFI_MENU: Netzwerkscan..., Manuelle Eingabe..., zurück
// WIFI_SCAN: Liste der gefundenen Netzwerke
// WIFI_MANUAL_INPUT: Manuelle Eingabe von SSID und Passwort
enum MenuState { NO_MENU, MAIN_MENU, WIFI_MENU, WIFI_SCAN, WIFI_MANUAL_INPUT };
int menuEntriesCount;;

/** Zeigt das aktuelle Menü auf dem Display an und hebt den Menüeintrag an der Position "pos" hervor
 * @param state aktuelles Menü
 * @param selectedIndex aktuelle Position im Menü
 */
void drawMenu(MenuState state, int selectedIndex) {
    // Wenn kein Menü aktiv, dann nichts anzeigen
    if (state == NO_MENU)
        return;

    // Menü anzeigen
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);

    if (state == MAIN_MENU)
    {
        oled.print("--- HAUPTMEN");
        oled.print(char(0x9A)); // Ü
        oled.println(" ---");

        for (int i = 0; i < menu_main_count; i++)
        {
            if (i == selectedIndex) {
                oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Invertierte Farben für Selektion
            } else {
                oled.setTextColor(SSD1306_WHITE);
            }
            oled.println(menu_main_entries[i]);
        }
        oled.display();
    }
}

void showQRCode()
{
    // QR-Code anzeigen
    drawQRCode("https://www.szu.at");
    delay(3000);
    oled.invertDisplay(false);
    showSlotscreen();
}

/**
 * Task zur Menüverwaltung, der bei leerer Rotary-Event-Queue blockiert
 * @param pvParameters
 */
void menuTask(void *pvParameters) {
    RotaryEvent event;
    MenuState currentState = NO_MENU;
    int selectedIndex = 0;

    //drawMenu(currentState, selected_index);

    while (true) {
        // Warten auf Event aus der Queue (blockiert verlustfrei)
        if (xQueueReceive(RotaryEventQueue, &event, portMAX_DELAY)) {

            // 1. Navigation (Drehung)
            if (event == ROTARY_CW && currentState != NO_MENU)
            {
                selectedIndex = (selectedIndex >= menuEntriesCount -1) ? selectedIndex : selectedIndex + 1;
            }
            if (event == ROTARY_CCW && currentState != NO_MENU)
            {
                selectedIndex = (selectedIndex > 0) ? selectedIndex - 1 : 0;
            }

            // 2. Aktion (Klick)
            if (event == SHORT_PRESS) {
                if (currentState == NO_MENU)
                {
                    showQRCode();
                }
                if (currentState == MAIN_MENU) {
                    switch (selectedIndex)
                    {
                        case 0:
                            // WIFI-Einstellungen...
                            currentState = WIFI_MENU;
                            selectedIndex = 0;
                            drawMenu(currentState, selectedIndex);
                            break;
                        case 1:
                            // Garagen-Server...
                            break;
                        case 2:
                            // Garage registrieren
                            break;
                        case 3:
                            // Zurück
                            currentState = NO_MENU;
                            selectedIndex = 0;
                            showSlotscreen();
                            break;
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
                        selectedIndex = 0;
                        menuEntriesCount = menu_main_count;
                    }
                }
                drawMenu(currentState, selectedIndex);
            }


        }
    }
}

/**
 * Erzeugt den Menü-Task
 */
void initializeMenu() {
    // Task auf Kern 1 starten
    xTaskCreate(menuTask, "MenuTask", 4096, NULL, 1, NULL);
}

#endif //HANDYGARAGE_ESP_MENU_H