# test/

Esta carpeta contiene los tests unitarios del proyecto.

PlatformIO usa el framework **Unity** para tests en dispositivo.

Los tests se ejecutan con:
```
pio test
```

**Tests previstos:**
- `test_irsignal.cpp`   — Validación del struct IRSignal
- `test_storage.cpp`    — Tests de Storage con mock de LittleFS
- `test_console.cpp`    — Tests del parser de consola
