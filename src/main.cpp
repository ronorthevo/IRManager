// ============================================================
//  IRManager — src/main.cpp
//  Punto de entrada del firmware.
//
//  v0.1: Receptor IR funcional + consola serie básica.
//        Todos los módulos instanciados y conectados.
//        Los módulos marcados [STUB] compilan pero no ejecutan
//        lógica real hasta su versión correspondiente.
//
//  ARQUITECTURA DE DEPENDENCIAS (v0.1):
//
//    main.cpp
//      ├── Receiver  (captura IR)
//      ├── Sender    [STUB v0.3]
//      ├── Storage   [STUB v0.2]
//      ├── Console   (banner + help)
//      ├── WifiManager [STUB v0.5]
//      ├── WebServer   [STUB v0.6]
//      └── Api         [STUB v0.5]
//
//  No hay dependencias circulares. main.cpp es el único
//  coordinador. Los módulos no se conocen entre sí.
// ============================================================

// ---- USB-CDC nativo del ESP32-S3 ---------------------------
// Estas macros deben ir ANTES de cualquier include de Arduino.
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
//  Instancias de módulos (lifetime = duración del programa)
// ─────────────────────────────────────────────────────────────
static Receiver    receiver(IR_RECV_PIN);
static Sender      sender(IR_SEND_PIN);
static Storage     storage;
static WifiManager wifiManager;
static WebServer   webServer(80);

// Console necesita punteros a los tres módulos de negocio
static Console     console(&storage, &receiver, &sender);

// Api no se instancia aquí en v0.1 (no hay servidor activo).
// Se instanciará en v0.5 cuando WebServer esté operativo.

// ─────────────────────────────────────────────────────────────
//  Helpers de impresión IR
// ─────────────────────────────────────────────────────────────

// Imprime un IRSignal por el puerto serie de forma legible.
static void printSignal(const IRSignal& sig) {
    Serial.println(F("\n╔══════════════════════════════════════╗"));
    Serial.println(F("║          SEÑAL IR RECIBIDA           ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));

    Serial.print(F("  Protocolo : "));
    Serial.println(sig.protocolName());

    Serial.print(F("  Valor     : "));
    Serial.println(sig.valueHex());

    Serial.print(F("  Bits      : "));
    Serial.println(sig.bits);

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

    // Imprime solo las primeras 16 muestras para no saturar el serie
    constexpr uint8_t MAX_RAW_PRINT = 16;
    const size_t count = min(sig.rawData.size(), (size_t)MAX_RAW_PRINT);
    for (size_t i = 0; i < count; ++i) {
        Serial.print(sig.rawData[i]);
        if (i < count - 1) Serial.print(F(", "));
    }
    if (sig.rawData.size() > MAX_RAW_PRINT) {
        Serial.print(F(" ..."));
    }
    Serial.println();
    Serial.println(F("──────────────────────────────────────────\n"));
}

// ─────────────────────────────────────────────────────────────
//  setup()
// ─────────────────────────────────────────────────────────────
void setup() {
    // ---- Puerto serie USB-CDC ----------------------------------
    Serial.begin(SERIAL_BAUD);

    // El USB-CDC del S3 necesita un momento para estar disponible.
    // Esperamos máximo SERIAL_TIMEOUT_MS milisegundos.
    const unsigned long t0 = millis();
    while (!Serial && (millis() - t0 < SERIAL_TIMEOUT_MS)) {
        delay(10);
    }

    // ---- Banner de bienvenida ---------------------------------
    console.printBanner();

    // ---- Inicialización de módulos ---------------------------
    // NOTA: Storage y WifiManager se inicializarán en sus versiones.
    // Se incluye la llamada aquí para que la estructura sea correcta
    // desde el principio.

    Serial.print(F("[Receiver] Iniciando en GPIO "));
    Serial.print(IR_RECV_PIN);
    Serial.println(F("..."));
    receiver.begin();
    Serial.println(F("[Receiver] OK — Escuchando señales IR.\n"));

    // [STUB v0.2] storage.begin()
    // [STUB v0.5] wifiManager.begin()
    // [STUB v0.6] webServer.begin()

    Serial.println(F("[IRManager] Sistema listo. Apunta un mando al receptor.\n"));
}

// ─────────────────────────────────────────────────────────────
//  loop()
// ─────────────────────────────────────────────────────────────
void loop() {
    // ── 1. Consola serie ──────────────────────────────────────
    console.loop();

    // ── 2. Receptor IR ───────────────────────────────────────
    IRSignal signal = receiver.loop();

    if (signal.valid) {
        // Imprime la señal capturada
        printSignal(signal);

        // TODO v0.2: preguntar si guardar y llamar storage.saveButton()
        // TODO v0.3: si venía de un comando 'learn', guardar automáticamente

        // Reanuda la escucha
        receiver.resume();
    }

    // ── 3. Módulos futuros ───────────────────────────────────
    // [STUB v0.5] wifiManager.loop()
}
