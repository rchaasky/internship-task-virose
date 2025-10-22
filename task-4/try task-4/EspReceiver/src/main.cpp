#include "ESPReceiver.h"

ESPReceiver receiver;

// Callback ESP-NOW
void onData(const uint8_t *mac_addr, const uint8_t *data, int len) {
    receiver.onDataReceived(mac_addr, data, len);
}

// Implementasi begin
void ESPReceiver::begin() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    // Inisialisasi SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Gagal mount SPIFFS");
        return;
    }

    // Inisialisasi ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW gagal diinisialisasi");
        return;
    }

    // Register callback
    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len) {
        receiver.onDataReceived(mac, data, len);
    });

    Serial.println("ESP-Receiver siap menerima data");
}

// Implementasi parseMessage
void ESPReceiver::parseMessage(const String &msg) {
    int sep = msg.indexOf('|');
    if (sep != -1) {
        Chunk c;
        c.number = msg.substring(0, sep).toInt();
        c.data = msg.substring(sep + 1);
        chunks.push_back(c);
        Serial.println("[Chunk diterima]");
    }
}

// Implementasi onDataReceived
void ESPReceiver::onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
    String msg = "";
    for (int i = 0; i < len; i++) msg += (char)data[i];

    // END menandai chunk terakhir
    bool isEnd = msg.endsWith("|END");
    if (isEnd) msg.replace("|END", "");

    parseMessage(msg);

    // simpan chunk ke SPIFFS
    saveToSPIFFS();

    if (isEnd) {
        printJSON();
        chunks.clear();
        SPIFFS.remove("/received.json");
    }
}

// Implementasi saveToSPIFFS
void ESPReceiver::saveToSPIFFS() {
    File file = SPIFFS.open("/received.json", FILE_APPEND);
    if (file) {
        for (auto &c : chunks) {
            file.println(c.data);
        }
        file.close();
    } else {
        Serial.println("Gagal menulis ke SPIFFS");
    }
}

void ESPReceiver::printJSON() {
    // Gabungkan semua chunk
    String fullData = "";
    for (auto &c : chunks) fullData += c.data;

    int n1 = fullData.indexOf("\"nama\":");
    int n2 = fullData.indexOf(",", n1);
    int j1 = fullData.indexOf("\"jurusan\":");
    int j2 = fullData.indexOf(",", j1);
    int u1 = fullData.indexOf("\"umur\":");
    int u2 = fullData.indexOf(",", u1);
    int d1 = fullData.indexOf("\"deskripsi\":");
    int d2 = fullData.indexOf("}", d1);

    Serial.println("[KONTEN FILE YANG DITERIMA]");
    Serial.print("NAMA: "); Serial.println(fullData.substring(n1 + 8, n2 - 1));
    Serial.print("JURUSAN: "); Serial.println(fullData.substring(j1 + 11, j2 - 1));
    Serial.print("UMUR: "); Serial.println(fullData.substring(u1 + 7, u2));
    Serial.print("DESKRIPSI DIRI: "); Serial.println(fullData.substring(d1 + 14, d2 - 1));
}

void setup() {
    receiver.begin();
}

void loop() {
}