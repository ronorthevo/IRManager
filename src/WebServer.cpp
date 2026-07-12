// ============================================================
//  IRManager — src/WebServer.cpp
//  v0.5 — Implementación completa.
// ============================================================
#include "WebServer.h"
#include <LittleFS.h>

WebServer::WebServer(uint16_t port)
    : _port(port)
    , _server(nullptr)
{
}

void WebServer::begin() {
    _server = new AsyncWebServer(_port);

    // ── CORS global ─────────────────────────────────────────
    // Permite que la futura app Android/web externa acceda a la API.
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin",  "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,POST,DELETE,OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // ── Preflight OPTIONS ────────────────────────────────────
    _server->onNotFound([](AsyncWebServerRequest* req) {
        if (req->method() == HTTP_OPTIONS) {
            req->send(204);
        } else {
            // Si no es OPTIONS y no existe el fichero, devolver index.html
            // (SPA fallback — el JS gestiona el routing)
            if (LittleFS.exists("/index.html")) {
                req->send(LittleFS, "/index.html", "text/html");
            } else {
                req->send(404, "text/plain", "Not Found. Sube la interfaz web con 'pio run -t uploadfs'");
            }
        }
    });

    // ── Ficheros estáticos desde LittleFS ───────────────────
    // La ruta "/" sirve index.html; el resto de rutas /data/
    // se sirven directamente (CSS, JS, imágenes…).
    _server->serveStatic("/", LittleFS, "/")
           .setDefaultFile("index.html")
           .setCacheControl("max-age=3600");

    _server->begin();
    Serial.printf("[WebServer] OK — Escuchando en puerto %u\n", _port);
}
