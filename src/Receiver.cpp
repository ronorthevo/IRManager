// ============================================================
//  IRManager — src/Receiver.cpp
// ============================================================
#include "Receiver.h"

#include <IRremoteESP8266.h>
#include <IRutils.h>        // typeToString(), resultToHumanReadableBasic()
#include "Config.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
Receiver::Receiver(uint8_t pin)
    : _pin(pin)
    , _irrecv(pin, IR_RECV_BUF_SIZE, IR_RECV_TIMEOUT, /* saveBuffer */ true)
    , _listening(false)
{
}

// ─────────────────────────────────────────────────────────────
//  begin()
//  Activa el receptor de hardware IR y habilita la escucha.
// ─────────────────────────────────────────────────────────────
void Receiver::begin() {
    _irrecv.setUnknownThreshold(12);   // mínimo de marcas para UNKNOWN
    _irrecv.setTolerance(25);           // tolerancia de timing en %
    _irrecv.enableIRIn();
    _listening = true;
}

// ─────────────────────────────────────────────────────────────
//  loop()
//  Sondea el decodificador. Si hay señal, la convierte a
//  IRSignal y suspende la escucha hasta resume().
// ─────────────────────────────────────────────────────────────
IRSignal Receiver::loop() {
    if (!_listening) {
        return IRSignal{};   // valid = false por defecto
    }

    if (_irrecv.decode(&_results)) {
        _listening = false;  // suspender hasta que el llamador decida
        return _buildSignal(_results);
    }

    return IRSignal{};       // nada recibido
}

// ─────────────────────────────────────────────────────────────
//  resume()
//  Reanuda la escucha IR. Llamar DESPUÉS de haber procesado
//  completamente la señal anterior (guardar, imprimir, etc.).
// ─────────────────────────────────────────────────────────────
void Receiver::resume() {
    _irrecv.resume();
    _listening = true;
}

// ─────────────────────────────────────────────────────────────
//  _buildSignal()  [private]
//  Transforma decode_results en un IRSignal limpio.
// ─────────────────────────────────────────────────────────────
IRSignal Receiver::_buildSignal(const decode_results& r) const {
    IRSignal sig;

    sig.valid    = true;
    sig.protocol = r.decode_type;
    sig.value    = r.value;
    sig.bits     = r.bits;
    sig.address  = r.address;
    sig.command  = r.command;

    // Frecuencia de portadora.
    // IRremoteESP8266 expone la frecuencia solo para algunos protocolos;
    // para el resto usamos el valor por defecto de 38 kHz.
    sig.frequencyHz = IR_DEFAULT_FREQ_HZ;

    // Raw data: copiar el buffer completo.
    // rawbuf[0] es el gap inicial (no pertenece a la señal), se omite.
    sig.rawData.reserve(r.rawlen - 1);
    for (uint16_t i = 1; i < r.rawlen; ++i) {
        // Los valores de rawbuf están en unidades de RAWTICK (50 µs).
        // Los multiplicamos por RAWTICK para obtener µs reales.
        sig.rawData.push_back(r.rawbuf[i] * kRawTick);
    }

    return sig;
}
