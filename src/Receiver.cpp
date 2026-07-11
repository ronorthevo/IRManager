#include "Receiver.h"

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

static const uint16_t RECV_PIN = 15;

IRrecv irrecv(RECV_PIN);
decode_results results;

void Receiver::begin()
{
    irrecv.enableIRIn();
}

bool Receiver::learn(const String &device, const String &button)
{
    Serial.println();
    Serial.println("======================");
    Serial.println("Esperando señal IR...");
    Serial.println("======================");

    while (true)
    {
        if (irrecv.decode(&results))
        {
            Serial.println();

            Serial.print("Dispositivo : ");
            Serial.println(device);

            Serial.print("Botón       : ");
            Serial.println(button);

            Serial.print("Protocolo   : ");
            Serial.println(typeToString(results.decode_type));

            Serial.print("Bits        : ");
            Serial.println(results.bits);

            Serial.print("Valor       : ");
            Serial.println(resultToHexidecimal(&results));

            Serial.println();
            Serial.println(resultToSourceCode(&results));

            irrecv.resume();

            return true;
        }

        delay(1);
    }
}