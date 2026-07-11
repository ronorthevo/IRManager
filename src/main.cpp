#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1

#include <Arduino.h>

void setup() {
    Serial.begin(115200);

    // Espera a que el puerto serie USB esté listo (máximo 5 s)
    unsigned long t0 = millis();
    while (!Serial && (millis() - t0 < 5000)) {
        delay(10);
    }

    Serial.println();
    Serial.println("================================");
    Serial.println(" ESP32-S3 FUNCIONANDO");
    Serial.println("================================");
}

void loop() {
    Serial.printf("Tiempo: %lu ms\n", millis());
    delay(1000);
}