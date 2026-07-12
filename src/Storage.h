// ============================================================
//  IRManager — src/Storage.h
//  Responsabilidad única: persistencia de botones en LittleFS.
//  Un fichero JSON por botón: /<Dispositivo>/<Boton>.json
//  [STUB v0.1] — Implementación completa en v0.2.
// ============================================================
#pragma once

#include <Arduino.h>
#include <vector>
#include "IRSignal.h"

// Par (dispositivo, botón) — se usa en listados
struct ButtonRef {
    String device;
    String button;
};

class Storage {
public:
    // Inicializa LittleFS. Devuelve false si falla el montaje.
    bool begin();

    // ---- Escritura ---------------------------------------------------------
    // Guarda un botón. Crea el directorio del dispositivo si no existe.
    // Devuelve false si hay error de escritura.
    bool saveButton(const String& device, const String& button,
                    const IRSignal& signal);

    // ---- Lectura -----------------------------------------------------------
    // Carga un botón desde LittleFS. signal.valid == false si no existe.
    IRSignal loadButton(const String& device, const String& button);

    // ---- Borrado -----------------------------------------------------------
    bool deleteButton(const String& device, const String& button);

    // ---- Listados ----------------------------------------------------------
    std::vector<String>    listDevices();
    std::vector<String>    listButtons(const String& device);
    std::vector<ButtonRef> listAll();

    // ---- Diagnóstico -------------------------------------------------------
    // Espacio libre en LittleFS en bytes.
    size_t freeBytes();
    // Espacio total en bytes.
    size_t totalBytes();

private:
    // Construye la ruta LittleFS para un botón dado.
    // Ejemplo: buildPath("Samsung","POWER") → "/Samsung/POWER.json"
    String _buildPath(const String& device, const String& button) const;

    // Sanitiza un nombre de dispositivo o botón para uso como
    // nombre de directorio / fichero en LittleFS.
    String _sanitize(const String& name) const;
};
