// ============================================================
//  IRManager — src/Sender.h
//  Responsabilidad única: emisión de señales IR.
//  v0.3 — Implementación completa.
//
//  ESTRATEGIA DE EMISIÓN:
//  1. Si el protocolo está soportado por IRsend, se usa el
//     método nativo (mejor timing, menor overhead).
//  2. Si el protocolo es UNKNOWN o no está soportado, se emite
//     el rawData como señal raw (máxima compatibilidad).
// ============================================================
#pragma once

#include <IRsend.h>
#include "IRSignal.h"

class Sender {
public:
    // pin: GPIO del LED IR (con transistor de potencia).
    explicit Sender(uint8_t pin);

    // Inicializa IRsend. Llamar una vez en setup().
    void begin();

    // Emite la señal IR descrita por IRSignal.
    // Devuelve true si la emisión se realizó.
    bool send(const IRSignal& signal);

    // Indica si el transmisor está listo (begin() fue llamado).
    bool isReady() const { return _ready; }

private:
    uint8_t  _pin;
    IRsend   _irsend;
    bool     _ready = false;

    // Emite usando el protocolo nativo de IRsend.
    bool _sendByProtocol(const IRSignal& signal);

    // Emite usando rawData (fallback universal).
    bool _sendRaw(const IRSignal& signal);
};
