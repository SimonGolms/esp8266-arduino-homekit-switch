#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <ESP8266WebServer.h>
#include "ESPButton.h"
#include <DNSServer.h>
#include <WiFiManager.h>	// https://github.com/tzapu/WiFiManager

extern "C" homekit_server_config_t config;
extern "C" void accessory_init();
extern "C" void switch_toggle();

#define PIN_BUTTON 14 		// D5 Button
#define PIN_BUTTON_FLASH 0 	// D3 Flash-Button of NodeMCU

void setup() {
	Serial.begin(115200);
	
	// WiFiManager
	WiFiManager wifiManager;
	wifiManager.setConfigPortalTimeout(180);  // in seconds
	if(!wifiManager.autoConnect("ESP_Switch_AP")) {
    	printf("Failed to connect and hit timeout");
	} 

	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	homekit_setup();
}

void loop() {
	homekit_loop();
	delay(10);
}


void homekit_setup() {
	accessory_init();

	pinMode(PIN_BUTTON, INPUT_PULLUP);
	pinMode(PIN_BUTTON_FLASH, INPUT_PULLUP);
	
	ESPButton.add(0, PIN_BUTTON, LOW, true, true);
	ESPButton.add(1, PIN_BUTTON_FLASH, LOW, true, true);
	
	ESPButton.setCallback([&](uint8_t id, ESPButtonEvent event) {
		printf("Button %d Event: %s\n", id, ESPButton.getButtonEventDescription(event));
		
		if (event == ESPBUTTONEVENT_SINGLECLICK) {
			switch_toggle();
		} else if (event == ESPBUTTONEVENT_DOUBLECLICK) {
			// Free to configure
		} else if (event == ESPBUTTONEVENT_LONGCLICK) {
			printf("Rebooting...\n");
			homekit_storage_reset();
			ESP.restart(); // or system_restart();
		}
	});

	ESPButton.begin();

	arduino_homekit_setup(&config);
}

static uint32_t next_heap_millis = 0;

void homekit_loop() {
	ESPButton.loop();
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// Show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		printf("Free heap: %d, HomeKit clients: %d\n",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}
