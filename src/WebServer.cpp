// ============================================================
//  IRManager — src/WebServer.cpp
//  [STUB v0.1] — Implementación completa en v0.6.
// ============================================================
#include "WebServer.h"

WebServer::WebServer(uint16_t port) : _port(port) {}

void WebServer::begin() {
    // TODO v0.6: instanciar AsyncWebServer, servir /data/
}

AsyncWebServer* WebServer::server() {
    return _server;
}
