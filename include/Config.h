// ============================================================
//  IRManager — include/Config.h
//  Constantes de compilación globales del proyecto.
//  NUNCA incluir lógica aquí. Solo #define y constexpr.
// ============================================================
#pragma once

// ---- Versión del firmware ----------------------------------
#define FW_VERSION      "0.1.0"
#define PROJECT_NAME    "IRManager"

// ---- Pines de hardware -------------------------------------
constexpr uint8_t IR_RECV_PIN  = 15;   // TSOP4838
constexpr uint8_t IR_SEND_PIN  = 4;    // LED IR + transistor (reservado)

// ---- Receptor IR -------------------------------------------
// Tamaño del buffer de captura raw (número de marcas/espacios)
constexpr uint16_t IR_RECV_BUF_SIZE = 1024;
// Timeout de señal en µs (50 ms es estándar para la mayoría de protocolos)
constexpr uint8_t  IR_RECV_TIMEOUT  = 50;

// ---- Comunicación serie ------------------------------------
constexpr uint32_t SERIAL_BAUD = 115200;
// Tiempo máximo de espera del puerto USB-CDC en ms
constexpr uint32_t SERIAL_TIMEOUT_MS = 5000;

// ---- LittleFS / Almacenamiento -----------------------------
// Versión del esquema JSON de cada botón (para migraciones futuras)
constexpr uint8_t JSON_SCHEMA_VERSION = 1;
// Frecuencia IR por defecto cuando el protocolo la especifica
constexpr uint32_t IR_DEFAULT_FREQ_HZ = 38000;
