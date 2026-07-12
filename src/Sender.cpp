// ============================================================
//  IRManager — src/Sender.cpp
//  v0.3 — Implementación completa de emisión IR.
// ============================================================
#include "Sender.h"
#include "Config.h"

// ─────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────
Sender::Sender(uint8_t pin)
    : _pin(pin)
    , _irsend(pin)
    , _ready(false)
{
}

// ─────────────────────────────────────────────────────────────
//  begin()
// ─────────────────────────────────────────────────────────────
void Sender::begin() {
    _irsend.begin();
    _ready = true;
    Serial.printf("[Sender] OK — Emisor IR en GPIO %d\n", _pin);
}

// ─────────────────────────────────────────────────────────────
//  send()
//  Punto de entrada público. Decide la estrategia de emisión.
// ─────────────────────────────────────────────────────────────
bool Sender::send(const IRSignal& signal) {
    if (!_ready) {
        Serial.println(F("[Sender] ERROR: begin() no fue llamado."));
        return false;
    }

    if (!signal.valid) {
        Serial.println(F("[Sender] ERROR: IRSignal no válido."));
        return false;
    }

    // Para protocolos UNKNOWN siempre usamos raw
    if (signal.isRawOnly()) {
        return _sendRaw(signal);
    }

    // Intentamos emisión nativa; si falla, fallback a raw
    if (_sendByProtocol(signal)) {
        return true;
    }

    Serial.printf("[Sender] Protocolo '%s' no soportado nativamente."
                  " Usando raw...\n", signal.protocolName().c_str());
    return _sendRaw(signal);
}

// ─────────────────────────────────────────────────────────────
//  _sendByProtocol()  [private]
//  Emisión nativa según decode_type_t.
//  Cubre los protocolos más comunes en electrónica de consumo.
//  Para protocolos de A/C (estado completo) se usa sendRaw
//  porque requieren el estado completo del climatizador,
//  que no se puede reconstruir solo con value+bits.
// ─────────────────────────────────────────────────────────────
bool Sender::_sendByProtocol(const IRSignal& signal) {
    const uint64_t val  = signal.value;
    const uint16_t bits = signal.bits;

    switch (signal.protocol) {

        // ── Protocolos de 1 valor (TV, amplificadores, etc.) ──
        case NEC:
        case NEC_LIKE:
            _irsend.sendNEC(val, bits);
            break;

        case SAMSUNG:
            _irsend.sendSAMSUNG(val, bits);
            break;

        case SONY:
            // Sony requiere 3 repeticiones por especificación
            _irsend.sendSony(val, bits, /* repeat */ 3);
            break;

        case RC5:
            _irsend.sendRC5(val, bits);
            break;

        case RC6:
            _irsend.sendRC6(val, bits);
            break;

        case LG:
        case LG2:
            _irsend.sendLG(val, bits);
            break;

        case PANASONIC:
            _irsend.sendPanasonic(signal.address, val, bits);
            break;

        case JVC:
            _irsend.sendJVC(val, bits, /* repeat */ 1);
            break;

        case SHARP:
        case SHARP_AC:
            _irsend.sendSharpRaw(val, bits);
            break;

        case DENON:
            _irsend.sendDenon(val, bits);
            break;

        case PIONEER:
            _irsend.sendPioneer(val, bits);
            break;

        case WHYNTER:
            _irsend.sendWhynter(val, bits);
            break;

        case COOLIX:
        case COOLIX48:
            _irsend.sendCOOLIX(val, bits);
            break;

        case DAIKIN:
        case DAIKIN2:
        case DAIKIN160:
        case DAIKIN176:
        case DAIKIN128:
        case DAIKIN152:
        case DAIKIN64:
            // Daikin A/C: estado completo no recuperable desde value,
            // delegar a raw
            return false;

        case MITSUBISHI_AC:
        case MITSUBISHI112:
        case MITSUBISHI136:
            // Mitsubishi A/C: ídem
            return false;

        case KELVINATOR:
            return false;

        // ── Tiras LED y mandos genéricos ──────────────────────
        case SYMPHONY:
            _irsend.sendSymphony(val, bits);
            break;

        case EPSON:
            _irsend.sendEpson(val, bits);
            break;

        case PRONTO:
            // Pronto ya viene en raw; usar _sendRaw
            return false;

        default:
            // Protocolo no listado → fallback a raw
            return false;
    }

    Serial.printf("[Sender] Emitido: protocolo=%s valor=%s bits=%u\n",
                  signal.protocolName().c_str(),
                  signal.valueHex().c_str(),
                  bits);
    return true;
}

// ─────────────────────────────────────────────────────────────
//  _sendRaw()  [private]
//  Emisión universal desde rawData.
//  Funciona con cualquier protocolo capturado, incluso
//  los desconocidos o los de A/C con estado completo.
// ─────────────────────────────────────────────────────────────
bool Sender::_sendRaw(const IRSignal& signal) {
    if (signal.rawData.empty()) {
        Serial.println(F("[Sender] ERROR: rawData vacío, no se puede emitir."));
        return false;
    }

    // IRsend::sendRaw espera uint16_t*, los valores ya están en µs
    _irsend.sendRaw(
        signal.rawData.data(),
        static_cast<uint16_t>(signal.rawData.size()),
        static_cast<uint16_t>(signal.frequencyHz / 1000)   // kHz
    );

    Serial.printf("[Sender] Emitido (raw): %u muestras @ %u kHz\n",
                  signal.rawData.size(),
                  signal.frequencyHz / 1000);
    return true;
}
