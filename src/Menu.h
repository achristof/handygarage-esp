//
// Created by Alexander Christof on 03.02.26.
//

#ifndef HANDYGARAGE_ESP_MENU_H
#define HANDYGARAGE_ESP_MENU_H

#include <Arduino.h>
#include <menu_data.h>
#include <Wlan.h>
#include <string>

#include "oled.h"
#include "../../../../../.platformio/packages/toolchain-riscv32-esp/riscv32-esp-elf/include/sys/_stdint.h"

// Vorhandene Menüs:
// NO_MENU: in keinem Menü
// MAIN_MENU: WIFI-Einstellung..., Garagen-Server..., Garage registrieren, zurück

// WIFI_MENU: Netzwerkscan..., Manuelle Eingabe..., zurück
// WIFI_SCAN: Liste der gefundenen Netzwerke
// WIFI_MANUAL_INPUT: Manuelle Eingabe von SSID und Passwort
enum MenuState { NO_MENU, MAIN_MENU, WIFI_MENU, WIFI_SCAN, WIFI_SCAN_RESULT, WIFI_MANUAL_INPUT, ENTER_TEXT };
inline int menuEntriesCount;


inline String createSpaces(int n) {
    String s = "";
    s.reserve(n); // Reserviert Speicher für n Zeichen
    for (int i = 0; i < n; i++) {
        s += " ";
    }
    return s;
}

inline void showScanScreen() {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.println(menu_wifi_scan_title);
    oled.drawFastHLine(0, 10, oled.width(), SSD1306_WHITE);
    oled.setCursor(10, yRowText[1]);
    oled.println("Scanne...");
    oled.display();
}

// Zeigt die 4 stärksten gefundenen WiFi-Netzwerke am OLED an
inline void showWiFiScanResult() {
    const int maxSsidLength = 17;
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.println(menu_wifi_scan_title);
    oled.drawFastHLine(0, 10, oled.width(), SSD1306_WHITE);

    if (networks.empty()) {
        // Kein Netzwerk gefunden
        oled.setCursor(0, yRowText[1]);
        oled.println("Keine Netzwerke gefunden");
        return;
    }

    for (int i = 0; i < networks.size() && i < 4; ++i) {
        // Netzwerk-SSID anzeigen
        const WiFiNetwork& net = networks[i];
        oled.setCursor(12, yRowText[i]);
        String ssid = net.ssid;
        // SSID auf die max. Zeichen kürzen, damit sie auf das Display passt.
        // Kürzung mit "..." am Ende, wenn sie zu lang ist.
        // Auffüllen mit Leerzeichen, damit die Zeile immer gleich lang ist (für sauberes Überschreiben).
        int length = ssid.length();
        if (length > maxSsidLength) {
            ssid = ssid.substring(0, maxSsidLength - 3) + "...";
        } else {
            ssid = ssid + createSpaces(maxSsidLength - length);
        }
        oled.print(ssid);

        // Signalstärke anzeigen
        oled.drawRect(113, yRowText[i], 14, 6, SSD1306_BLACK); // Rahmen für Signalstärke
        switch (net.signalQuality) {
            case 0: oled.drawBitmap(113,yRowText[i],wlan0,14,6,SSD1306_WHITE); break;
            case 1: oled.drawBitmap(113,yRowText[i],wlan1,14,6,SSD1306_WHITE); break;
            case 2: oled.drawBitmap(113,yRowText[i],wlan2,14,6,SSD1306_WHITE); break;
            case 3: oled.drawBitmap(113,yRowText[i],wlan3,14,6,SSD1306_WHITE); break;
            case 4: oled.drawBitmap(113,yRowText[i],wlan4,14,6,SSD1306_WHITE); break;
            case 5: oled.drawBitmap(113,yRowText[i],wlan5,14,6,SSD1306_WHITE); break;
        }
    }
    oled.setCursor(10, yRowText[(networks.size()>4) ? 4 : networks.size()]);
    oled.print("Zur\x81" "ck...");
    oled.display();
}

/** Zeigt das aktuelle Menü auf dem Display an und hebt den Menüeintrag an der Position "pos" hervor
 * @param state aktuelles Menü
 * @param selectedIndex aktuelle Position im Menü
 */
inline void drawMenu(MenuState state, int selectedIndex) {
    // Wenn kein Menü aktiv, dann nichts anzeigen
    if (state == NO_MENU)
        return;

    // zeichnet nur den Auswahlpfeil bei den gefundenen WLAN-SSIDs
    if (state == WIFI_SCAN_RESULT)
    {
        menuEntriesCount = menu_wifi_scan_count + 1;
        for (int i = 0; i < menuEntriesCount; i++)
        {
            if (i == selectedIndex) {
                oled.drawBitmap(0,yRowText[i],arrowRight,6,8,SSD1306_WHITE);
            } else {
                oled.fillRect(0,yRowText[i],6,8,SSD1306_BLACK);
            }
        }
        oled.display();
        return;
    }

    // "normale" Menüs anzeigen anzeigen
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);

    if (state == MAIN_MENU)
    {
        oled.print(menu_main_title);
        oled.drawFastHLine(0, 10, oled.width(), SSD1306_WHITE);

        for (int i = 0; i < menu_main_count; i++)
        {
            oled.setCursor(0,yRowText[i]);
            if (i == selectedIndex) {
                oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Invertierte Farben für Selektion
            } else {
                oled.setTextColor(SSD1306_WHITE);
            }
            oled.println(menu_main_entries[i]);
        }
        menuEntriesCount = menu_main_count;
        oled.display();
    }

    // zeichnet das WIFI-Menü
    if (state == WIFI_MENU)
    {
        oled.print(menu_wifi_title);
        oled.drawFastHLine(0, 10, oled.width(), SSD1306_WHITE);

        for (int i = 0; i < menu_wifi_count; i++)
        {
            oled.setCursor(0,yRowText[i]);
            if (i == selectedIndex) {
                oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Invertierte Farben für Selektion
            } else {
                oled.setTextColor(SSD1306_WHITE);
            }
            oled.println(menu_wifi_entries[i]);
        }
        menuEntriesCount = menu_wifi_count;
        oled.display();
    }
}

inline void showQRCode()
{
    // QR-Code anzeigen
    drawQRCode("https://www.szu.at");
    delay(5000);
    oled.invertDisplay(false);
    showSlotscreen();
}

/**
 * Task zur Menüverwaltung, der bei leerer Rotary-Event-Queue blockiert
 * @param pvParameters
 */
inline void menuTask(void *pvParameters) {
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
                switch (currentState)
                {
                    case NO_MENU:
                        showQRCode();
                        break;
                    case MAIN_MENU:
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
                        break;
                    case WIFI_MENU:
                        switch (selectedIndex)
                        {
                            case 0:
                                // Scannen...
                                showScanScreen();
                                scanWiFiNetworks();
                                showWiFiScanResult();
                                currentState = WIFI_SCAN_RESULT;
                                selectedIndex = 0;
                                menu_wifi_scan_count = (networks.size() > 4)? 4 : networks.size();
                                drawMenu(currentState, selectedIndex);
                                break;
                            case 1:
                                // Manuell eingeben
                                break;
                            case 2:
                                // Zurück
                                currentState = MAIN_MENU;
                                selectedIndex = 0;
                                drawMenu(currentState, selectedIndex);
                                break;
                        }
                        break;
                    case WIFI_MANUAL_INPUT:
                        break;
                    case WIFI_SCAN_RESULT:
                        if (selectedIndex >= menuEntriesCount)
                        {
                            selectedIndex = 4;
                        }
                        switch (selectedIndex)
                        {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                // Verbindung mit ausgewähltem Netzwerk herstellen
                                connectToWifi(networks[selectedIndex].ssid, "tri++reme");
                                delay(5000);
                                currentState = NO_MENU;
                                selectedIndex = 0;
                                showSlotscreen();
                                break;
                            case 4:
                                // Zurück
                                currentState = WIFI_MENU;
                                selectedIndex = 0;
                                drawMenu(currentState, selectedIndex);
                                break;
                        }
                        break;
                }
            }

            // 3. Zurück/Spezial (Langer Druck)
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

/**
 * Erzeugt den Menü-Task
 */
inline void initializeMenu() {
    // Task auf Kern 1 starten
    xTaskCreate(menuTask, "MenuTask", 4096, NULL, 1, NULL);
}

#endif //HANDYGARAGE_ESP_MENU_H