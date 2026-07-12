// ============================================================
//  IRManager — src/Api.h
//  Responsabilidad única: endpoints REST sobre AsyncWebServer.
//  [STUB v0.1] — Implementación completa en v0.5.
// ============================================================
#pragma once

#include <Arduino.h>

// Forward declaration — no incluimos AsyncWebServer aún
// para evitar que toda la librería se compile en v0.1
class AsyncWebServer;
class Storage;
class Receiver;
class Sender;

class Api {
public:
    Api(AsyncWebServer* server, Storage* storage,
        Receiver* receiver, Sender* sender);

    // Registra todos los endpoints REST en el servidor.
    // Llamar DESPUÉS de WebServer::begin().
    void registerRoutes();

private:
    AsyncWebServer* _server;
    Storage*        _storage;
    Receiver*       _receiver;
    Sender*         _sender;

    // ---- Handlers (uno por endpoint) -----------------------------------
    // GET /api/status
    void _handleStatus();
    // GET /api/devices
    void _handleGetDevices();
    // GET /api/device/:name/buttons
    void _handleGetButtons();
    // GET /api/button/:device/:button
    void _handleGetButton();
    // POST /api/learn
    void _handleLearn();
    // POST /api/send
    void _handleSend();
    // DELETE /api/button/:device/:button
    void _handleDeleteButton();
};
