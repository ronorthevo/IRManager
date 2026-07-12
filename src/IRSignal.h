// ============================================================
//  IRManager — src/IRSignal.h
//  Struct de datos puro (POD) que representa una señal IR
//  capturada. Actúa como contrato entre la capa de captura
//  (Receiver) y el resto del sistema.
//
//  DISEÑO INTENCIONAL:
//  - Header-only: sin .cpp asociado.
//  - Sin dependencias de IRremoteESP8266 fuera de Receiver.
//  - Todos los campos son tipos estándar de C++/Arduino.
//  - Permite serialización/deserialización independiente.
// ============================================================
#pragma once

#include <Arduino.h>
#include <IRremoteESP8266.h>   // decode_type_t
#include <IRutils.h>           // typeToString()
#include <vector>

// ─────────────────────────────────────────────────────────────
//  IRSignal
//  Resultado completo de una captura IR.
// ─────────────────────────────────────────────────────────────
struct IRSignal {

    // ---- Protocolo -----------------------------------------
    decode_type_t protocol = decode_type_t::UNKNOWN;

    // ---- Valor decodificado --------------------------------
    // Se almacena como uint64_t.
    // En JSON se serializa como string hex para evitar pérdida
    // de precisión (JSON number solo garantiza 53 bits seguros).
    uint64_t value   = 0;
    uint16_t bits    = 0;

    // ---- Detalles extendidos (cuando el protocolo los da) --
    uint64_t address = 0;
    uint64_t command = 0;

    // ---- Frecuencia de portadora ---------------------------
    uint32_t frequencyHz = 38000;

    // ---- Raw data ------------------------------------------
    // Siempre se captura. Necesario para protocolos UNKNOWN
    // y como fallback de emisión si el protocolo no está soportado.
    std::vector<uint16_t> rawData;

    // ---- Marca de validez ----------------------------------
    bool valid = false;

    // ─────────────────────────────────────────────────────────
    //  Helpers
    // ─────────────────────────────────────────────────────────

    /// Devuelve el nombre del protocolo como String.
    String protocolName() const {
        return typeToString(protocol, /* verbose */ false);
    }

    /// Devuelve el valor como string hexadecimal con prefijo "0x".
    String valueHex() const {
        char buf[20];
        snprintf(buf, sizeof(buf), "0x%llX", (unsigned long long)value);
        return String(buf);
    }

    /// True si el protocolo es desconocido (solo raw disponible).
    bool isRawOnly() const {
        return protocol == decode_type_t::UNKNOWN;
    }
};
