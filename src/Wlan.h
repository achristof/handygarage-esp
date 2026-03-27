//
// Created by Alexander Christof on 16.02.26.
//

#ifndef HANDYGARAGE_ESP_WLAN_H
#define HANDYGARAGE_ESP_WLAN_H

#include <Arduino.h>
#include <WiFi.h>

struct WiFiNetwork {
    String ssid;
    uint8_t signalQuality; // Empfangslevel 0..5 (statt dBm)
    bool secure;
};

inline std::vector<WiFiNetwork> networks;

inline void initializeWiFi() {
    WiFi.mode(WIFI_STA); // Nur Station-Modus, kein Access Point
    WiFi.disconnect();   // Vorhandene Verbindungen trennen
    delay(100);         // Kurze Pause, damit die Trennung wirksam wird
}

inline void scanWiFiNetworks() {
    networks.clear();
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
        WiFiNetwork net;
        net.ssid = WiFi.SSID(i);
        int db = WiFi.RSSI(i); // RSSI in dBm, typ. Negative Werte
        // Mappe dBm auf Level 0..5 (0 = sehr schlecht, 5 = sehr gut)
        // Bereichsgrenzen: ≤ -100 -> 0, >= -50 -> 5, linear dazwischen
        if (db <= -100) {
            net.signalQuality = 0;
        } else if (db >= -50) {
            net.signalQuality = 5;
        } else {
            long mapped = map(db, -100, -50, 0, 5); // map liefert long
            // Sicherheitshalber einschränken
            if (mapped < 0) mapped = 0;
            if (mapped > 5) mapped = 5;
            net.signalQuality = (uint8_t)mapped;
        }
        net.secure = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        networks.push_back(net);
    }
}

inline boolean connectToWifi(String ssid, String presharedKey)
{
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(10,yRowText[0]);
    oled.println("Verbinde mit:");
    oled.setCursor(10,yRowText[2]);
    oled.print(ssid);
    oled.display();

    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, presharedKey);
    int count = 0;
    uint16_t timeout = 15000; // 15 Sekunden Timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        if (count++ % 4 == 0)
        {
            oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            oled.setCursor(10,yRowText[2]);
            oled.print(ssid);
            oled.print("     ");
            oled.display();
            oled.setCursor(10,yRowText[2]);
            oled.print(ssid);
        } else
        {
            oled.print(".");
            oled.display();
        }
        if (millis() - startTime > timeout)
        {
            WiFi.disconnect();
            oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            oled.setCursor(10,yRowText[2]);
            oled.print(ssid);
            oled.print("     ");
            oled.display();
            oled.setCursor(10,yRowText[2]);
            oled.println("Verbindungsfehler");
            oled.display();
            delay(2000);
            return false;
        }
    }
    oled.clearDisplay();
    oled.setCursor(10,yRowText[0]);
    oled.println("Verbunden mit");
    oled.setCursor(10,yRowText[1]);
    oled.print("IP-Adresse: ");
    oled.setCursor(10,yRowText[3]);
    oled.print(WiFi.localIP());
    oled.display();
    delay(3000);
    return true;
}

#endif //HANDYGARAGE_ESP_WLAN_H