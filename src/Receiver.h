// ============================================================
//  IRManager — src/Receiver.h
//  Responsabilidad única: captura de señales IR desde el
//  TSOP4838 y entrega de un IRSignal al llamador.
//
//  DISEÑO:
//  - No almacena, no imprime, no envía. Solo captura.
//  - El objeto IRrecv se crea internamente (ownership claro).
//  - loop() devuelve un IRSignal. Si valid==false, no hay señal.
//  - El llamador es responsable de llamar a resume() cuando
//    quiera volver a escuchar (permite control de flujo externo).
// ============================================================
#pragma once

#include <IRrecv.h>
#include "IRSignal.h"

class Receiver {
public:
    // ---- Constructor -------------------------------------------------------
    // pin: GPIO donde está conectado el TSOP4838.
    explicit Receiver(uint8_t pin);

    // ---- Ciclo de vida -----------------------------------------------------
    // Inicializa el hardware de recepción IR. Llamar una vez en setup().
    void begin();

    // ---- Loop principal ----------------------------------------------------
    // Debe llamarse en cada iteración del loop de Arduino.
    // Devuelve un IRSignal. Si signal.valid == false, no se recibió nada.
    // Si signal.valid == true, el receptor queda SUSPENDIDO hasta que
    // el llamador invoque resume() explícitamente.
    IRSignal loop();

    // ---- Control de estado -------------------------------------------------
    // Reanuda la escucha después de haber procesado una señal.
    void resume();

    // Devuelve true si el receptor está actualmente escuchando.
    bool isListening() const { return _listening; }

private:
    uint8_t        _pin;
    IRrecv         _irrecv;
    decode_results _results;
    bool           _listening = false;

    // Convierte decode_results (tipo de IRremoteESP8266) en IRSignal.
    IRSignal _buildSignal(const decode_results& results) const;
};
