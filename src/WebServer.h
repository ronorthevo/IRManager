// ============================================================
//  IRManager — src/WebServer.h
//  Responsabilidad única: servir ficheros estáticos desde
//  LittleFS (/data/) — HTML, CSS, JS de la interfaz web.
//  [STUB v0.1] — Implementación completa en v0.6.
// ============================================================
#pragma once

#include <Arduino.h>

class AsyncWebServer;

class WebServer {
public:
    explicit WebServer(uint16_t port = 80);

    // Inicializa y arranca el servidor HTTP.
    void begin();

    // Devuelve el puntero al servidor (para que Api lo use).
    AsyncWebServer* server();

private:
    uint16_t        _port;
    AsyncWebServer* _server = nullptr;
};
