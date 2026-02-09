#include <Arduino.h>
#include <Ticker.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <oled.h>
#include <Rotary.h>
#include <Menu.h>

Preferences prefs;

// Pins und Staus der Drucksensoren der Slots
const int sensorSlotPin[] = {0, 1, 2, 3, 4};
bool sensorSlotState[] = {false, false, false, false, false};

// FLAGS für ISR
// speichert die Nummer des Slots, wo eine Änderung erfolgt ist (laden/entnehmen)
volatile int slotNumberStateChanged = -1;

// zeigt an, ob der Timer für die Slot-Überprüfung abgelaufen ist
volatile bool timeToCheckSlot = false;

// Configuration parameters
String garageId;
String wifi_ssid;
String wifi_password;

Ticker timerCheckSlotState;

// called every 250ms by Timer to check if State of any of the five Slot is changed
void IRAM_ATTR handleTimerInterrupt() {
	timeToCheckSlot = true;
}

void setup() {
	// Initialize Slotsensors
	for (int i = 0; i < 5; i++) {
		pinMode(sensorSlotPin[i], INPUT);
	}

	// Initialize I2C for ESP32-C6 (SDA=6, SCL=7)
	Wire.begin(6, 7);

	// Initialize OLED display
	initializeOLED();

	// Initialize preferences store and get stored/default values
	prefs.begin("handygarage", false);
	garageId = (prefs.isKey("garageId")) ? prefs.getString("garageId") : "";
	wifi_ssid = (prefs.isKey("wifi_ssid")) ? prefs.getString("wifi_ssid") : "Trireme";
	wifi_password = (prefs.isKey("wifi_password")) ? prefs.getString("wifi_password") : "tri++reme";
	prefs.end();

	// Initialize Rotary Encoder
	initializeRotary();

	// Show splashscreen and Slotscreen
	showSplashscreen();
	showSlotscreen();

	timerCheckSlotState.attach(0.25, handleTimerInterrupt);

	// Initialize Menu
	initializeMenu();

}


void loop() {
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