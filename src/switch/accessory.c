#include <Arduino.h>
#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <stdio.h>
#include <port.h>

#define PIN_SWITCH 2         // D4 Output to trigger on/off
#define PIN_LED_BUILD_IN 16  // Build in LED 

bool switch_power = false;

void accessory_init() {
	pinMode(PIN_SWITCH, OUTPUT);
    switch_off(); // Default off
}

void builtInLed_set_status(bool on) {
	digitalWrite(PIN_LED_BUILD_IN, on ? LOW : HIGH);
}

void builtInLed_blink(int intervalInMs, int count) {
	for (int i = 0; i < count; i++) {
		builtInLed_set_status(true);
		delay(intervalInMs);
		builtInLed_set_status(false);
		delay(intervalInMs);
	}
}

void accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
    builtInLed_blink(500, 3);
}

homekit_value_t switch_on_get() {
	return HOMEKIT_BOOL(switch_power);
}

void switch_on_set(homekit_value_t value) {
	if (value.format != homekit_format_bool) {
		printf("Invalid on-value format: %d\n", value.format);
		return;
	}

    switch_power = value.bool_value; // Sync value

	if (switch_power) {
		printf("ON\n");
		digitalWrite(PIN_SWITCH, LOW);
	} else {
		printf("OFF\n");
		digitalWrite(PIN_SWITCH, HIGH);
	}
}

homekit_characteristic_t switcher = HOMEKIT_CHARACTERISTIC_(ON, true,
    .getter=switch_on_get,
	.setter=switch_on_set
);

void switch_on() {
    switcher.value.bool_value = true;
	switcher.setter(switcher.value);
	homekit_characteristic_notify(&switcher, switcher.value);
}

void switch_off() {
    switcher.value.bool_value = false;
	switcher.setter(switcher.value);
	homekit_characteristic_notify(&switcher, switcher.value);
}

void switch_toggle() {
	// switcher.value.bool_value = !switcher.value.bool_value;
	switcher.value.bool_value = !switch_power;
	switcher.setter(switcher.value);
	homekit_characteristic_notify(&switcher, switcher.value);
}

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Switch"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Arduino HomeKit"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
            NULL
		}),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &switcher,
			HOMEKIT_CHARACTERISTIC(NAME, "Switch"),
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111",
    .setupId = "ABCD"
};
