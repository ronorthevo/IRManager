# data/

Esta carpeta contiene los ficheros de la interfaz web (HTML, CSS, JS).
Se sube al ESP32 mediante el comando:

```
pio run --target uploadfs
```

Los ficheros de esta carpeta son servidos por el módulo `WebServer`
directamente desde LittleFS.

**Contenido previsto (v0.6):**
- `index.html`  — Interfaz web principal
- `style.css`   — Estilos (tema oscuro, responsive)
- `app.js`      — Lógica de la interfaz (comunicación REST)
