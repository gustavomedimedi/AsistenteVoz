# AsistenteVoz
Una biblioteca didáctica para interactuar por voz con modelos de lenguaje (GPT, Mistral, etc.) desde un ESP32-C3 Mini. Ideal para proyectos educativos, accesibles y creativos.
¡Aquí tienes, Gustavo! Esta es la guía de uso de la biblioteca `AsistenteVoz` lista para incluir en tu repositorio o compartir con tus estudiantes como archivo de referencia:

---
# 📘 Guía de obtener API de Openrouter

### Para obtener la API KEY primero registrate en OPENROUTER (https://openrouter.ai) y luego vas a Modelos (https://openrouter.ai/models) , eliges el modelo que prefieras. Despues vas a la seccion de Crear API y luego en CURL de la Api para copiar el modelo el cual usaras en el codigo que deseas implementar la Biblioteca.

# 📘 Guía de uso — Biblioteca `AsistenteVoz` para ESP32-C3 Mini

### 🎯 ¿Qué es?
Una biblioteca didáctica que permite a estudiantes interactuar por voz con un modelo de lenguaje (como GPT o Mistral) desde un ESP32-C3 Mini. Se conecta a WiFi, envía preguntas vía OpenRouter y reproduce respuestas habladas usando Google TTS.

---

## 🧩 Constructor

```cpp
AsistenteVoz asistente(ssid, password, token, modelo, pin_bclk, pin_lrc, pin_dout);
```

- `ssid`: nombre de la red WiFi.
- `password`: contraseña de la red.
- `token`: API key de OpenRouter.
- `modelo`: nombre del modelo (ej. `"mistral"`, `"gpt-3.5-turbo"`).
- `pin_bclk`, `pin_lrc`, `pin_dout`: pines I2S para salida de audio.

---

## 🔧 Funciones principales

### `void iniciar();`
- Conecta a WiFi.
- Inicializa el sistema de audio.
- Debe llamarse en `setup()`.

### `void decir(String texto);`
- Reproduce el texto directamente como voz.
- Usa Google TTS en español latino (`es-419`).

### `void preguntar(String mensaje);`
- Envía el mensaje al modelo de lenguaje.
- Imprime la respuesta en el Monitor Serial.
- Reproduce la respuesta como voz.

---

## 🖥️ Ejemplo básico

```cpp
void setup() {
  asistente.iniciar();
  asistente.decir("Hola, bienvenido.");
  asistente.preguntar("¿Qué es la robótica educativa?");
}
```

---

## 🖨️ Ejemplo con entrada por Monitor Serial

```cpp
String bufferPregunta = "";

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      bufferPregunta.trim();
      if (bufferPregunta.length() > 0) {
        asistente.preguntar(bufferPregunta);
        bufferPregunta = "";
      }
    } else {
      bufferPregunta += c;
    }
  }
}
```

---

## 🛠️ Recomendaciones

- Usa fragmentos cortos de texto (máx. 100 caracteres por bloque).
- Verifica que los pines I2S estén bien conectados.
- Puedes ajustar el idioma o velocidad de voz modificando `generarURL()` en el archivo `.cpp`.
- Para evitar reinicios, monitorea el uso de memoria con `ESP.getFreeHeap()` y usa `AudioLogger::Warning` o `None`.

---

## 📦 Archivos incluidos

- `AsistenteVoz.h`: definición de la clase.
- `AsistenteVoz.cpp`: implementación completa.
- `main.ino`: ejemplo de uso básico.
- `main_serial.ino`: ejemplo con entrada por Monitor Serial.

---

