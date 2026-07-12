// ============================================================
//  IRManager — src/Console.h
//  Responsabilidad única: parser de comandos por puerto serie.
//  Patrón: Command Dispatcher — cada comando es independiente.
//  [STUB v0.1] — Solo impresión del banner. Implementación
//  completa de comandos en v0.4.
// ============================================================
#pragma once

#include <Arduino.h>

// Forward declarations para evitar dependencias circulares en v0.4
class Storage;
class Receiver;
class Sender;

class Console {
public:
    // storage, receiver, sender: punteros a las instancias del sistema.
    // En v0.1 solo se usan para el banner.
    Console(Storage* storage, Receiver* receiver, Sender* sender);

    // Imprime el banner de bienvenida al arrancar.
    void printBanner();

    // Debe llamarse en cada iteración del loop de Arduino.
    // Lee líneas del puerto serie y despacha comandos.
    void loop();

private:
    Storage*  _storage;
    Receiver* _receiver;
    Sender*   _sender;

    String    _inputBuffer;

    // Procesa una línea completa de comando.
    void _dispatch(const String& line);

    // Imprime la ayuda de comandos disponibles.
    void _printHelp();
};
