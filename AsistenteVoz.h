#ifndef ASISTENTE_VOZ_H
#define ASISTENTE_VOZ_H

#include <Arduino.h>

class AsistenteVoz {
public:
    AsistenteVoz(const char* ssid, const char* password, const char* token, const char* modelo,
                 int pin_bclk, int pin_lrc, int pin_dout);

    void iniciar();
    void decir(const String& texto);
    void preguntar(const String& mensaje);

private:
    const char* _ssid;
    const char* _password;
    const char* _token;
    const char* _modelo;

    int _pin_bclk;
    int _pin_lrc;
    int _pin_dout;

    String construirPayload(const String& mensaje);
    String enviarPregunta(const String& mensaje, int reintentos = 2);
    String limpiarTexto(const String& entrada);
    const char* generarURL(const char* texto, const char* lang = "es-419", const char* speed = "1");
    void fragmentarYReproducirTexto(const String& texto);
};

#endif