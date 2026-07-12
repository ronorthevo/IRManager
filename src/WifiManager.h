// ============================================================
//  IRManager — src/WifiManager.h
//  Responsabilidad única: gestión de conectividad WiFi.
//
//  ESTRATEGIA (AP+STA con portal cautivo):
//
//  1. Al arrancar, busca credenciales guardadas en NVS.
//  2. Si existen: intenta conectar en modo STA con timeout.
//  3. Si NO existen o falla la conexión:
//       → levanta un AP propio "IRManager-Setup"
//       → sirve un portal cautivo para introducir credenciales
//       → guarda las credenciales en NVS y reinicia en STA
//  4. Una vez conectado en STA: monitoriza la conexión y
//     reconecta automáticamente si se cae.
//
//  v0.4 — Implementación completa.
// ============================================================
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// Estado de la conexión — permite que otros módulos reaccionen
enum class WifiState : uint8_t {
    DISCONNECTED = 0,
    CONNECTING,
    CONNECTED_STA,    // conectado a router (modo estación)
    AP_ACTIVE         // punto de acceso propio activo (portal cautivo)
};

// ─────────────────────────────────────────────────────────────
//  Configuración del AP de portal cautivo
// ─────────────────────────────────────────────────────────────
namespace WifiConfig {
    constexpr char     AP_SSID[]       = "IRManager-Setup";
    constexpr char     AP_PASSWORD[]   = "irmanager";     // mín. 8 chars
    constexpr char     AP_IP[]         = "192.168.4.1";
    constexpr char     NVS_NAMESPACE[] = "irmanager";
    constexpr char     NVS_KEY_SSID[]  = "wifi_ssid";
    constexpr char     NVS_KEY_PASS[]  = "wifi_pass";
    constexpr uint32_t STA_TIMEOUT_MS  = 15000;           // 15 s
    constexpr uint32_t RECONNECT_MS    = 30000;           // cada 30 s
}

// ─────────────────────────────────────────────────────────────
//  WifiManager
// ─────────────────────────────────────────────────────────────
class WifiManager {
public:
    // Inicializa WiFi según la estrategia descrita arriba.
    void begin();

    // Debe llamarse en el loop de Arduino.
    // Gestiona reconexión automática en STA.
    void loop();

    // ── Estado ───────────────────────────────────────────────
    WifiState   state()       const { return _state; }
    bool        isConnected() const { return _state == WifiState::CONNECTED_STA; }
    bool        isAP()        const { return _state == WifiState::AP_ACTIVE; }

    String      staSSID()     const;
    String      localIP()     const;
    String      apIP()        const;
    int8_t      rssi()        const;

    // ── Gestión de credenciales ──────────────────────────────
    // Guarda credenciales en NVS y reinicia para reconectar.
    void        setCredentials(const String& ssid, const String& password);

    // Borra las credenciales y levanta el AP de configuración.
    void        clearCredentials();

    // Devuelve true si hay credenciales guardadas en NVS.
    bool        hasCredentials() const;

private:
    WifiState   _state        = WifiState::DISCONNECTED;
    uint32_t    _lastReconnect = 0;
    Preferences _prefs;

    // ── Helpers internos ─────────────────────────────────────
    bool        _loadCredentials(String& ssid, String& password) const;
    bool        _startSTA(const String& ssid, const String& password);
    void        _startAP();
    void        _checkReconnect();
};
