#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <bitmaps.h>
#include <Ticker.h>
#include <qrcode.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ButtonHandler.h>

// oled
#define SCREEN_WIDTH 128 // OLED Breite in Pixeln
#define SCREEN_HEIGHT 64 // OLED Höhe in Pixeln

// Rotary encoder
#define ROTARY_SW 15
#define ROTARY_DT 20
#define ROTARY_CLK 21

// Reset-Pin wird bei I2C oft nicht benötigt (-1)
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C // Standard I2C Adresse für SSD1306

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
QRCode qrcode;
Preferences prefs;

// Position der Texte am OLED für die einzelnen Slots
const int ySlotText[] = {16, 26, 36, 46, 56};
const int xSlotText = 18;

// Pins und Staus der Drucksensoren der Slots
const int sensorSlotPin[] = {0, 1, 2, 3, 4};
bool sensorSlotState[] = {false, false, false, false, false};

// FLAGS für ISR
// speichert die Nummer des Slots, wo eine Änderung erfolgt ist (laden/entnehmen)
volatile int slotNumberStateChanged = -1;

// zeigt an, ob der Timer für die Slot-Überprüfung abgelaufen ist
volatile bool timeToCheckSlot = false;

// die Aktion, die beim Drücken des Buttons gewählt werden soll
// 0x00 .. kein Menü (Handy soll registriert werden)
int buttonAction = 0x00;

// Configuration parameters
String garageId;
String wifi_ssid;
String wifi_password;

// erzeugt eine Instanz von Button
ButtonHandler button(ROTARY_SW);


Ticker timerCheckSlotState;

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
			y = ySlotText[slotNumber - 1];
			break;
		default:
			y = ySlotText[0];
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
		oled.setCursor(0, ySlotText[i]);
		oled.printf("%d: leer", i+1);
	}
	oled.display();
}

// called every 250ms by Timer to check if State of any of the five Slot is changed
void IRAM_ATTR handleTimerInterrupt() {
	timeToCheckSlot = true;
}


void shortPressAction() {
	Serial.println("shortpress erkannt");
	switch (buttonAction) {
		case 0x00: // kein Menü aufgerufen
			drawQRCode("https://www.szu.at");
			delay(3000);
			oled.invertDisplay(false);
			showSlotscreen();
			break;
	}
}

void longPressAction() {
	printTextForSlot(1,"longpress erkannt");
	delay(1000);
	printTextForSlot(1,"leer");
}


void setup() {
	// Serial.begin(115200);


	// Initialize Slotsensors
	for (int i = 0; i < 5; i++) {
		pinMode(sensorSlotPin[i], INPUT);
	}

	// I2C Initialisierung für ESP32-C6 (SDA=6, SCL=7)
	Wire.begin(6, 7);
	// Initialize OLED display
	oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
	// stellt die refresh rate des displays ein, damit QR-Codes gut mit dem
	// Handy gescannt werden können
	oled.ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
	oled.ssd1306_command(0xA0);
	oled.clearDisplay();
	oled.display();
	oled.setTextColor(SSD1306_WHITE);

	// Initialize preferences store and get stored values
	prefs.begin("handygarage", false);
	garageId = (prefs.isKey("garageId")) ? prefs.getString("garageId") : "";
	wifi_ssid = (prefs.isKey("wifi_ssid")) ? prefs.getString("wifi_ssid") : "Trireme";
	wifi_password = (prefs.isKey("wifi_password")) ? prefs.getString("wifi_password") : "tri++reme";
	prefs.end();

	// Initialize button
	button.begin();
	button.setShortPressHandler(shortPressAction);
	button.setLongPressHandler(longPressAction);

	// Show splashscreen and Slotscreen
	showSplashscreen();
	showSlotscreen();

	timerCheckSlotState.attach(0.25, handleTimerInterrupt);
}


void loop() {
	button.check();
	if(slotNumberStateChanged >= 0) {
		if(sensorSlotState[slotNumberStateChanged - 1]){
			printTextForSlot(slotNumberStateChanged, "geladen");
			// notify about placed
		} else {
			printTextForSlot(slotNumberStateChanged, "leer");
			// notify about removal
		}
		slotNumberStateChanged = -1;
	}

	// 250ms timer for checking if device was placed or removed from a slot
	if (timeToCheckSlot) {
		timeToCheckSlot = false;
		// Checks whether a mobile phone has been placed into or removed from one of the slots
		volatile uint32_t sensorValue;
		volatile bool newState;
		for (int i= 0; i < 5; i++) {
			sensorValue = analogReadMilliVolts(sensorSlotPin[i]);
			newState = (sensorValue < 1500) ? true : false;
			if (sensorSlotState[i] != newState) {
				sensorSlotState[i] = newState;
				slotNumberStateChanged = i + 1;
			}

		}
	}
}