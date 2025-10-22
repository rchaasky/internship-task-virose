#include "EspBridge.h"

// Buat instance
ESPBridge bridge;

// Implementasi begin()
void ESPBridge::begin() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    // Set MAC ESP-Receiver 
    receiver_mac[0] = 0x44;
    receiver_mac[1] = 0x1D;
    receiver_mac[2] = 0x64;
    receiver_mac[3] = 0xF6;
    receiver_mac[4] = 0xD3;
    receiver_mac[5] = 0x70;

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init gagal!");
        return;
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiver_mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Gagal menambahkan peer");
        return;
    }

    Serial.println("ESP-Bridge siap mengirim ke Receiver");
}

// Implementasi receiveSerial()
void ESPBridge::receiveSerial() {
    static String line = "";
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            if (line.length() > 0) {
                Serial.println("Meneruskan: " + line);
                sendESPNow(line);
                line = "";
            }
        } else {
            line += c;
        }
    }
}

// Implementasi sendESPNow()
void ESPBridge::sendESPNow(const String &msg) {
    esp_err_t result = esp_now_send(receiver_mac, (uint8_t *)msg.c_str(), msg.length());
    if (result == ESP_OK) {
        Serial.println("Chunk terkirim ke Receiver!");
    } else {
        Serial.println("Gagal kirim chunk");
    }
}

// Arduino setup dan loop
void setup() {
    bridge.begin();
}

void loop() {
    bridge.receiveSerial();
}