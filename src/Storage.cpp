// ============================================================
//  IRManager — src/Storage.cpp
//  v0.2 — Implementación completa con LittleFS + ArduinoJson 7
// ============================================================
#include "Storage.h"

#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Config.h"

// ─────────────────────────────────────────────────────────────
//  begin()
// ─────────────────────────────────────────────────────────────
bool Storage::begin() {
    if (!LittleFS.begin(/* formatOnFail */ true)) {
        Serial.println(F("[Storage] ERROR: No se pudo montar LittleFS."));
        return false;
    }
    Serial.printf("[Storage] OK — Total: %u KB | Libre: %u KB\n",
                  totalBytes() / 1024,
                  freeBytes()  / 1024);
    return true;
}

// ─────────────────────────────────────────────────────────────
//  saveButton()
// ─────────────────────────────────────────────────────────────
bool Storage::saveButton(const String& device, const String& button,
                          const IRSignal& signal) {
    const String devSan = _sanitize(device);
    const String btnSan = _sanitize(button);
    const String dirPath = "/" + devSan;
    const String filePath = _buildPath(devSan, btnSan);

    // Crear directorio del dispositivo si no existe
    if (!LittleFS.exists(dirPath)) {
        if (!LittleFS.mkdir(dirPath)) {
            Serial.printf("[Storage] ERROR: No se pudo crear el directorio %s\n",
                          dirPath.c_str());
            return false;
        }
    }

    File file = LittleFS.open(filePath, "w");
    if (!file) {
        Serial.printf("[Storage] ERROR: No se pudo abrir para escritura: %s\n",
                      filePath.c_str());
        return false;
    }

    // ── Serialización JSON (ArduinoJson 7) ───────────────────
    JsonDocument doc;

    doc["schema_version"] = JSON_SCHEMA_VERSION;
    doc["device"]         = device;          // nombre original (no sanitizado)
    doc["button"]         = button;
    doc["protocol"]       = signal.protocolName();
    doc["value"]          = signal.valueHex();
    doc["bits"]           = signal.bits;
    doc["frequency"]      = signal.frequencyHz;

    // address y command solo si el protocolo los provee
    if (!signal.isRawOnly()) {
        doc["address"] = (uint32_t)signal.address;
        doc["command"] = (uint32_t)signal.command;
    }

    // Raw data completo (necesario para protocolos UNKNOWN y como
    // fallback de emisión si IRsend no soporta el protocolo)
    JsonArray raw = doc["raw"].to<JsonArray>();
    for (const uint16_t sample : signal.rawData) {
        raw.add(sample);
    }

    const size_t written = serializeJson(doc, file);
    file.close();

    if (written == 0) {
        Serial.printf("[Storage] ERROR: Escritura fallida en %s\n", filePath.c_str());
        return false;
    }

    Serial.printf("[Storage] Guardado: %s (%u bytes)\n", filePath.c_str(), written);
    return true;
}

// ─────────────────────────────────────────────────────────────
//  loadButton()
// ─────────────────────────────────────────────────────────────
IRSignal Storage::loadButton(const String& device, const String& button) {
    const String filePath = _buildPath(_sanitize(device), _sanitize(button));

    if (!LittleFS.exists(filePath)) {
        Serial.printf("[Storage] No encontrado: %s\n", filePath.c_str());
        return IRSignal{};
    }

    File file = LittleFS.open(filePath, "r");
    if (!file) {
        Serial.printf("[Storage] ERROR: No se pudo abrir: %s\n", filePath.c_str());
        return IRSignal{};
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.printf("[Storage] ERROR JSON en %s: %s\n",
                      filePath.c_str(), err.c_str());
        return IRSignal{};
    }

    IRSignal sig;
    sig.valid = true;

    // Protocolo — convierte string → decode_type_t
    const String protName = doc["protocol"] | "UNKNOWN";
    sig.protocol = strToDecodeType(protName.c_str());

    // Valor desde string hex "0x..."
    const String valueStr = doc["value"] | "0x0";
    sig.value = (uint64_t)strtoull(valueStr.c_str(), nullptr, 16);

    sig.bits        = doc["bits"]      | (uint16_t)0;
    sig.frequencyHz = doc["frequency"] | IR_DEFAULT_FREQ_HZ;
    sig.address     = doc["address"]   | (uint32_t)0;
    sig.command     = doc["command"]   | (uint32_t)0;

    // Raw data
    JsonArray raw = doc["raw"].as<JsonArray>();
    sig.rawData.reserve(raw.size());
    for (const uint16_t sample : raw) {
        sig.rawData.push_back(sample);
    }

    return sig;
}

// ─────────────────────────────────────────────────────────────
//  deleteButton()
// ─────────────────────────────────────────────────────────────
bool Storage::deleteButton(const String& device, const String& button) {
    const String devSan  = _sanitize(device);
    const String filePath = _buildPath(devSan, _sanitize(button));

    if (!LittleFS.exists(filePath)) {
        Serial.printf("[Storage] No existe: %s\n", filePath.c_str());
        return false;
    }

    if (!LittleFS.remove(filePath)) {
        Serial.printf("[Storage] ERROR borrando: %s\n", filePath.c_str());
        return false;
    }

    // Si el directorio del dispositivo quedó vacío, eliminarlo también
    const String dirPath = "/" + devSan;
    File dir = LittleFS.open(dirPath);
    if (dir) {
        File next = dir.openNextFile();
        const bool isEmpty = !next;
        if (next) next.close();
        dir.close();
        if (isEmpty) {
            LittleFS.rmdir(dirPath);
        }
    }

    Serial.printf("[Storage] Borrado: %s\n", filePath.c_str());
    return true;
}

// ─────────────────────────────────────────────────────────────
//  listDevices()
//  Devuelve todos los subdirectorios de la raíz LittleFS.
// ─────────────────────────────────────────────────────────────
std::vector<String> Storage::listDevices() {
    std::vector<String> devices;

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) return devices;

    File entry;
    while ((entry = root.openNextFile())) {
        if (entry.isDirectory()) {
            // entry.name() devuelve la ruta completa, extraemos solo el nombre
            const String fullPath = String(entry.name());
            const int lastSlash = fullPath.lastIndexOf('/');
            const String dirName = (lastSlash >= 0)
                                   ? fullPath.substring(lastSlash + 1)
                                   : fullPath;
            if (dirName.length() > 0) {
                devices.push_back(dirName);
            }
        }
        entry.close();
    }
    root.close();

    return devices;
}

// ─────────────────────────────────────────────────────────────
//  listButtons()
//  Devuelve todos los botones (sin extensión .json) de un dispositivo.
// ─────────────────────────────────────────────────────────────
std::vector<String> Storage::listButtons(const String& device) {
    std::vector<String> buttons;

    const String dirPath = "/" + _sanitize(device);
    File dir = LittleFS.open(dirPath);
    if (!dir || !dir.isDirectory()) return buttons;

    File entry;
    while ((entry = dir.openNextFile())) {
        const String fullPath = String(entry.name());
        const int lastSlash = fullPath.lastIndexOf('/');
        String fileName = (lastSlash >= 0)
                          ? fullPath.substring(lastSlash + 1)
                          : fullPath;

        if (fileName.endsWith(".json")) {
            buttons.push_back(fileName.substring(0, fileName.length() - 5));
        }
        entry.close();
    }
    dir.close();

    return buttons;
}

// ─────────────────────────────────────────────────────────────
//  listAll()
// ─────────────────────────────────────────────────────────────
std::vector<ButtonRef> Storage::listAll() {
    std::vector<ButtonRef> result;

    for (const String& device : listDevices()) {
        for (const String& button : listButtons(device)) {
            result.push_back({device, button});
        }
    }

    return result;
}

// ─────────────────────────────────────────────────────────────
//  freeBytes() / totalBytes()
// ─────────────────────────────────────────────────────────────
size_t Storage::freeBytes() {
    return LittleFS.totalBytes() - LittleFS.usedBytes();
}

size_t Storage::totalBytes() {
    return LittleFS.totalBytes();
}

// ─────────────────────────────────────────────────────────────
//  _buildPath()  [private]
// ─────────────────────────────────────────────────────────────
String Storage::_buildPath(const String& device, const String& button) const {
    return "/" + device + "/" + button + ".json";
}

// ─────────────────────────────────────────────────────────────
//  _sanitize()  [private]
//  Elimina caracteres inválidos para nombres de fichero/directorio
//  en LittleFS. Los espacios se convierten en guiones bajos.
// ─────────────────────────────────────────────────────────────
String Storage::_sanitize(const String& name) const {
    String result = name;
    const String invalid = "\\/:*?\"<>| ";
    for (char c : invalid) {
        result.replace(String(c), (c == ' ') ? "_" : "");
    }
    result.trim();
    return result;
}
