// ============================================================
//  IRManager — src/Api.cpp
//  v0.5 — REST API completa.
//
//  Endpoints:
//    GET  /api/status
//    GET  /api/devices
//    GET  /api/device/:device/buttons
//    GET  /api/button/:device/:button
//    POST /api/learn          body: {"device":"X","button":"Y"}
//    POST /api/send           body: {"device":"X","button":"Y"}
//    DELETE /api/button/:device/:button
// ============================================================
#include "Api.h"
#include <ArduinoJson.h>
#include "Config.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
Api::Api(AsyncWebServer* server, Storage* storage,
         Receiver* receiver, Sender* sender, WifiManager* wifi)
    : _server(server)
    , _storage(storage)
    , _receiver(receiver)
    , _sender(sender)
    , _wifi(wifi)
{
}

// ─────────────────────────────────────────────────────────────
//  registerRoutes()
// ─────────────────────────────────────────────────────────────
void Api::registerRoutes() {

    // ── GET /api/status ─────────────────────────────────────
    _server->on("/api/status", HTTP_GET,
        [this](AsyncWebServerRequest* req) {
            JsonDocument doc;

            doc["version"]      = FW_VERSION;
            doc["uptime_s"]     = millis() / 1000;
            doc["heap_free"]    = ESP.getFreeHeap();
            doc["heap_total"]   = ESP.getHeapSize();
            doc["fs_free"]      = _storage->freeBytes();
            doc["fs_total"]     = _storage->totalBytes();
            doc["receiver"]     = _receiver->isListening() ? "listening" : "suspended";
            doc["learn_pending"] = _pending.active;
            doc["last_learn"]   = (_lastLearnResult == 1) ? "ok"
                                : (_lastLearnResult == 2) ? "error"
                                : "none";

            // WiFi
            switch (_wifi->state()) {
                case WifiState::CONNECTED_STA:
                    doc["wifi_mode"] = "sta";
                    doc["wifi_ssid"] = _wifi->staSSID();
                    doc["wifi_ip"]   = _wifi->localIP();
                    doc["wifi_rssi"] = _wifi->rssi();
                    break;
                case WifiState::AP_ACTIVE:
                    doc["wifi_mode"] = "ap";
                    doc["wifi_ssid"] = WifiConfig::AP_SSID;
                    doc["wifi_ip"]   = _wifi->apIP();
                    doc["wifi_rssi"] = 0;
                    break;
                default:
                    doc["wifi_mode"] = "disconnected";
                    doc["wifi_ip"]   = "0.0.0.0";
                    doc["wifi_rssi"] = 0;
                    break;
            }

            String json;
            serializeJson(doc, json);
            _sendJson(req, 200, json);
        }
    );

    // ── GET /api/devices ────────────────────────────────────
    _server->on("/api/devices", HTTP_GET,
        [this](AsyncWebServerRequest* req) {
            const auto devices = _storage->listDevices();

            JsonDocument doc;
            JsonArray arr = doc["devices"].to<JsonArray>();
            for (const String& dev : devices) {
                JsonObject obj = arr.add<JsonObject>();
                obj["name"]    = dev;
                obj["buttons"] = _storage->listButtons(dev).size();
            }
            doc["count"] = devices.size();

            String json;
            serializeJson(doc, json);
            _sendJson(req, 200, json);
        }
    );

    // ── GET /api/device/:device/buttons ─────────────────────
    _server->on("/api/device/{device}/buttons", HTTP_GET,
        [this](AsyncWebServerRequest* req) {
            const String device = req->pathArg(0);
            const auto buttons  = _storage->listButtons(device);

            if (buttons.empty() &&
                !_storage->listDevices().size()) {
                _sendError(req, 404, "Dispositivo no encontrado: " + device);
                return;
            }

            JsonDocument doc;
            doc["device"]  = device;
            JsonArray arr  = doc["buttons"].to<JsonArray>();
            for (const String& btn : buttons) { arr.add(btn); }
            doc["count"]   = buttons.size();

            String json;
            serializeJson(doc, json);
            _sendJson(req, 200, json);
        }
    );

    // ── GET /api/button/:device/:button ─────────────────────
    _server->on("/api/button/{device}/{button}", HTTP_GET,
        [this](AsyncWebServerRequest* req) {
            const String device = req->pathArg(0);
            const String button = req->pathArg(1);

            const IRSignal sig = _storage->loadButton(device, button);
            if (!sig.valid) {
                _sendError(req, 404,
                    "Botón no encontrado: " + device + "/" + button);
                return;
            }

            JsonDocument doc;
            doc["device"]    = device;
            doc["button"]    = button;
            doc["protocol"]  = sig.protocolName();
            doc["value"]     = sig.valueHex();
            doc["bits"]      = sig.bits;
            doc["frequency"] = sig.frequencyHz;
            doc["address"]   = (uint32_t)sig.address;
            doc["command"]   = (uint32_t)sig.command;

            // Incluir raw solo si el cliente lo solicita
            const bool includeRaw = req->hasParam("raw") &&
                                    req->getParam("raw")->value() != "false";
            if (includeRaw || sig.isRawOnly()) {
                JsonArray raw = doc["raw"].to<JsonArray>();
                for (const uint16_t v : sig.rawData) { raw.add(v); }
            }

            String json;
            serializeJson(doc, json);
            _sendJson(req, 200, json);
        }
    );

    // ── DELETE /api/button/:device/:button ──────────────────
    _server->on("/api/button/{device}/{button}", HTTP_DELETE,
        [this](AsyncWebServerRequest* req) {
            const String device = req->pathArg(0);
            const String button = req->pathArg(1);

            if (_storage->deleteButton(device, button)) {
                JsonDocument doc;
                doc["status"]  = "ok";
                doc["message"] = "Botón eliminado: " + device + "/" + button;
                String json; serializeJson(doc, json);
                _sendJson(req, 200, json);
            } else {
                _sendError(req, 404,
                    "No se pudo eliminar: " + device + "/" + button);
            }
        }
    );

    // ── POST /api/learn ─────────────────────────────────────
    // Body: {"device":"Samsung","button":"POWER"}
    // El callback de cuerpo corre en task de AsyncWebServer,
    // por eso se usa ApiPendingLearn thread-safe.
    _server->on("/api/learn", HTTP_POST,
        // Request handler (sin body — solo se llama en peticiones sin body)
        [](AsyncWebServerRequest* req) {},
        // Upload handler (no aplica)
        nullptr,
        // Body handler
        [this](AsyncWebServerRequest* req,
               uint8_t* data, size_t len, size_t index, size_t total) {

            if (index + len < total) return;    // esperar cuerpo completo

            if (_pending.active) {
                _sendError(req, 409,
                    "Ya hay un aprendizaje en curso");
                return;
            }

            String device, button;
            if (!_parseBody(data, len, device, button)) {
                _sendError(req, 400,
                    "Body inválido. Esperado: {\"device\":\"X\",\"button\":\"Y\"}");
                return;
            }

            _lastLearnResult = 0;
            _pending.set(device.c_str(), button.c_str());
            _receiver->resume();   // asegura que el receptor esté escuchando

            JsonDocument doc;
            doc["status"]  = "listening";
            doc["device"]  = device;
            doc["button"]  = button;
            doc["message"] = "Apunta el mando al receptor y pulsa el botón";
            String json; serializeJson(doc, json);
            _sendJson(req, 202, json);
        }
    );

    // ── POST /api/send ──────────────────────────────────────
    // Body: {"device":"Samsung","button":"POWER"}
    _server->on("/api/send", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        nullptr,
        [this](AsyncWebServerRequest* req,
               uint8_t* data, size_t len, size_t index, size_t total) {

            if (index + len < total) return;

            String device, button;
            if (!_parseBody(data, len, device, button)) {
                _sendError(req, 400,
                    "Body inválido. Esperado: {\"device\":\"X\",\"button\":\"Y\"}");
                return;
            }

            const IRSignal sig = _storage->loadButton(device, button);
            if (!sig.valid) {
                _sendError(req, 404,
                    "Botón no encontrado: " + device + "/" + button);
                return;
            }

            const bool ok = _sender->send(sig);
            JsonDocument doc;
            doc["status"]   = ok ? "ok" : "error";
            doc["device"]   = device;
            doc["button"]   = button;
            doc["protocol"] = sig.protocolName();
            String json; serializeJson(doc, json);
            _sendJson(req, ok ? 200 : 500, json);
        }
    );

    Serial.println(F("[Api] Rutas REST registradas:"));
    Serial.println(F("  GET    /api/status"));
    Serial.println(F("  GET    /api/devices"));
    Serial.println(F("  GET    /api/device/{device}/buttons"));
    Serial.println(F("  GET    /api/button/{device}/{button}[?raw=true]"));
    Serial.println(F("  POST   /api/learn"));
    Serial.println(F("  POST   /api/send"));
    Serial.println(F("  DELETE /api/button/{device}/{button}"));
}

// ─────────────────────────────────────────────────────────────
//  notifyLearnResult()
//  Llamado desde main.cpp después de procesar una señal IR
//  pendiente que vino del endpoint /api/learn.
// ─────────────────────────────────────────────────────────────
void Api::notifyLearnResult(bool success) {
    _lastLearnResult = success ? 1 : 2;
}

// ─────────────────────────────────────────────────────────────
//  _sendJson()  [private, static]
// ─────────────────────────────────────────────────────────────
void Api::_sendJson(AsyncWebServerRequest* req, int code,
                    const String& json) {
    req->send(code, "application/json", json);
}

// ─────────────────────────────────────────────────────────────
//  _sendError()  [private, static]
// ─────────────────────────────────────────────────────────────
void Api::_sendError(AsyncWebServerRequest* req, int code,
                     const String& message) {
    JsonDocument doc;
    doc["status"]  = "error";
    doc["message"] = message;
    doc["code"]    = code;
    String json; serializeJson(doc, json);
    req->send(code, "application/json", json);
}

// ─────────────────────────────────────────────────────────────
//  _parseBody()  [private, static]
//  Parsea el body JSON y extrae device y button.
// ─────────────────────────────────────────────────────────────
bool Api::_parseBody(uint8_t* data, size_t len,
                     String& device, String& button) {
    JsonDocument doc;
    if (deserializeJson(doc, data, len)) return false;

    device = doc["device"] | "";
    button = doc["button"] | "";

    return device.length() > 0 && button.length() > 0;
}
