#include <Arduino.h>
#include "Zigbee.h"

// Forward declaration of callback functions
void ledChangeCallback1(bool value);
void valveChangeCallback1(bool value);
void valveChangeCallback2(bool value);
void valveChangeCallback3(bool value);
void valveChangeCallback4(bool value);

// Define the ZigbeeValveController class
class ZigbeeValveController {
private:
    ZigbeeLight valve;
    uint8_t ledPin;
    unsigned long valveTimeout;
    unsigned long valveOnMillis;
    void (*callback)(bool); // Pointer to callback function

public:
    // Constructor
    ZigbeeValveController(uint8_t endpoint, uint8_t ledPin, unsigned long timeout, void (*cb)(bool))
        : valve(endpoint), ledPin(ledPin), valveTimeout(timeout), valveOnMillis(0), callback(cb) {}

    // Method to initialize the Zigbee valve
    void init() {
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);

        // Optional: set Zigbee device name and model
        valve.setManufacturerAndModel("Espressif", "ZBValve");

        // Set the callback function for valve change
        valve.onLightChange(callback);

        // Add endpoint to Zigbee Core
        Zigbee.addEndpoint(&valve);
    }

    // Method to check the valve timeout
    void checkTimeout() {
        if (valve.getLightState()) {
            if (millis() - valveOnMillis >= valveTimeout || 
                (millis() < valveOnMillis && (ULONG_MAX - valveOnMillis + millis()) >= valveTimeout)) {
                valve.setLight(false);
                valveOnMillis = 0;
            }
        }
    }

    ZigbeeLight& getValve() {
        return valve;
    }

    // Getter and setter for valveOnMillis
    unsigned long getValveOnMillis() {
        return valveOnMillis;
    }

    void setValveOnMillis(unsigned long millis) {
        valveOnMillis = millis;
    }
};

// Configuration
#define ZIGBEE_LED1_ENDPOINT 10
#define ZIGBEE_VALVE1_ENDPOINT 11
#define ZIGBEE_VALVE2_ENDPOINT 12
#define ZIGBEE_VALVE3_ENDPOINT 13
#define ZIGBEE_VALVE4_ENDPOINT 14
uint8_t led1 = RGB_BUILTIN;
uint8_t valve1 = 15;
uint8_t valve2 = 23;
uint8_t valve3 = 22;
uint8_t valve4 = 21;
uint8_t button = BOOT_PIN;
unsigned long valveTimeout = 1000*60*10; // 10 minutes in milliseconds

// Create ZigbeeValveController instances
ZigbeeValveController zbLight(ZIGBEE_LED1_ENDPOINT, led1, valveTimeout, ledChangeCallback1);
ZigbeeValveController zbValve1(ZIGBEE_VALVE1_ENDPOINT, valve1, valveTimeout, valveChangeCallback1);
ZigbeeValveController zbValve2(ZIGBEE_VALVE2_ENDPOINT, valve2, valveTimeout, valveChangeCallback2);
ZigbeeValveController zbValve3(ZIGBEE_VALVE3_ENDPOINT, valve3, valveTimeout, valveChangeCallback3);
ZigbeeValveController zbValve4(ZIGBEE_VALVE4_ENDPOINT, valve4, valveTimeout, valveChangeCallback4);

void ledChangeCallback1(bool value) {
    digitalWrite(led1, value);
    if (value) {
        zbLight.setValveOnMillis(millis());
    } else {
        zbLight.setValveOnMillis(0);
    }
}

void valveChangeCallback1(bool value) {
    digitalWrite(valve1, value);
    if (value) {
        zbValve2.setValveOnMillis(millis());
    } else {
        zbValve2.setValveOnMillis(0);
    }
}

void valveChangeCallback2(bool value) {
    digitalWrite(valve2, value);
    if (value) {
        zbValve2.setValveOnMillis(millis());
    } else {
        zbValve2.setValveOnMillis(0);
    }
}

void valveChangeCallback3(bool value) {
    digitalWrite(valve3, value);
    if (value) {
        zbValve3.setValveOnMillis(millis());
    } else {
        zbValve3.setValveOnMillis(0);
    }
}

void valveChangeCallback4(bool value) {
    digitalWrite(valve4, value);
    if (value) {
        zbValve4.setValveOnMillis(millis());
    } else {
        zbValve4.setValveOnMillis(0);
    }
}

void setup() {
    Serial.begin(115200);

    // Init button for factory reset
    pinMode(button, INPUT_PULLUP);

    // Initialize the Zigbee valve
    zbLight.init();
    zbValve1.init();
    zbValve2.init();
    zbValve3.init();
    zbValve4.init();

    // Start Zigbee
    Serial.println("Adding ZigbeeValve endpoint to Zigbee Core");
    if (!Zigbee.begin()) {
        Serial.println("Zigbee failed to start!");
        Serial.println("Rebooting...");
        ESP.restart();
    }
    Serial.println("Connecting to network");
    while (!Zigbee.connected()) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
}

void loop() {
    // Check button for factory reset
    if (digitalRead(button) == LOW) {
        delay(100);
        int startTime = millis();
        while (digitalRead(button) == LOW) {
            delay(50);
            if ((millis() - startTime) > 3000) {
                Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
                delay(1000);
                Zigbee.factoryReset();
            }
        }
        // Toggle valve by pressing the button
        zbLight.getValve().setLight(!zbLight.getValve().getLightState());
    }

    // Check the valve timeout
    zbLight.checkTimeout();
    zbValve1.checkTimeout();
    zbValve2.checkTimeout();
    zbValve3.checkTimeout();
    zbValve4.checkTimeout();

    delay(100);  // Add a small delay to prevent overloading the processor
}
