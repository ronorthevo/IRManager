// ============================================================
//  IRManager — src/main.cpp
//  Punto de entrada del firmware.
//
//  v0.5: WebServer + REST API activos.
//        El flujo learn acepta peticiones desde consola serie
//        Y desde el endpoint POST /api/learn.
//
//  ARQUITECTURA DE DEPENDENCIAS:
//    main.cpp es el único coordinador. No hay dependencias
//    circulares. main.cpp es el único que llama a múltiples
//    módulos en secuencia.
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
//  Orden: módulos sin dependencias primero.
// ─────────────────────────────────────────────────────────────
static Receiver    receiver(IR_RECV_PIN);
static Sender      sender(IR_SEND_PIN);
static Storage     storage;
static WifiManager wifiManager;
static WebServer   webServer(80);
static Console     console(&storage, &receiver, &sender, &wifiManager);

// Api se instancia tras webServer (necesita el AsyncWebServer*)
// Se usa un puntero para permitir inicialización diferida.
static Api*        api = nullptr;

// ─────────────────────────────────────────────────────────────
//  Helper: imprimir señal IR en monitor serie
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
//  Helper: procesar señal IR capturada
//  Comprueba si hay un learn pendiente (desde consola o API)
//  y actúa en consecuencia.
// ─────────────────────────────────────────────────────────────
static void handleSignal(const IRSignal& signal) {
    // ── Prioridad 1: learn desde consola serie ───────────────
    const PendingLearn& consolePending = console.getPendingLearn();
    if (consolePending.active) {
        const bool ok = storage.saveButton(
            consolePending.device, consolePending.button, signal);
        if (ok) console.notifyLearnSuccess(consolePending.device, consolePending.button);
        else    console.notifyLearnFailed (consolePending.device, consolePending.button);
        console.clearPendingLearn();
        return;
    }

    // ── Prioridad 2: learn desde REST API ───────────────────
    if (api != nullptr) {
        const ApiPendingLearn& apiPending = api->getPendingLearn();
        if (apiPending.active) {
            const String device(apiPending.device);
            const String button(apiPending.button);
            const bool ok = storage.saveButton(device, button, signal);
            api->notifyLearnResult(ok);
            api->clearPendingLearn();
            Serial.printf("[IRManager] Learn API: %s → %s / %s\n",
                          ok ? "OK" : "ERROR", device.c_str(), button.c_str());
            return;
        }
    }

    // ── Sin learn pendiente: modo monitor ───────────────────
    printSignal(signal);
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

    // ── LittleFS ────────────────────────────────────────────
    Serial.println(F("[Storage] Montando LittleFS..."));
    if (!storage.begin()) {
        Serial.println(F("[Storage] ADVERTENCIA: LittleFS no disponible."));
    }

    // ── Receptor IR ─────────────────────────────────────────
    Serial.printf("[Receiver] Iniciando en GPIO %d...\n", IR_RECV_PIN);
    receiver.begin();
    Serial.println(F("[Receiver] OK — Escuchando señales IR."));

    // ── Emisor IR ───────────────────────────────────────────
    Serial.printf("[Sender] Iniciando en GPIO %d...\n", IR_SEND_PIN);
    sender.begin();

    // ── WiFi ────────────────────────────────────────────────
    wifiManager.begin();

    // ── WebServer ───────────────────────────────────────────
    webServer.begin();

    // ── REST API ────────────────────────────────────────────
    // Instanciamos Api aquí (post-WiFi, post-WebServer)
    api = new Api(webServer.server(), &storage,
                  &receiver, &sender, &wifiManager);
    api->registerRoutes();

    Serial.println();
    Serial.println(F("[IRManager] Sistema listo."));
    if (wifiManager.isConnected()) {
        Serial.printf("[IRManager] Interfaz web: http://%s\n",
                      wifiManager.localIP().c_str());
    } else if (wifiManager.isAP()) {
        Serial.printf("[IRManager] Portal: http://%s  (AP: %s)\n",
                      wifiManager.apIP().c_str(), WifiConfig::AP_SSID);
    }
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
        handleSignal(signal);
        receiver.resume();
    }

    // ── 3. WiFi ──────────────────────────────────────────────
    wifiManager.loop();

    // AsyncWebServer no requiere loop() — maneja conexiones
    // en su propio task de FreeRTOS.
}
