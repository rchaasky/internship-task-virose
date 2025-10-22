#ifndef ESPBRIDGE_H
#define ESPBRIDGE_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

class ESPBridge {
private:
    uint8_t receiver_mac[6]; // MAC ESP-Receiver

    void sendESPNow(const String &msg); // Kirim chunk via ESP-NOW

public:
    void begin();          // Inisialisasi Serial & ESP-NOW
    void receiveSerial();  // Baca chunk dari laptop dan teruskan
};

#endif