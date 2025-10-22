#ifndef ESPRECEIVER_H
#define ESPRECEIVER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <vector>

struct Chunk {
    int number;    // urutan chunk
    String data;   // isi chunk
};

class ESPReceiver {
private:
    std::vector<Chunk> chunks;
    void parseMessage(const String &msg);

public:
    void begin();  // inisialisasi Serial, ESP-NOW & SPIFFS
    void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len);
    void saveToSPIFFS();
    void printJSON();
};

#endif