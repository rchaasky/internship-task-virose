#include "Laptop.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>
#include <windows.h> // untuk serial dan Sleep

Laptop::Laptop(const std::string& jsonFile) : filename(jsonFile) {}

// Membaca file JSON dan mengambil semua field
void Laptop::readJSON() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "File " << filename << " tidak ditemukan!\n";
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();

    // Fungsi untuk mengambil value dari key tertentu
    auto getValue = [&](const std::string& key) -> std::string {
        size_t start = jsonContent.find(key);
        if (start == std::string::npos) return "";
        start = jsonContent.find(":", start) + 1;

        while (start < jsonContent.size() && isspace(jsonContent[start])) start++;

        if (jsonContent[start] == '"') { // string
            start++;
            size_t end = start;
            while (end < jsonContent.size()) {
                if (jsonContent[end] == '"' && jsonContent[end-1] != '\\') break;
                end++;
            }
            return jsonContent.substr(start, end - start);
        } else { // angka
            size_t end = jsonContent.find_first_of(",}", start);
            return jsonContent.substr(start, end - start);
        }
    };

    nama = getValue("nama");
    jurusan = getValue("jurusan");
    umur = std::stoi(getValue("umur"));
    deskripsi = getValue("deskripsi");
}

// Pecah data menjadi chunk (DESKRIPSI per kata, max 85 karakter)
void Laptop::splitData() {
    chunks.clear();
    chunks.push_back("NAMA:" + nama);
    chunks.push_back("JURUSAN:" + jurusan);
    chunks.push_back("UMUR:" + std::to_string(umur));

    // Pecah deskripsi
    const size_t maxChunk = 85;
    std::string currentChunk = "";
    std::istringstream iss(deskripsi);
    std::string word;
    while (iss >> word) {
        if (currentChunk.length() + word.length() + 1 > maxChunk) {
            chunks.push_back("DESKRIPSI:" + currentChunk);
            currentChunk = word;
        } else {
            if (!currentChunk.empty()) currentChunk += " ";
            currentChunk += word;
        }
    }
    if (!currentChunk.empty())
        chunks.push_back("DESKRIPSI:" + currentChunk);
}

// Kirim semua chunk ke ESP-Bridge via serial
void Laptop::sendToESPBridge() {
    const char* comPort = "COM6"; // ganti sesuai port ESP-Bridge
    HANDLE hSerial = CreateFile(comPort, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Gagal membuka serial port " << comPort << "!\n";
        return;
    }

    for (const auto& chunk : chunks) {
        DWORD bytesWritten;
        std::string toSend = chunk + "\n";
        if (!WriteFile(hSerial, toSend.c_str(), (DWORD)toSend.size(), &bytesWritten, NULL)) {
            std::cerr << "Gagal mengirim data ke ESP-Bridge\n";
        } else {
            std::cout << "[Sent] " << chunk << std::endl;
        }
        Sleep(100); // delay singkat, ganti std::this_thread::sleep_for jika cross-platform
    }

    CloseHandle(hSerial);
}
