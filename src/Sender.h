// ============================================================
//  IRManager — src/Sender.h
//  Responsabilidad única: emisión de señales IR.
//  [STUB v0.1] — Implementación completa en v0.3.
// ============================================================
#pragma once

#include "IRSignal.h"

class Sender {
public:
    explicit Sender(uint8_t pin);

    void begin();

    // Emite la señal IR descrita por el IRSignal dado.
    // Devuelve true si la emisión se realizó correctamente.
    bool send(const IRSignal& signal);

private:
    uint8_t _pin;
};
