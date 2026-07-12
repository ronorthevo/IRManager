# IRManager

Gestor universal de mandos infrarrojos basado en **ESP32-S3**.

## Hardware

| Componente | GPIO | Notas |
|---|---|---|
| TSOP4838 (receptor IR) | 15 | Activo bajo, pull-up interno |
| LED IR + transistor (emisor) | 4 | Reservado — v0.3 |

## Entorno

- PlatformIO + Arduino Framework
- ESP32-S3-DevKitC-1 (USB-CDC nativo)
- VS Code

## Librerías

| Librería | Versión | Uso |
|---|---|---|
| IRremoteESP8266 | ^2.8.6 | Captura y emisión IR |
| ArduinoJson | ^7.0.0 | Serialización de botones |
| ESPAsyncWebServer | ^3.x | Servidor web (v0.5+) |

## Arquitectura

```
src/
├── main.cpp          — Coordinador principal
├── IRSignal.h        — Struct POD de señal IR (header-only)
├── Receiver.cpp/h    — Captura IR
├── Sender.cpp/h      — Emisión IR (v0.3)
├── Storage.cpp/h     — LittleFS, un JSON por botón (v0.2)
├── Console.cpp/h     — Consola serie tipo Linux (v0.4)
├── WifiManager.cpp/h — Gestión WiFi AP+STA (v0.5)
├── WebServer.cpp/h   — Servidor HTTP estático (v0.6)
└── Api.cpp/h         — REST API (v0.5)
include/
└── Config.h          — Constantes de compilación
data/                 — Interfaz web (v0.6)
```

## Almacenamiento

Un fichero JSON por botón en LittleFS:

```
/Samsung/POWER.json
/Samsung/VOL+.json
/LG/POWER.json
```

Formato:
```json
{
  "schema_version": 1,
  "device": "Samsung",
  "button": "POWER",
  "protocol": "NEC",
  "value": "0xFF15EA",
  "bits": 32,
  "frequency": 38000,
  "address": 0,
  "command": 168,
  "raw": [9024, 4512, 564, ...]
}
```

## Consola serie

```
learn Samsung POWER    → aprende señal IR y la guarda
send Samsung POWER     → emite la señal guardada
list                   → lista todos los botones
devices                → lista dispositivos
buttons Samsung        → lista botones de Samsung
delete Samsung POWER   → borra un botón
status                 → estado del sistema
restart                → reinicia el ESP32
help                   → ayuda
```

## REST API (v0.5+)

```
GET    /api/devices
GET    /api/device/{name}/buttons
GET    /api/button/{device}/{button}
POST   /api/learn
POST   /api/send
DELETE /api/button/{device}/{button}
GET    /api/status
```

## Roadmap

| Versión | Contenido |
|---|---|
| v0.1 | Arquitectura base, receptor IR funcional ✅ |
| v0.2 | Storage — guardar/cargar/borrar botones |
| v0.3 | Sender — emisión IR |
| v0.4 | Console — todos los comandos |
| v0.5 | WiFi + REST API |
| v0.6 | Interfaz web responsive |
| v1.0 | Aplicación completa + OTA |

## Ramas

- `main` — releases estables
- `develop` — desarrollo activo

## Licencia

MIT
