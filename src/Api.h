// ============================================================
//  IRManager — src/Api.h
//  Responsabilidad única: endpoints REST sobre AsyncWebServer.
//  v0.5 — Implementación completa.
//
//  THREAD SAFETY:
//  AsyncWebServer ejecuta sus callbacks en un task de FreeRTOS
//  distinto al loop principal. Para comunicar el estado "learn
//  pendiente" de forma segura se usan arrays de char fijos con
//  escritura-antes-de-activar (producer/consumer pattern).
// ============================================================
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "Storage.h"
#include "Receiver.h"
#include "Sender.h"
#include "WifiManager.h"

// ─────────────────────────────────────────────────────────────
//  ApiPendingLearn
//  Estado compartido entre el callback HTTP (writer) y el
//  loop principal (reader). Se usa un flag volatile + arrays
//  de char de tamaño fijo para evitar problemas de heap en
//  contexto de interrupción/task.
// ─────────────────────────────────────────────────────────────
struct ApiPendingLearn {
    volatile bool active = false;
    char device[33]      = {};
    char button[33]      = {};

    void set(const char* dev, const char* btn) {
        strncpy(device, dev, 32); device[32] = '\0';
        strncpy(button, btn, 32); button[32] = '\0';
        active = true;             // activar DESPUÉS de copiar strings
    }

    void clear() { active = false; device[0] = '\0'; button[0] = '\0'; }
};

// ─────────────────────────────────────────────────────────────
//  Api
// ─────────────────────────────────────────────────────────────
class Api {
public:
    Api(AsyncWebServer* server, Storage* storage,
        Receiver* receiver, Sender* sender, WifiManager* wifi);

    // Registra todos los endpoints REST.
    // Llamar DESPUÉS de WebServer::begin().
    void registerRoutes();

    // ── Estado compartido con main.cpp ───────────────────────
    const ApiPendingLearn& getPendingLearn() const { return _pending; }
    void clearPendingLearn()                       { _pending.clear(); }

    // Notifica el resultado del último learn (llamado desde main.cpp).
    void notifyLearnResult(bool success);

private:
    AsyncWebServer* _server;
    Storage*        _storage;
    Receiver*       _receiver;
    Sender*         _sender;
    WifiManager*    _wifi;

    ApiPendingLearn _pending;

    // ── Flag de resultado del último learn ───────────────────
    // 0 = ninguno, 1 = OK, 2 = error
    volatile uint8_t _lastLearnResult = 0;

    // ── Helpers de respuesta ─────────────────────────────────
    static void _sendJson(AsyncWebServerRequest* req, int code,
                          const String& json);
    static void _sendError(AsyncWebServerRequest* req, int code,
                           const String& message);
    static bool _parseBody(uint8_t* data, size_t len,
                           String& device, String& button);
};
