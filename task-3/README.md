Program ini dibuat untuk mentransfer file JSON dari laptop ke ESP-Receiver melalui ESP-Bridge menggunakan ESP-NOW. Program harus memecah file JSON menjadi beberapa chunk agar sesuai batasan ESP-NOW 250 byte per packet, kemudian menyusun kembali di ESP-Receiver dan menampilkan data ke serial monitor.

*Struktur Proyek*
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

*Flow Program*
Laptop:
- Membaca JSON dari data.json menggunakan Laptop::readJSON().
- Memecah data menjadi chunk dengan Laptop::splitData(). Khusus DESKRIPSI, dipisahkan per kata - agar tidak lebih dari 85 karakter per chunk, menjaga kata tidak terpotong.
- Mengirim setiap chunk ke ESP-Bridge via serial (Laptop::sendToESPBridge()).
ESP-Bridge:
- Menerima serial dari Laptop (ESPBridge::receiveSerial()).
- Meneruskan setiap chunk ke ESP-Receiver melalui ESP-NOW (ESPBridge::sendESPNow()).
ESP-Receiver:
- Menerima chunk melalui ESP-NOW di callback onDataReceived().
- Menyimpan semua chunk ke memory sementara (parseMessage()).
- Setelah menerima semua chunk (|END), menyusun kembali data menjadi JSON utuh dan menampilkan di Serial Monitor (printJSON()).

#Bagian Laptop#
*Laptop.h*
1. Membaca file JSON (data.json) yang berisi:
```cpp
File JSON
{
    "nama": "Africha Sekar Wangi",
    "jurusan": "Sistem Informasi",
    "umur": 18,
    "deskripsi": "Saya adalah orang yang sangat soft spoken, tidak sombong, rendah hati, dan rajin menabung. Saya juga senang menonton film dan drama korea, tapi sekarang sudah sibuk coolyeah"
}
```
2. Memecah data menjadi chunk agar sesuai batasan ESP-NOW (maks 250 byte per packet).
3. Mengirim setiap chunk ke ESP-Bridge via serial port.
4. Menjaga agar DESKRIPSI tetap terbaca rapi: tidak memotong kata, chunk seimbang (<85 karakter per chunk).
#Laptop.h#
`#pragma once` mencegah file header di-include lebih dari sekali.
```cpp
#include <string>
#include <vector>```
 <string>: digunakan untuk tipe std::string agar kita bisa menyimpan teks, nama, jurusan, deskripsi.
<vector>: digunakan untuk std::vector agar bisa menyimpan list chunk dinamis.
```cpp
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
