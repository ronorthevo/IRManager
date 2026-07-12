// ============================================================
//  IRManager — src/Sender.cpp
//  [STUB v0.1] — Implementación completa en v0.3.
// ============================================================
#include "Sender.h"

Sender::Sender(uint8_t pin) : _pin(pin) {}

void Sender::begin() {
    // TODO v0.3: inicializar IRsend
}

bool Sender::send(const IRSignal& signal) {
    // TODO v0.3: implementar emisión por protocolo y fallback raw
    (void)signal;
    return false;
}
