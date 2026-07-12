// ============================================================
//  IRManager — src/Console.cpp
//  v0.2 — Implementación completa de comandos disponibles.
// ============================================================
#include "Console.h"
#include "Config.h"
#include "Storage.h"
#include "Receiver.h"
#include "Sender.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
Console::Console(Storage* storage, Receiver* receiver, Sender* sender)
    : _storage(storage)
    , _receiver(receiver)
    , _sender(sender)
{
}

// ─────────────────────────────────────────────────────────────
//  printBanner()
// ─────────────────────────────────────────────────────────────
void Console::printBanner() {
    Serial.println();
    Serial.println(F("╔══════════════════════════════════════╗"));
    Serial.println(F("║         I R M A N A G E R            ║"));
    Serial.print  (F("║  Firmware : "));
    Serial.print  (FW_VERSION);
    Serial.println(F("                      ║"));
    Serial.println(F("║  ESP32-S3 · PlatformIO · Arduino     ║"));
    Serial.println(F("╚══════════════════════════════════════╝"));
    Serial.println();
}

// ─────────────────────────────────────────────────────────────
//  loop()
//  Lee el puerto serie byte a byte, construye líneas completas
//  y las despacha al handler correspondiente.
// ─────────────────────────────────────────────────────────────
void Console::loop() {
    while (Serial.available()) {
        const char c = static_cast<char>(Serial.read());

        if (c == '\r') continue;   // ignorar CR de terminales Windows

        if (c == '\n') {
            _inputBuffer.trim();
            if (_inputBuffer.length() > 0) {
                Serial.print(F("> "));
                Serial.println(_inputBuffer);
                _dispatch(_inputBuffer);
            }
            _inputBuffer = "";
        } else {
            if (_inputBuffer.length() < 128) {   // límite de seguridad
                _inputBuffer += c;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  notifyLearnSuccess() / notifyLearnFailed()
//  Llamados desde main.cpp después de intentar guardar la señal.
// ─────────────────────────────────────────────────────────────
void Console::notifyLearnSuccess(const String& device, const String& button) {
    Serial.printf("[OK] Señal guardada → %s / %s\n",
                  device.c_str(), button.c_str());
}

void Console::notifyLearnFailed(const String& device, const String& button) {
    Serial.printf("[ERROR] No se pudo guardar → %s / %s\n",
                  device.c_str(), button.c_str());
}

// ─────────────────────────────────────────────────────────────
//  _dispatch()  [private]
// ─────────────────────────────────────────────────────────────
void Console::_dispatch(const String& line) {
    String args;
    const String cmd = _firstToken(line, args);

    if      (cmd.equalsIgnoreCase("learn"))   _cmdLearn(args);
    else if (cmd.equalsIgnoreCase("send"))    _cmdSend(args);
    else if (cmd.equalsIgnoreCase("list"))    _cmdList();
    else if (cmd.equalsIgnoreCase("devices")) _cmdDevices();
    else if (cmd.equalsIgnoreCase("buttons")) _cmdButtons(args);
    else if (cmd.equalsIgnoreCase("delete"))  _cmdDelete(args);
    else if (cmd.equalsIgnoreCase("status"))  _cmdStatus();
    else if (cmd.equalsIgnoreCase("restart")) _cmdRestart();
    else if (cmd.equalsIgnoreCase("help"))    _cmdHelp();
    else {
        Serial.printf("[Console] Comando desconocido: '%s'  (escribe 'help')\n",
                      cmd.c_str());
    }
}

// ─────────────────────────────────────────────────────────────
//  _cmdLearn()
//  Uso: learn <dispositivo> <boton>
// ─────────────────────────────────────────────────────────────
void Console::_cmdLearn(const String& args) {
    if (_pending.active) {
        Serial.println(F("[learn] Ya hay una captura en curso. Espera o reinicia."));
        return;
    }

    String rest;
    const String device = _firstToken(args, rest);
    String button = rest;
    button.trim();

    if (device.isEmpty() || button.isEmpty()) {
        Serial.println(F("[learn] Uso: learn <dispositivo> <boton>"));
        Serial.println(F("        Ejemplo: learn Samsung POWER"));
        return;
    }

    _pending.active = true;
    _pending.device = device;
    _pending.button = button;

    // Asegura que el receptor esté escuchando
    _receiver->resume();

    Serial.printf("\n[learn] Apunta el mando al receptor y pulsa el botón '%s'...\n",
                  button.c_str());
    Serial.println(F("[learn] Esperando señal IR (timeout: ninguno, reinicia para cancelar)"));
}

// ─────────────────────────────────────────────────────────────
//  _cmdSend()
//  Uso: send <dispositivo> <boton>
//  [STUB] Implementación completa en v0.3
// ─────────────────────────────────────────────────────────────
void Console::_cmdSend(const String& args) {
    String rest;
    const String device = _firstToken(args, rest);
    String button = rest;
    button.trim();

    if (device.isEmpty() || button.isEmpty()) {
        Serial.println(F("[send] Uso: send <dispositivo> <boton>"));
        Serial.println(F("        Ejemplo: send Samsung POWER"));
        return;
    }

    const IRSignal signal = _storage->loadButton(device, button);
    if (!signal.valid) {
        Serial.printf("[send] Botón no encontrado: %s / %s\n",
                      device.c_str(), button.c_str());
        return;
    }

    Serial.printf("[send] Emitiendo %s / %s (protocolo: %s)...\n",
                  device.c_str(), button.c_str(),
                  signal.protocolName().c_str());

    if (_sender->send(signal)) {
        Serial.println(F("[send] OK"));
    } else {
        Serial.println(F("[send] ERROR: la emisión falló."));
    }
}

// ─────────────────────────────────────────────────────────────
//  _cmdList()
//  Uso: list
// ─────────────────────────────────────────────────────────────
void Console::_cmdList() {
    const auto all = _storage->listAll();

    if (all.empty()) {
        Serial.println(F("[list] No hay botones guardados."));
        return;
    }

    Serial.printf("\n[list] %u botones guardados:\n", all.size());
    Serial.println(F("  ┌──────────────────────┬──────────────────────┐"));
    Serial.println(F("  │ Dispositivo          │ Botón                │"));
    Serial.println(F("  ├──────────────────────┼──────────────────────┤"));
    for (const auto& ref : all) {
        Serial.printf("  │ %-20s │ %-20s │\n",
                      ref.device.c_str(), ref.button.c_str());
    }
    Serial.println(F("  └──────────────────────┴──────────────────────┘\n"));
}

// ─────────────────────────────────────────────────────────────
//  _cmdDevices()
//  Uso: devices
// ─────────────────────────────────────────────────────────────
void Console::_cmdDevices() {
    const auto devices = _storage->listDevices();

    if (devices.empty()) {
        Serial.println(F("[devices] No hay dispositivos guardados."));
        return;
    }

    Serial.printf("\n[devices] %u dispositivos:\n", devices.size());
    for (const String& dev : devices) {
        const auto btns = _storage->listButtons(dev);
        Serial.printf("  • %-20s  (%u botones)\n", dev.c_str(), btns.size());
    }
    Serial.println();
}

// ─────────────────────────────────────────────────────────────
//  _cmdButtons()
//  Uso: buttons <dispositivo>
// ─────────────────────────────────────────────────────────────
void Console::_cmdButtons(const String& args) {
    String device = args;
    device.trim();

    if (device.isEmpty()) {
        Serial.println(F("[buttons] Uso: buttons <dispositivo>"));
        return;
    }

    const auto buttons = _storage->listButtons(device);

    if (buttons.empty()) {
        Serial.printf("[buttons] No hay botones para '%s'.\n", device.c_str());
        return;
    }

    Serial.printf("\n[buttons] %s (%u botones):\n", device.c_str(), buttons.size());
    for (const String& btn : buttons) {
        Serial.printf("  • %s\n", btn.c_str());
    }
    Serial.println();
}

// ─────────────────────────────────────────────────────────────
//  _cmdDelete()
//  Uso: delete <dispositivo> <boton>
// ─────────────────────────────────────────────────────────────
void Console::_cmdDelete(const String& args) {
    String rest;
    const String device = _firstToken(args, rest);
    String button = rest;
    button.trim();

    if (device.isEmpty() || button.isEmpty()) {
        Serial.println(F("[delete] Uso: delete <dispositivo> <boton>"));
        return;
    }

    if (_storage->deleteButton(device, button)) {
        Serial.printf("[delete] Eliminado: %s / %s\n",
                      device.c_str(), button.c_str());
    } else {
        Serial.printf("[delete] Error al eliminar: %s / %s\n",
                      device.c_str(), button.c_str());
    }
}

// ─────────────────────────────────────────────────────────────
//  _cmdStatus()
// ─────────────────────────────────────────────────────────────
void Console::_cmdStatus() {
    const uint32_t uptime    = millis() / 1000;
    const uint32_t freeHeap  = ESP.getFreeHeap();
    const uint32_t totalHeap = ESP.getHeapSize();
    const size_t   fsTotal   = _storage->totalBytes();
    const size_t   fsFree    = _storage->freeBytes();

    Serial.println(F("\n──── Estado del sistema ────────────────────"));
    Serial.printf("  Firmware     : %s\n",    FW_VERSION);
    Serial.printf("  Uptime       : %lu s\n", uptime);
    Serial.printf("  Heap libre   : %lu / %lu KB\n",
                  freeHeap  / 1024, totalHeap / 1024);
    Serial.printf("  LittleFS     : %u / %u KB libres\n",
                  fsFree  / 1024, fsTotal / 1024);
    Serial.printf("  Receptor     : %s\n",
                  _receiver->isListening() ? "escuchando" : "suspendido");
    Serial.printf("  WiFi         : no configurado (v0.5)\n");
    Serial.println(F("────────────────────────────────────────────\n"));
}

// ─────────────────────────────────────────────────────────────
//  _cmdRestart()
// ─────────────────────────────────────────────────────────────
void Console::_cmdRestart() {
    Serial.println(F("[restart] Reiniciando en 1 segundo..."));
    delay(1000);
    ESP.restart();
}

// ─────────────────────────────────────────────────────────────
//  _cmdHelp()
// ─────────────────────────────────────────────────────────────
void Console::_cmdHelp() {
    Serial.println(F("\n──── Comandos disponibles ──────────────────"));
    Serial.println(F("  learn <dispositivo> <boton>  — Aprende señal IR y la guarda"));
    Serial.println(F("  send  <dispositivo> <boton>  — Emite señal IR"));
    Serial.println(F("  list                         — Lista todos los botones"));
    Serial.println(F("  devices                      — Lista dispositivos guardados"));
    Serial.println(F("  buttons <dispositivo>        — Botones de un dispositivo"));
    Serial.println(F("  delete <dispositivo> <boton> — Borra un botón"));
    Serial.println(F("  status                       — Estado del sistema"));
    Serial.println(F("  restart                      — Reinicia el ESP32"));
    Serial.println(F("  help                         — Esta ayuda"));
    Serial.println(F("────────────────────────────────────────────\n"));
}

// ─────────────────────────────────────────────────────────────
//  _firstToken()  [private, static]
//  Extrae el primer token separado por espacio.
//  Devuelve el token; "rest" recibe el resto del string.
// ─────────────────────────────────────────────────────────────
String Console::_firstToken(const String& line, String& rest) {
    const int spaceIdx = line.indexOf(' ');
    if (spaceIdx < 0) {
        rest = "";
        return line;
    }
    rest = line.substring(spaceIdx + 1);
    rest.trim();
    return line.substring(0, spaceIdx);
}
