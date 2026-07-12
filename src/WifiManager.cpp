// ============================================================
//  IRManager — src/WifiManager.cpp
//  [STUB v0.1] — Implementación completa en v0.5.
// ============================================================
#include "WifiManager.h"

void WifiManager::begin(uint32_t connectTimeoutMs) {
    // TODO v0.5: intentar STA → fallback a AP
    (void)connectTimeoutMs;
}

void WifiManager::loop() {
    // TODO v0.5: reconexión automática, portal cautivo
}

String WifiManager::ssid() const {
    // TODO v0.5
    return "";
}

String WifiManager::localIP() const {
    // TODO v0.5
    return "0.0.0.0";
}

int8_t WifiManager::rssi() const {
    // TODO v0.5
    return 0;
}

void WifiManager::setCredentials(const String& ssid, const String& password) {
    // TODO v0.5: guardar en NVS con Preferences
    (void)ssid; (void)password;
}

void WifiManager::clearCredentials() {
    // TODO v0.5
}

bool WifiManager::_startSTA() {
    // TODO v0.5
    return false;
}

void WifiManager::_startAP() {
    // TODO v0.5
}
