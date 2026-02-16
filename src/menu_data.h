//
// Created by Alexander Christof on 08.02.26.
//

#ifndef HANDYGARAGE_ESP_MENU_DATA_H
#define HANDYGARAGE_ESP_MENU_DATA_H

#include <Arduino.h>

// MAXIMAL 5 Menüeinträge!!

// Hex-Codes für Umlaute: \x81 = ä, \x84 = Ä, \x94 = ö, \x99 = Ö, \x84 = ü, \x9a = Ü, \x9f = ß

// Menütext für Hauptmenü
const char menu_main_title[] PROGMEM = "--- HAUPTMEN\x9a" " ---";
const char menu_main_0[] PROGMEM = "> WIFI-Einstellungen ";
const char menu_main_1[] PROGMEM = "> Garagen-Server     ";
const char menu_main_2[] PROGMEM = "> Garage registrieren";
const char menu_main_3[] PROGMEM = "> Zur\x81" "ck            ";
const char* const menu_main_entries[] PROGMEM = {menu_main_0, menu_main_1, menu_main_2, menu_main_3};
const int menu_main_count = 4;

// Menütext für WIFI-Einstellungen
const char menu_wifi_title[] PROGMEM = "WIFI-Einstellungen";
const char menu_wifi_0[] PROGMEM = "> Scannen...         ";
const char menu_wifi_1[] PROGMEM = "> Manuell eingeben   ";
const char menu_wifi_2[] PROGMEM = "> Zur\x81" "ck             ";
const char* const menu_wifi_entries[] PROGMEM = {menu_wifi_0, menu_wifi_1, menu_wifi_2};
const int menu_wifi_count = 3;

#endif //HANDYGARAGE_ESP_MENU_DATA_H