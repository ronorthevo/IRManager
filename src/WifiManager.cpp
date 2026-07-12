// ============================================================
//  IRManager — src/WifiManager.cpp
//  v0.4 — Implementación completa AP+STA con portal cautivo.
// ============================================================
#include "WifiManager.h"

// ─────────────────────────────────────────────────────────────
//  begin()
// ─────────────────────────────────────────────────────────────
void WifiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);    // lo gestionamos nosotros

    String ssid, password;

    if (_loadCredentials(ssid, password)) {
        Serial.printf("[WiFi] Credenciales encontradas. Conectando a '%s'...\n",
                      ssid.c_str());
        if (_startSTA(ssid, password)) {
            return;   // conexión STA exitosa
        }
        Serial.println(F("[WiFi] Conexión STA fallida. Iniciando portal de configuración."));
    } else {
        Serial.println(F("[WiFi] Sin credenciales guardadas. Iniciando portal de configuración."));
    }

    _startAP();
}

// ─────────────────────────────────────────────────────────────
//  loop()
// ─────────────────────────────────────────────────────────────
void WifiManager::loop() {
    if (_state == WifiState::CONNECTED_STA) {
        _checkReconnect();
    }
}

// ─────────────────────────────────────────────────────────────
//  Estado público
// ─────────────────────────────────────────────────────────────
String WifiManager::staSSID() const {
    return (_state == WifiState::CONNECTED_STA)
           ? WiFi.SSID()
           : String("");
}

String WifiManager::localIP() const {
    return (_state == WifiState::CONNECTED_STA)
           ? WiFi.localIP().toString()
           : String("0.0.0.0");
}

String WifiManager::apIP() const {
    return (_state == WifiState::AP_ACTIVE)
           ? WiFi.softAPIP().toString()
           : String("0.0.0.0");
}

int8_t WifiManager::rssi() const {
    return (_state == WifiState::CONNECTED_STA)
           ? WiFi.RSSI()
           : 0;
}

// ─────────────────────────────────────────────────────────────
//  setCredentials()
//  Guarda credenciales en NVS y reinicia para aplicarlas.
// ─────────────────────────────────────────────────────────────
void WifiManager::setCredentials(const String& ssid, const String& password) {
    Preferences prefs;
    prefs.begin(WifiConfig::NVS_NAMESPACE, /* readOnly */ false);
    prefs.putString(WifiConfig::NVS_KEY_SSID, ssid);
    prefs.putString(WifiConfig::NVS_KEY_PASS, password);
    prefs.end();

    Serial.printf("[WiFi] Credenciales guardadas para '%s'. Reiniciando...\n",
                  ssid.c_str());
    delay(500);
    ESP.restart();
}

// ─────────────────────────────────────────────────────────────
//  clearCredentials()
// ─────────────────────────────────────────────────────────────
void WifiManager::clearCredentials() {
    Preferences prefs;
    prefs.begin(WifiConfig::NVS_NAMESPACE, false);
    prefs.remove(WifiConfig::NVS_KEY_SSID);
    prefs.remove(WifiConfig::NVS_KEY_PASS);
    prefs.end();
    Serial.println(F("[WiFi] Credenciales borradas."));
}

// ─────────────────────────────────────────────────────────────
//  hasCredentials()
// ─────────────────────────────────────────────────────────────
bool WifiManager::hasCredentials() const {
    String ssid, password;
    return _loadCredentials(ssid, password);
}

// ─────────────────────────────────────────────────────────────
//  _loadCredentials()  [private]
// ─────────────────────────────────────────────────────────────
bool WifiManager::_loadCredentials(String& ssid, String& password) const {
    Preferences prefs;
    prefs.begin(WifiConfig::NVS_NAMESPACE, /* readOnly */ true);
    ssid     = prefs.getString(WifiConfig::NVS_KEY_SSID, "");
    password = prefs.getString(WifiConfig::NVS_KEY_PASS, "");
    prefs.end();
    return ssid.length() > 0;
}

// ─────────────────────────────────────────────────────────────
//  _startSTA()  [private]
//  Intenta conectar en modo estación. Timeout configurable.
// ─────────────────────────────────────────────────────────────
bool WifiManager::_startSTA(const String& ssid, const String& password) {
    _state = WifiState::CONNECTING;

    WiFi.begin(ssid.c_str(), password.c_str());

    const uint32_t deadline = millis() + WifiConfig::STA_TIMEOUT_MS;
    while (millis() < deadline) {
        if (WiFi.status() == WL_CONNECTED) {
            _state = WifiState::CONNECTED_STA;
            Serial.printf("[WiFi] Conectado! IP: %s | RSSI: %d dBm\n",
                          WiFi.localIP().toString().c_str(),
                          WiFi.RSSI());
            return true;
        }
        delay(250);
        Serial.print(F("."));
    }

    Serial.println();
    WiFi.disconnect(/* wifioff */ true);
    _state = WifiState::DISCONNECTED;
    return false;
}

// ─────────────────────────────────────────────────────────────
//  _startAP()  [private]
//  Levanta el punto de acceso de configuración.
//  La configuración real del portal cautivo (redirección HTTP)
//  se integrará en WebServer en v0.5.
// ─────────────────────────────────────────────────────────────
void WifiManager::_startAP() {
    WiFi.mode(WIFI_AP);

    IPAddress ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gateway, subnet);

    const bool ok = WiFi.softAP(
        WifiConfig::AP_SSID,
        WifiConfig::AP_PASSWORD
    );

    if (ok) {
        _state = WifiState::AP_ACTIVE;
        Serial.println(F("[WiFi] Modo AP activo."));
        Serial.printf("[WiFi] SSID: %s | Password: %s | IP: %s\n",
                      WifiConfig::AP_SSID,
                      WifiConfig::AP_PASSWORD,
                      WiFi.softAPIP().toString().c_str());
        Serial.println(F("[WiFi] Conéctate al AP y accede a http://192.168.4.1"));
        Serial.println(F("[WiFi] Usa 'wifi <ssid> <password>' para configurar."));
    } else {
        Serial.println(F("[WiFi] ERROR: No se pudo iniciar el AP."));
        _state = WifiState::DISCONNECTED;
    }
}

// ─────────────────────────────────────────────────────────────
//  _checkReconnect()  [private]
//  Comprueba periódicamente si la conexión STA sigue activa.
// ─────────────────────────────────────────────────────────────
void WifiManager::_checkReconnect() {
    if (WiFi.status() == WL_CONNECTED) return;

    const uint32_t now = millis();
    if (now - _lastReconnect < WifiConfig::RECONNECT_MS) return;

    _lastReconnect = now;
    _state = WifiState::DISCONNECTED;

    Serial.println(F("[WiFi] Conexión perdida. Intentando reconectar..."));

    String ssid, password;
    if (_loadCredentials(ssid, password) && _startSTA(ssid, password)) {
        Serial.println(F("[WiFi] Reconexión exitosa."));
    } else {
        Serial.println(F("[WiFi] Reconexión fallida. Reintentará en 30 s."));
    }
}
