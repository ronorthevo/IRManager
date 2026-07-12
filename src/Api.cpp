// ============================================================
//  IRManager — src/Api.cpp
//  [STUB v0.1] — Implementación completa en v0.5.
// ============================================================
#include "Api.h"

Api::Api(AsyncWebServer* server, Storage* storage,
         Receiver* receiver, Sender* sender)
    : _server(server)
    , _storage(storage)
    , _receiver(receiver)
    , _sender(sender)
{
}

void Api::registerRoutes() {
    // TODO v0.5: registrar handlers sobre _server
}

void Api::_handleStatus()      { /* TODO v0.5 */ }
void Api::_handleGetDevices()  { /* TODO v0.5 */ }
void Api::_handleGetButtons()  { /* TODO v0.5 */ }
void Api::_handleGetButton()   { /* TODO v0.5 */ }
void Api::_handleLearn()       { /* TODO v0.5 */ }
void Api::_handleSend()        { /* TODO v0.5 */ }
void Api::_handleDeleteButton(){ /* TODO v0.5 */ }
