// ============================================================
//  IRManager — src/Console.h
//  v0.2 — Consola con learn, list, devices, buttons, delete,
//          status, restart. Patrón: estado PendingLearn que
//          main.cpp consume de forma no bloqueante.
// ============================================================
#pragma once

#include <Arduino.h>

class Storage;
class Receiver;
class Sender;

// ─────────────────────────────────────────────────────────────
//  PendingLearn
//  Cuando el usuario escribe "learn Samsung POWER", la consola
//  no captura la señal directamente (sería bloqueante).
//  En cambio, publica este estado para que main.cpp lo consuma
//  en el próximo ciclo con señal IR válida.
// ─────────────────────────────────────────────────────────────
struct PendingLearn {
    bool   active = false;
    String device;
    String button;
};

// ─────────────────────────────────────────────────────────────
//  Console
// ─────────────────────────────────────────────────────────────
class Console {
public:
    Console(Storage* storage, Receiver* receiver, Sender* sender);

    // Imprime el banner de bienvenida al arrancar.
    void printBanner();

    // Debe llamarse en cada iteración del loop de Arduino.
    void loop();

    // ── Estado PendingLearn (interfaz con main.cpp) ──────────
    // Devuelve el estado de la operación "learn" pendiente.
    const PendingLearn& getPendingLearn() const { return _pending; }

    // Llamar desde main.cpp DESPUÉS de haber procesado la señal.
    void clearPendingLearn() { _pending = PendingLearn{}; }

    // Imprime el resultado de un learn completado.
    void notifyLearnSuccess(const String& device, const String& button);
    void notifyLearnFailed (const String& device, const String& button);

private:
    Storage*    _storage;
    Receiver*   _receiver;
    Sender*     _sender;

    String      _inputBuffer;
    PendingLearn _pending;

    // ── Dispatcher ───────────────────────────────────────────
    void _dispatch(const String& line);

    // ── Handlers de cada comando ─────────────────────────────
    void _cmdLearn  (const String& args);
    void _cmdSend   (const String& args);
    void _cmdList   ();
    void _cmdDevices();
    void _cmdButtons(const String& args);
    void _cmdDelete (const String& args);
    void _cmdStatus ();
    void _cmdRestart();
    void _cmdHelp   ();

    // ── Utilidades ───────────────────────────────────────────
    // Extrae el primer token y devuelve el resto como "args".
    static String _firstToken(const String& line, String& rest);
};
