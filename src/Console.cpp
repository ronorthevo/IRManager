// ============================================================
//  IRManager — src/Console.cpp
//  [STUB v0.1] — Banner funcional. Comandos completos en v0.4.
// ============================================================
#include "Console.h"
#include "Config.h"

Console::Console(Storage* storage, Receiver* receiver, Sender* sender)
    : _storage(storage)
    , _receiver(receiver)
    , _sender(sender)
{
}

void Console::printBanner() {
    Serial.println();
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║         I R M A N A G E R            ║"));
    Serial.print  (F("║  Firmware: "));
    Serial.print  (FW_VERSION);
    Serial.println(F("                       ║"));
    Serial.println(F("║  ESP32-S3 · PlatformIO · Arduino     ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println();
    Serial.println(F("Escribe 'help' para ver los comandos disponibles."));
    Serial.println();
}

void Console::loop() {
    while (Serial.available()) {
        char c = static_cast<char>(Serial.read());

        if (c == '\r') continue;   // ignorar CR (Windows)

        if (c == '\n') {
            _inputBuffer.trim();
            if (_inputBuffer.length() > 0) {
                _dispatch(_inputBuffer);
            }
            _inputBuffer = "";
        } else {
            _inputBuffer += c;
        }
    }
}

void Console::_dispatch(const String& line) {
    // TODO v0.4: CommandDispatcher completo
    if (line.equalsIgnoreCase("help")) {
        _printHelp();
    } else {
        Serial.print(F("[Console] Comando no reconocido: "));
        Serial.println(line);
        Serial.println(F("Escribe 'help' para ver los comandos disponibles."));
    }
}

void Console::_printHelp() {
    Serial.println(F("\n---- Comandos disponibles ----"));
    Serial.println(F("  learn <dispositivo> <boton>   — Aprende una señal IR"));
    Serial.println(F("  send  <dispositivo> <boton>   — Emite una señal IR"));
    Serial.println(F("  list                          — Lista todos los botones"));
    Serial.println(F("  devices                       — Lista dispositivos"));
    Serial.println(F("  buttons <dispositivo>         — Lista botones del dispositivo"));
    Serial.println(F("  delete <dispositivo> <boton>  — Borra un botón"));
    Serial.println(F("  export <dispositivo>          — Exporta dispositivo"));
    Serial.println(F("  import <dispositivo>          — Importa dispositivo"));
    Serial.println(F("  wifi                          — Estado WiFi"));
    Serial.println(F("  status                        — Estado del sistema"));
    Serial.println(F("  restart                       — Reinicia el ESP32"));
    Serial.println(F("  help                          — Esta ayuda"));
    Serial.println(F("------------------------------\n"));
}
