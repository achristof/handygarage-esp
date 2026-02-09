//
// Created by Alexander Christof on 08.02.26.
//

#ifndef HANDYGARAGE_ESP_MENU_DATA_H
#define HANDYGARAGE_ESP_MENU_DATA_H

#include <Arduino.h>

// Menü-Text für Hauptmenü
const char menu_main_0[] PROGMEM = "> WIFI-Einstellungen ";
const char menu_main_1[] PROGMEM = "> Garagen-Server     ";
const char menu_main_2[] PROGMEM = "> Garage registrieren";
const char menu_main_3[] PROGMEM = "> Zurueck            ";
const char* const menu_main_entries[] PROGMEM = {menu_main_0, menu_main_1, menu_main_2, menu_main_3};
const int menu_main_count = 4;

#endif //HANDYGARAGE_ESP_MENU_DATA_H