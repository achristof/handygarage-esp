//
// Created by Alexander Christof on 03.02.26.
//

#ifndef HANDYGARAGE_ESP_OLED_H
#define HANDYGARAGE_ESP_OLED_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <bitmaps.h>
#include <qrcode.h>

// oled
#define SCREEN_WIDTH 128 // OLED Breite in Pixeln
#define SCREEN_HEIGHT 64 // OLED Höhe in Pixeln

// Reset-Pin wird bei I2C oft nicht benötigt (-1)
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C // Standard I2C Adresse für SSD1306

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
QRCode qrcode;

// Position der Texte am OLED für die einzelnen Slots
const int yRowText[] = {16, 26, 36, 46, 56};
const int xSlotText = 18;

void drawQRCode(const char* data) {
    uint8_t qrcodeData[qrcode_getBufferSize(3)];

    // Initialisierung: Version 3, niedrige Fehlerkorrektur
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, data);

    int scale = 2; // Jedes QR-Modul wird 2x2 Pixel groß

    // Zentrierung berechnen
    // QR-Größe in Pixeln = qrcode.size * scale (29 * 2 = 58)
    int qrPixelSize = qrcode.size * scale;
    int xOffset = (SCREEN_WIDTH - qrPixelSize) / 2;
    int yOffset = (SCREEN_HEIGHT - qrPixelSize) / 2;

    oled.clearDisplay();

    // QR-Code zeichnen
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                // Ein 2x2 Rechteck für jedes Modul zeichnen
                oled.fillRect(
                  xOffset + (x * scale),
                  yOffset + (y * scale),
                  scale,
                  scale,
                  SSD1306_WHITE
                );
            }
        }
    }

    oled.display();
    oled.invertDisplay(true);
}

void printTextForSlot(int slotNumber, const char* text ) {
    int y;

    switch (slotNumber) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        y = yRowText[slotNumber - 1];
        break;
    default:
        y = yRowText[0];
    }

    oled.fillRect(xSlotText, y, oled.width() - xSlotText - 1, 8, SSD1306_BLACK);
    oled.setCursor(xSlotText, y);
    oled.print(text);
    oled.display();
}

void showSplashscreen() {
    // Zeichne die Bitmap: drawBitmap(x, y, data, breite, höhe, farbe)
    oled.drawBitmap(0, 0, logo, 128, 64, SSD1306_WHITE);
    oled.display();

    delay(3500);

    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("powered by");
    oled.drawBitmap(8, 20, szuLogo, 58, 44, SSD1306_WHITE);
    oled.setCursor(63, 37);
    oled.print("UNGARGASSE");
    oled.display();

    delay(2500);
}

void showSlotscreen() {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("SLOTS:");
    for (int i = 0; i<5; i++) {
        oled.setCursor(0, yRowText[i]);
        oled.printf("%d: leer", i+1);
    }
    oled.display();
}


#endif //HANDYGARAGE_ESP_OLED_H