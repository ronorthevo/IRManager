// ============================================================
//  IRManager — src/Storage.cpp
//  [STUB v0.1] — Implementación completa en v0.2.
// ============================================================
#include "Storage.h"

bool Storage::begin() {
    // TODO v0.2: LittleFS.begin(formatOnFail=true)
    return false;
}

bool Storage::saveButton(const String& device, const String& button,
                          const IRSignal& signal) {
    // TODO v0.2
    (void)device; (void)button; (void)signal;
    return false;
}

IRSignal Storage::loadButton(const String& device, const String& button) {
    // TODO v0.2
    (void)device; (void)button;
    return IRSignal{};
}

bool Storage::deleteButton(const String& device, const String& button) {
    // TODO v0.2
    (void)device; (void)button;
    return false;
}

std::vector<String> Storage::listDevices() {
    // TODO v0.2
    return {};
}

std::vector<String> Storage::listButtons(const String& device) {
    // TODO v0.2
    (void)device;
    return {};
}

std::vector<ButtonRef> Storage::listAll() {
    // TODO v0.2
    return {};
}

size_t Storage::freeBytes() {
    // TODO v0.2
    return 0;
}

size_t Storage::totalBytes() {
    // TODO v0.2
    return 0;
}

String Storage::_buildPath(const String& device, const String& button) const {
    return "/" + _sanitize(device) + "/" + _sanitize(button) + ".json";
}

String Storage::_sanitize(const String& name) const {
    String result = name;
    // Caracteres inválidos para nombres de fichero LittleFS
    const String invalid = "\\/:*?\"<>|";
    for (char c : invalid) {
        result.replace(String(c), "_");
    }
    result.trim();
    return result;
}
