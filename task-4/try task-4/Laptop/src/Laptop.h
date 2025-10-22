#pragma once
#include <string>
#include <vector>

class Laptop {
public:
    Laptop(const std::string& jsonFile);

    void readJSON();        // Baca JSON dari file
    void splitData();       // Pecah menjadi chunk (per kata max 85)
    void sendToESPBridge(); // Kirim ke ESP-Bridge via serial

private:
    std::string filename;
    std::string nama;
    std::string jurusan;
    int umur;
    std::string deskripsi;
    std::vector<std::string> chunks;
};
