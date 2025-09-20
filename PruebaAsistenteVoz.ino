#include "AsistenteVoz.h"

// 🔐 Configuración WiFi y API
const char* ssid     = "ROBOTICA_LUZ";
const char* password = "Esp32-2024";
const char* token    = "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";
const char* modelo   = "deepseek/deepseek-r1-0528-qwen3-8b:free"; // Puedes usar "gpt-3.5-turbo", "mistral", etc.

// 🎚️ Pines I2S configurables
const int PIN_BCLK = 33;
const int PIN_LRC  = 32;
const int PIN_DOUT = 25;

/*// Pines I2S para ESP32-C3 supermini
#define I2S_BCLK 6
#define I2S_LRC  7
#define I2S_DOUT 5
*/

// 🎙️ Crear instancia del asistente
AsistenteVoz asistente(ssid, password, token, modelo, PIN_BCLK, PIN_LRC, PIN_DOUT);

String bufferPregunta = "";

void setup() {
  Serial.begin(115200);
  asistente.iniciar();
  asistente.decir("Hola. Escribe tu pregunta en el monitor serial.");
  asistente.decir("Si solo les mostramos amor a algunos hermanos de la congregación, no estamos obedeciendo lo que dijo Jesús sobre el amor. Claro, puede que tengamos una amistad más estrecha con algunos de nuestros hermanos, tal como le pasó a Jesús con algunos de sus discípulos");
}

void loop() {
  // Leer entrada del usuario desde el monitor serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      bufferPregunta.trim();
      if (bufferPregunta.length() > 0) {
        asistente.preguntar(bufferPregunta);
        bufferPregunta = "";
        Serial.println("\n✅ Puedes escribir otra pregunta:");
      }
    } else {
      bufferPregunta += c;
    }
  }
}