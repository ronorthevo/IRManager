// ============================================================
//  IRManager — src/WifiManager.h
//  Responsabilidad única: gestión de conectividad WiFi.
//  Modos: STA (conecta a router) + AP fallback (portal cautivo).
//  [STUB v0.1] — Implementación completa en v0.5.
// ============================================================
#pragma once

#include <Arduino.h>

// Estado de la conexión WiFi — permite que otros módulos reaccionen
enum class WifiState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED_STA,    // conectado a router (modo estación)
    AP_ACTIVE         // punto de acceso propio activo
};

class WifiManager {
public:
    // Inicializa WiFi: intenta STA con credenciales guardadas.
    // Si falla en connectTimeoutMs ms, levanta AP de configuración.
    void begin(uint32_t connectTimeoutMs = 15000);

    // Debe llamarse en el loop de Arduino.
    void loop();

    // ---- Estado --------------------------------------------------------
    WifiState   state()    const { return _state; }
    String      ssid()     const;
    String      localIP()  const;
    int8_t      rssi()     const;
    bool        isConnected() const { return _state == WifiState::CONNECTED_STA; }

    // ---- Configuración -------------------------------------------------
    // Guarda las credenciales en NVS (Preferences).
    void setCredentials(const String& ssid, const String& password);

    // Borra las credenciales almacenadas.
    void clearCredentials();

private:
    WifiState _state = WifiState::DISCONNECTED;

    bool _startSTA();
    void _startAP();
};
