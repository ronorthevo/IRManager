// ============================================================
//  IRManager — src/main.cpp
//  Punto de entrada del firmware.
//
//  v0.2: Storage LittleFS operativo. Flujo learn completo:
//        consola → PendingLearn → captura IR → Storage.
//
//  ARQUITECTURA DE DEPENDENCIAS (sin cambios desde v0.1):
//    main.cpp es el único coordinador. Los módulos no se
//    conocen entre sí. No hay dependencias circulares.
// ============================================================

#define ARDUINO_USB_MODE        1
#define ARDUINO_USB_CDC_ON_BOOT 1

#include <Arduino.h>

#include "Config.h"
#include "IRSignal.h"
#include "Receiver.h"
#include "Sender.h"
#include "Storage.h"
#include "Console.h"
#include "WifiManager.h"
#include "WebServer.h"
#include "Api.h"

// ─────────────────────────────────────────────────────────────
//  Instancias de módulos
// ─────────────────────────────────────────────────────────────
static Receiver    receiver(IR_RECV_PIN);
static Sender      sender(IR_SEND_PIN);
static Storage     storage;
static WifiManager wifiManager;
static WebServer   webServer(80);
static Console     console(&storage, &receiver, &sender, &wifiManager);

// ─────────────────────────────────────────────────────────────
//  Helpers de impresión IR
// ─────────────────────────────────────────────────────────────
static void printSignal(const IRSignal& sig) {
    Serial.println(F("\n╔══════════════════════════════════════╗"));
    Serial.println(F("║          SEÑAL IR RECIBIDA           ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));

    Serial.print(F("  Protocolo : ")); Serial.println(sig.protocolName());
    Serial.print(F("  Valor     : ")); Serial.println(sig.valueHex());
    Serial.print(F("  Bits      : ")); Serial.println(sig.bits);

    if (!sig.isRawOnly()) {
        Serial.print(F("  Address   : 0x"));
        Serial.println((uint32_t)sig.address, HEX);
        Serial.print(F("  Command   : 0x"));
        Serial.println((uint32_t)sig.command, HEX);
    }

    Serial.print(F("  Frecuencia: "));
    Serial.print(sig.frequencyHz / 1000);
    Serial.println(F(" kHz"));

    Serial.print(F("  Raw ("));
    Serial.print(sig.rawData.size());
    Serial.print(F(" muestras): "));

    constexpr uint8_t MAX_RAW_PRINT = 16;
    const size_t count = min(sig.rawData.size(), (size_t)MAX_RAW_PRINT);
    for (size_t i = 0; i < count; ++i) {
        Serial.print(sig.rawData[i]);
        if (i < count - 1) Serial.print(F(", "));
    }
    if (sig.rawData.size() > MAX_RAW_PRINT) Serial.print(F(" ..."));
    Serial.println();
    Serial.println(F("──────────────────────────────────────────\n"));
}

// ─────────────────────────────────────────────────────────────
//  setup()
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(SERIAL_BAUD);
    const unsigned long t0 = millis();
    while (!Serial && (millis() - t0 < SERIAL_TIMEOUT_MS)) {
        delay(10);
    }

    console.printBanner();

    // ── Storage (LittleFS) ───────────────────────────────────
    Serial.println(F("[Storage] Montando LittleFS..."));
    if (!storage.begin()) {
        Serial.println(F("[Storage] ADVERTENCIA: LittleFS no disponible."));
        Serial.println(F("[Storage] Los botones no se podrán guardar."));
    }

    // ── Receptor IR ─────────────────────────────────────────
    Serial.printf("[Receiver] Iniciando en GPIO %d...\n", IR_RECV_PIN);
    receiver.begin();
    Serial.println(F("[Receiver] OK — Escuchando señales IR."));

    // Emisor IR — begin() reserva el canal RMT del GPIO.
    // El LED IR no necesita estar físicamente conectado para inicializar.
    Serial.printf("[Sender] Iniciando en GPIO %d...\n", IR_SEND_PIN);
    sender.begin();

    // [STUB v0.6] webServer.begin()

    // WiFi AP+STA con portal cautivo
    wifiManager.begin();

    Serial.println();
    Serial.println(F("[IRManager] Sistema listo."));
    Serial.println(F("[IRManager] Escribe 'help' para ver los comandos.\n"));
}

// ─────────────────────────────────────────────────────────────
//  loop()
// ─────────────────────────────────────────────────────────────
void loop() {
    // ── 1. Consola serie ─────────────────────────────────────
    console.loop();

    // ── 2. Receptor IR ───────────────────────────────────────
    const IRSignal signal = receiver.loop();

    if (signal.valid) {
        const PendingLearn& pending = console.getPendingLearn();

        if (pending.active) {
            // ── Flujo learn: guardar la señal capturada ──────
            const bool saved = storage.saveButton(
                pending.device, pending.button, signal);

            if (saved) {
                console.notifyLearnSuccess(pending.device, pending.button);
            } else {
                console.notifyLearnFailed(pending.device, pending.button);
            }

            console.clearPendingLearn();

        } else {
            // ── Modo monitor: solo imprimir ──────────────────
            printSignal(signal);
        }

        // Reanudar escucha en ambos casos
        receiver.resume();
    }

    // ── 3. WiFi ───────────────────────────────────────
    wifiManager.loop();
    // [STUB v0.6] webServer implicitly handled by AsyncWebServer
}
