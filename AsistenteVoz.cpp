#include "AsistenteVoz.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/CoreAudio/AudioHttp/URLStream.h"

using namespace audio_tools;

URLStream url;
I2SStream i2s;
MP3DecoderHelix decoder;
EncodedAudioStream dec(&i2s, &decoder);
StreamCopy copier(dec, url);

Str queryBase("https://translate.google.com/translate_tts?ie=UTF-8&tl=%1&client=tw-ob&ttsspeed=%2&q=%3");
Str finalQuery;

AsistenteVoz::AsistenteVoz(const char* ssid, const char* password, const char* token, const char* modelo,
                           int pin_bclk, int pin_lrc, int pin_dout)
    : _ssid(ssid), _password(password), _token(token), _modelo(modelo),
      _pin_bclk(pin_bclk), _pin_lrc(pin_lrc), _pin_dout(pin_dout) {}

void AsistenteVoz::iniciar() {
    Serial.begin(115200);
    AudioLogger::instance().begin(Serial, AudioLogger::Warning); // coloca Info al final en vez de warning si desea que se vea la info en M serial
    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Conectado a WiFi");

    auto config = i2s.defaultConfig(TX_MODE);
    config.pin_bck  = _pin_bclk;
    config.pin_ws   = _pin_lrc;
    config.pin_data = _pin_dout;
    config.sample_rate = 22050;
    i2s.begin(config);
    dec.begin();
}

void AsistenteVoz::decir(const String& texto) {
    fragmentarYReproducirTexto(texto);
}

void AsistenteVoz::preguntar(const String& mensaje) {
    String respuesta = limpiarTexto(enviarPregunta(mensaje));
    Serial.println("🤖 Asistente:\n" + respuesta);
    fragmentarYReproducirTexto(respuesta);
}

String AsistenteVoz::construirPayload(const String& mensaje) {
    DynamicJsonDocument doc(1024);
    doc["model"] = _modelo;
    JsonArray mensajes = doc.createNestedArray("messages");
    JsonObject userMsg = mensajes.createNestedObject();
    userMsg["role"] = "user";
    userMsg["content"] = mensaje;

    String payload;
    serializeJson(doc, payload);
    return payload;
}

String AsistenteVoz::enviarPregunta(const String& mensaje, int reintentos) {
    for (int intento = 1; intento <= reintentos; intento++) {
        WiFiClientSecure client;
        client.setInsecure();
        client.setTimeout(8000);

        if (!client.connect("openrouter.ai", 443)) {
            delay(1000);
            continue;
        }

        String payload = construirPayload(mensaje);
        String request = "POST /api/v1/chat/completions HTTP/1.1\r\n";
        request += "Host: openrouter.ai\r\n";
        request += "Authorization: Bearer " + String(_token) + "\r\n";
        request += "Content-Type: application/json\r\n";
        request += "Content-Length: " + String(payload.length()) + "\r\n";
        request += "Connection: close\r\n\r\n";
        request += payload;

        client.print(request);

        unsigned long lastDataTime = millis();
        while (client.connected() || client.available()) {
            while (client.available()) {
                String line = client.readStringUntil('\n');
                lastDataTime = millis();
                if (line.startsWith("{")) {
                    DynamicJsonDocument doc(8192);
                    if (deserializeJson(doc, line) == DeserializationError::Ok) {
                        return doc["choices"][0]["message"]["content"].as<String>();
                    } else {
                        return "❌ Error al procesar respuesta JSON";
                    }
                }
            }
            if (millis() - lastDataTime > 8000) break;
        }

        client.stop();
    }
    return "❌ Todos los intentos fallaron.";
}

String AsistenteVoz::limpiarTexto(const String& entrada) {
    String salida = "";
    for (int i = 0; i < entrada.length(); i++) {
        char c = entrada[i];
        if (c != '*' && c != '#') salida += c;
    }
    return salida;
}

const char* AsistenteVoz::generarURL(const char* texto, const char* lang, const char* speed) {
    finalQuery = queryBase;
    finalQuery.replace("%1", lang);
    finalQuery.replace("%2", speed);
    Str encoded(texto);
    encoded.urlEncode();
    finalQuery.replace("%3", encoded.c_str());
    return finalQuery.c_str();
}

void AsistenteVoz::fragmentarYReproducirTexto(const String& texto) {
    const int maxLong = 100;
    int inicio = 0;

    while (inicio < texto.length()) {
        int fin = inicio + maxLong;
        if (fin < texto.length()) {
            int punto = texto.lastIndexOf('.', fin);
            int coma = texto.lastIndexOf(',', fin);
            int espacio = texto.lastIndexOf(' ', fin);
            if (punto > inicio) fin = punto + 1;
            else if (coma > inicio) fin = coma + 1;
            else if (espacio > inicio) fin = espacio;
        } else {
            fin = texto.length();
        }

        String fragmento = texto.substring(inicio, fin);
        const char* url_str = generarURL(fragmento.c_str());
        url.begin(url_str, "audio/mp3");

        // Verificar si la conexión se abrió correctamente y si hay datos disponibles.
        if (url.available()) {
        Serial.println("✅ Conexión con la fuente de audio establecida.");
        unsigned long timeout = millis() + 11000; // Timeout de 11 segundos.

        // 🔄 Mantener el flujo activo mientras hay datos y el tiempo no se agote.
        while (url.available() && millis() < timeout) {
            size_t copied = copier.copy();
            if (copied == 0) {
                delay(50); // espera si no hay datos
            }
        }
        
        // Si el bucle termina por timeout, se registrará un mensaje en el Serial.
        if (millis() >= timeout) {
            Serial.println("⚠️ Timeout: La transmisión de audio se interrumpió.");
        }

        } else {
        // Si la URL no está disponible, informamos del error y no intentamos copiar.
        Serial.println("❌ No se pudo conectar a la fuente de audio. Saltando reproducción.");
        }
        
        url.end(); // Cerrar la conexión para liberar recursos.

        inicio = fin;
        while (texto[inicio] == ' ') inicio++; // Saltar espacios sobrantes
    }
}