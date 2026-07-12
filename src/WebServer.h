// ============================================================
//  IRManager — src/WebServer.h
//  Responsabilidad única: arrancar AsyncWebServer y servir
//  los ficheros estáticos desde LittleFS (/data/).
//  v0.5 — Implementación completa.
// ============================================================
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class WebServer {
public:
    explicit WebServer(uint16_t port = 80);

    // Inicializa AsyncWebServer, registra la ruta estática y
    // habilita CORS global (necesario para futura app Android).
    void begin();

    // Devuelve puntero al servidor para que Api registre las rutas.
    AsyncWebServer* server() { return _server; }

private:
    uint16_t        _port;
    AsyncWebServer* _server = nullptr;
};
