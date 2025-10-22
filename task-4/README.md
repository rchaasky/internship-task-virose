# Cross PLatform FIle Transfer ESP-NOW

## Deskripsi Umum
Program ini dibuat untuk mentransfer file JSON dari laptop ke ESP-Receiver melalui ESP-Bridge menggunakan ESP-NOW. Program harus memecah file JSON menjadi beberapa chunk agar sesuai batasan ESP-NOW 250 byte per packet, kemudian menyusun kembali di ESP-Receiver dan menampilkan data ke serial monitor.

## File.json
file .json pada proyek bernama data.json yang value deskripsi harus memiliki kata setidaknya 25 kata.
```cpp
{
    "nama": "Africha Sekar Wangi",
    "jurusan": "Sistem Informasi",
    "umur": 18,
    "deskripsi": "Saya adalah orang yang sangat soft spoken, tidak sombong, rendah hati, dan rajin menabung. Saya juga senang menonton film dan drama korea, tapi sekarang sudah sibuk coolyeah"
}
```
Setelah esp-receiver selesai menyusun data dari ESP-NOW, esp-receiver menampilkan data ke serial monitor dengan format:
```cpp
[KONTEN FILE YANG DITERIMA]
NAMA: ...
JURUSAN: ...
UMUR: ...
DESKRIPSI DIRI: ...
```
## Struktur proyek
```cpp
try task-4
│
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── extensions.json
│   └── launch.json
│
├── EspBridge/
│   ├── pio/
│   ├── .vscode/
│   ├── src/
│   │   ├── EspBridge.h
│   │   └── main.cpp
│   ├── .gitignore
│   └── platformio.ini
│
├── EspReceiver/
│   ├── pio/
│   ├── .vscode/
│   ├── src/
│   │   ├── EspReceiver.h
│   │   └── main.cpp
│   ├── .gitignore
│   └── platformio.ini
│
├── include/
│   └── README
│
├── laptop/
│   ├── build/
│   ├── data/
│   │   └── data.json
│   ├── src/
│   │   ├── Laptop.h
│   │   ├── Laptop.cpp
│   │   └── main.cpp
│   └── CMakeLists.txt
│
└── lib/
    └── README
```
# Alur Program
Laptop:
- Membaca JSON dari `data.json` menggunakan `Laptop::readJSON()`.
- Memecah data menjadi chunk dengan `Laptop::splitData()`. Khusus DESKRIPSI, dipisahkan per kata agar tidak lebih dari 85 karakter per chunk dan per kata tidak terpotong.
- Mengirim setiap chunk ke ESP-Bridge via serial `(Laptop::sendToESPBridge())`.
ESP-Bridge:
- Menerima serial dari Laptop `(ESPBridge::receiveSerial())`.
- Meneruskan setiap chunk ke ESP-Receiver melalui ESP-NOW `(ESPBridge::sendESPNow())`.
ESP-Receiver:
- Menerima chunk melalui ESP-NOW di callback `onDataReceived()`.
- Menyimpan semua chunk ke memory sementara `(parseMessage())`.
- Setelah menerima semua chunk `(|END)`, menyusun kembali data menjadi JSON utuh dan menampilkan di Serial Monitor `(printJSON())`.

# Bagian Laptop
**Laptop.h**
```cpp
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
```
Di dalam Laptop.h, terdapat tiga fungsi utama:
1. `readJSON()` → membaca isi file .json dan mengambil nilai-nilai seperti nama, jurusan, umur, dan deskripsi.
2. `splitData()` → memecah isi data terutama bagian deskripsi menjadi beberapa potongan teks berukuran lebih kecil yaitu maksimal sekitar 85 karakter supaya aman dikirim ke ESP.
3. `sendToESPBridge()` → mengirimkan potongan-potongan data tersebut ke ESP-Bridge melalui port serial.
   
**Laptop.cpp**
Alur Kerja
1. Membuka file JSON → ambil nama, jurusan, umur, deskripsi.
2. Pecah data menjadi chunk, terutama deskripsi agar ≤ 85 karakter per chunk.
3. Kirim chunk satu per satu ke ESP-Bridge melalui serial.

```cpp
#include "Laptop.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>
#include <windows.h>
```
- `Laptop.h` → Header milik class ini sendiri yang berisi deklarasi fungsi dan variabel.
- `fstream` → Untuk membaca file  data.json.
- `iostream` → Untuk menampilkan pesan ke terminal (cout, cerr).
- `sstream` → Untuk membaca teks dari memori, seperti parsing data.
- `vector` → Menyimpan potongan data (chunks) hasil pemecahan.
- `cctype` → Digunakan untuk pengecekan karakter seperti isspace().
- `windows.h` → Library Windows yang menyediakan fungsi komunikasi serial (CreateFile, WriteFile, Sleep).
```cpp
Laptop::Laptop(const std::string& jsonFile) : filename(jsonFile) {}
```
Konstruktor menerima path file JSON (jsonFile) dan menyimpannya di member filename.
```cpp
void Laptop::readJSON() {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();
    
    auto getValue = [&](const std::string& key) -> std::string { /* ambil value */ };
    nama = getValue("nama");
    jurusan = getValue("jurusan");
    umur = std::stoi(getValue("umur"));
    deskripsi = getValue("deskripsi");
}
```
Fungsi `readJSON()` yaitu membuka file JSON apabila gagal akan menampilkan error, membaca seluruh konten file ke dalam `std::string`, mengambil nilai masing-masing field (nama, jurusan, umur, deskripsi) menggunakan fungsi `getValue` kemudian hasilnya disimpan.

```cpp
void Laptop::splitData() {
    chunks.clear();
    chunks.push_back("NAMA:" + nama);
    chunks.push_back("JURUSAN:" + jurusan);
    chunks.push_back("UMUR:" + std::to_string(umur));

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
    if (!currentChunk.empty()) chunks.push_back("DESKRIPSI:" + currentChunk);
}
```
Fungsi `splitData()` yaitu membersihkan vector chunks agar kosong sebelum digunakan, menambahkan field NAMA, JURUSAN, dan UMUR ke dalam vector sebagai chunk, memecah deskripsi menjadi beberapa chunk per kata agar panjang tiap chunk ≤ 85 karakter, dan menambahkan setiap chunk deskripsi ke vector chunks.

```cpp
void Laptop::sendToESPBridge() {
    HANDLE hSerial = CreateFile("COM6", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    for (const auto& chunk : chunks) {
        DWORD bytesWritten;
        WriteFile(hSerial, (chunk + "\n").c_str(), chunk.size()+1, &bytesWritten, NULL);
        Sleep(100);
    }
    CloseHandle(hSerial);
}
```
Fungsi `sendToESPBridge()` yaitu membuka COM port untuk komunikasi serial dengan ESP-Bridge, memeriksa apakah port berhasil dibuka, jika gagal, menampilkan error, mengirim setiap chunk dari vector chunks ke ESP-Bridge, memberikan delay singkat agar ESP-Bridge bisa menerima data dengan stabil.menutup COM port setelah pengiriman selesai.

**main.cpp**
Alur kerja:
1. Inisialisasi objek Laptop dengan file JSON yang sudah dibuat (data/data.json).
2. Panggil `readJSON()` untuk membaca file dan mengambil field nama, jurusan, umur, dan deskripsi.
3. Panggil `splitData()` untuk memecah data menjadi beberapa chunk agar deskripsi panjang tetap terjaga per kata ≤ 85 karakter.
4. Panggil `sendToESPBridge()` untuk mengirim seluruh chunk ke ESP-Bridge via COM port.
```cpp
#include "Laptop.h"

int main() {
    Laptop laptop("data/data.json");  // Buat objek Laptop dengan file JSON

    laptop.readJSON();    // Baca file JSON dan ambil semua field
    laptop.splitData();   // Pecah data menjadi chunk agar bisa dikirim
    laptop.sendToESPBridge();  // Kirim chunk ke ESP-Bridge via serial
    
    return 0;
}
```
# Bagian Esp Bridge
**EspBridge.h**
```cpp
#ifndef ESPBRIDGE_H
#define ESPBRIDGE_H
```
Mencegah file header di-include lebih dari sekali dalam satu proyek, menghindari error duplikasi.
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
```
- `Arduino.h` → menyediakan fungsi dasar Arduino, termasuk Serial.
- `WiFi.h` → digunakan untuk mengatur mode WiFi ESP32.
- `esp_now.h` → protokol ESP-NOW, digunakan untuk komunikasi satu arah ke ESP-Receiver.
```cpp
class ESPBridge {
private:
    uint8_t receiver_mac[6]; // MAC ESP-Receiver
    void sendESPNow(const String &msg); // Kirim chunk via ESP-NOW

public:
    void begin();          // Inisialisasi Serial & ESP-NOW
    void receiveSerial();  // Baca chunk dari laptop dan teruskan
};
```
- a. Private 
  - `receiver_mac[6]` → menyimpan MAC address ESP-Receiver, sebagai tujuan pengiriman ESP-NOW.
  - `sendESPNow(const String &msg)` → fungsi internal untuk mengirim satu chunk ke ESP-Receiver dan hanya bisa dipanggil dari dalam kelas.
- b. Public
  - `begin()` → menyalakan serial monitor (Serial.begin) untuk debugging dan komunikasi dengan laptop, mengatur ESP32 ke mode WiFi STA, menginisialisasi ESP-NOW, dan menambahkan peer (ESP-Receiver) dengan MAC yang telah disimpan.
  - `receiveSerial()` → mengecek serial buffer, membaca tiap karakter hingga menemukan \n sebagai penanda akhir chunk, dan memanggil sendESPNow() untuk meneruskan chunk ke ESP-Receiver.

**main.cpp**
```cpp
ESPBridge bridge;
```
Membuat objek bridge dari kelas ESPBridge yang digunakan untuk memanggil fungsi-fungsi seperti begin() dan receiveSerial().
```cpp
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
```
Fungsi ini dijalankan sekali saat ESP-Bridge dinyalakan.
1. `Serial.begin(115200)` → memulai komunikasi serial dengan baud rate 115200 untuk debugging.
2. `WiFi.mode(WIFI_STA)` → ESP32 menjadi station mode, hanya bisa menerima/mengirim data ke perangkat lain.
3. `MAC address` → menyimpan MAC ESP-Receiver tujuan pengiriman ESP-NOW.
4. `esp_now_init()` → inisialisasi ESP-NOW.
5. `esp_now_add_peer()` → menambahkan peer tujuan agar bisa dikirim data.
```cpp
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
```
Mengecek apakah ada data masuk di serial port.Membaca karakter satu per satu, menyusun menjadi satu line hingga menemukan \n. Setelah \n, panggil sendESPNow(line) untuk meneruskan data ke ESP-Receiver. line dikosongkan untuk chunk berikutnya.
```cpp
void ESPBridge::sendESPNow(const String &msg) {
    esp_err_t result = esp_now_send(receiver_mac, (uint8_t *)msg.c_str(), msg.length());
    if (result == ESP_OK) {
        Serial.println("Chunk terkirim ke Receiver!");
    } else {
        Serial.println("Gagal kirim chunk");
    }
}
```
Fungsi ini mengirim data ke ESP-Receiver menggunakan ESP-NOW dan menampilkan status pengiriman di serial monitor, untuk memudahkan debugging.
```cpp
void setup() {
    bridge.begin();
}

void loop() {
    bridge.receiveSerial();
}
```
- `setup()` → memanggil `bridge.begin()` untuk menyiapkan ESP-Bridge.
- `loop()` → terus-menerus memanggil `bridge.receiveSerial()` untuk mengecek dan meneruskan data dari laptop.

# Bagian Esp Receiver
**EspReceiver.h**
Alur Kerja
1. Menyediakan struktur data (Chunk) untuk menyimpan potongan data.
2. Mendefinisikan kelas ESPReceiver dengan fungsi untuk:
3. Inisialisasi ESP32 dan ESP-NOW (begin)
4. Menangani setiap paket yang diterima (onDataReceived)
5. Menyusun dan menampilkan data akhir (printJSON)
6. Header ini hanya mendeklarasikan semua fungsi dan struktur.

```cpp
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <vector>
```
- `Arduino.h` → untuk fungsi dasar Arduino (Serial, String, dll).
- `esp_now.h` → untuk komunikasi ESP-NOW.
- `WiFi.h` → untuk mengatur mode WiFi ESP32.
- `vector` → untuk menampung chunk data yang diterima.
```cpp
struct Chunk {
    int number;
    String data;
};
```
- Chunk digunakan untuk menyimpan satu potongan data dari ESP-Bridge.
- number → nomor urut chunk, berguna jika pengurutan perlu dipastikan.
- data → isi chunk 
```cpp
class ESPReceiver {
private:
    std::vector<Chunk> chunks;
    void parseMessage(const String &msg);

public:
    void begin();                      
    void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len);
    void printJSON();                  
};
```
- Private:
  - `chunks` → menyimpan semua chunk yang diterima sampai data lengkap siap ditampilkan.
  - `parseMessage()` → fungsi internal untuk memproses pesan dari ESP-Bridge menjadi Chunk.
- Public:
  - `begin()` → inisialisasi ESP-NOW, WiFi, dan siap menerima data.
  - `onDataReceived()` → callback untuk menerima data ESP-NOW, memanggil `parseMessage()`.
  - `printJSON()` → menyusun seluruh chunk menjadi satu output JSON lengkap, lalu menampilkannya di serial monitor.

**main.cpp**
Alur Kerja
1. ESP-Receiver diinisialisasi (WiFi + ESP-NOW).
2. Menerima paket dari ESP-Bridge secara asinkron.
3. Setiap paket di-parse dan disimpan sebagai Chunk.
4. Jika paket terakhir diterima (|END), semua chunk digabung dan ditampilkan sebagai JSON di serial monitor.
5. Vector chunks dikosongkan untuk batch berikutnya.

```cpp
#include "ESPReceiver.h"
ESPReceiver receiver;
```
- `#include "ESPReceiver.h"` → memanggil deklarasi kelas ESPReceiver dan Chunk.
- `ESPReceiver receiver` membuat instance ESPReceiver yang akan mengelola penerimaan data dan penyusunan JSON.
```cpp
void onData(const uint8_t *mac_addr, const uint8_t *data, int len) {
    receiver.onDataReceived(mac_addr, data, len);
}
```
Fungsi ini digunakan oleh ESP-NOW untuk memproses setiap paket yang diterima dan memanggil metode onDataReceived dari instance receiver untuk parsing dan penyimpanan chunk.
```cpp
void ESPReceiver::begin() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW gagal diinisialisasi");
        return;
    }

    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *data, int len) {
        receiver.onDataReceived(mac, data, len);
    });

    Serial.println("ESP-Receiver siap menerima data");
}
```
- `Serial.begin(115200)` → inisialisasi serial monitor untuk debugging.
- `WiFi.mode(WIFI_STA)` → ESP dijadikan station (client) agar bisa menggunakan ESP-NOW.
- `esp_now_init()` → menginisialisasi ESP-NOW, jika gagal akan keluar.
- `esp_now_register_recv_cb()` → mendaftarkan callback untuk setiap paket yang masuk. Callback memanggil onDataReceived.
```cpp
void ESPReceiver::parseMessage(const String &msg) {
    int sep = msg.indexOf('|');
    if (sep != -1) {
        Chunk c;
        c.number = msg.substring(0, sep).toInt();
        c.data = msg.substring(sep + 1);
        chunks.push_back(c);
        Serial.println("Chunk terkirim ke Receiver!");
    }
}
```
- Memisahkan nomor urut dan isi chunk dari pesan yang diterima.
- Menyimpan chunk ke chunks untuk nanti disusun kembali.
- Memberikan info di serial monitor setiap kali chunk diterima.
```cpp
void ESPReceiver::onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
    String msg = "";
    for (int i = 0; i < len; i++) msg += (char)data[i];

    parseMessage(msg);

    if (msg.endsWith("|END")) {
        printJSON();
        chunks.clear();
    }
}
```
- Mengubah data mentah menjadi String.
- Memanggil parseMessage untuk menambahkan ke vector chunk.
- Jika pesan menandakan akhir data (|END), maka:
- Memanggil printJSON() untuk menyusun dan menampilkan JSON lengkap.
- Membersihkan vector chunks untuk data berikutnya.
```cpp
void ESPReceiver::printJSON() {
    String fullData = "";
    for (auto &c : chunks) fullData += c.data;

    Serial.println("[KONTEN FILE YANG DITERIMA]");

    int n1 = fullData.indexOf("\"nama\":");
    int n2 = fullData.indexOf(",", n1);
    int j1 = fullData.indexOf("\"jurusan\":");
    int j2 = fullData.indexOf(",", j1);
    int u1 = fullData.indexOf("\"umur\":");
    int u2 = fullData.indexOf(",", u1);
    int d1 = fullData.indexOf("\"deskripsi\":");
    int d2 = fullData.indexOf("}", d1);

    Serial.print("NAMA: "); Serial.println(fullData.substring(n1 + 8, n2 - 1));
    Serial.print("JURUSAN: "); Serial.println(fullData.substring(j1 + 11, j2 - 1));
    Serial.print("UMUR: "); Serial.println(fullData.substring(u1 + 7, u2));
    Serial.print("DESKRIPSI DIRI: "); Serial.println(fullData.substring(d1 + 14, d2 - 1));
}
```
- Menggabungkan semua chunk menjadi satu string fullData.
- Mencari posisi masing-masing field JSON (nama, jurusan, umur, deskripsi).
- Menampilkan hasil akhir di serial monitor sesuai format ketentuan:
```cpp
[KONTEN FILE YANG DITERIMA]
NAMA: ...
JURUSAN: ...
UMUR: ...
DESKRIPSI DIRI: ...
```
```cpp
void setup() {
    receiver.begin();
}

void loop() {
}
```
- `setup()` → memanggil receiver.begin() untuk memulai serial, WiFi, dan ESP-NOW.
- `loop()` → kosong, karena seluruh proses penerimaan berjalan asinkron via callback.

