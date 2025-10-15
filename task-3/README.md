# Sapa Menyawa ESP-NOW

## Deskripsi Umum
Proyek ini merupakan task-3 dari intern programming virose yang merupakan implementasi proyek untuk membangun sistem komunikasi nirkabel antar perangkat ESP32 menggunakan protokol ESP-NOW. Dalam proyek ini, setiap ESP32 memiliki identitas unik yang ditentukan berdasarkan alamat MAC. Identitas ini digunakan untuk:
1. Mengirim pesan sapaan antar-perangkat (contoh: perintah HALO).
2. Memeriksa status perangkat lain (contoh: perintah CEK).
3. Menangani tanggapan atau jawaban dari perangkat tujuan (contoh: perintah JAWAB).

---

## Struktur src pada file
- main.h      → Deklarasi fungsi, array MAC, enum, konstanta, dan array MAC + nama perangkat.
- main.cpp     → Setup ESP32, loop utama, TODO 1: Cetak nama pengguna berdasarkan MAC
- utility.cpp  → Logika baca serial & proses perintah, TODO 2 & 3 baca paket dari serial, cek header & panjang data, proses perintah `HALO, CEK, JAWAB`, kirim/terima pesan via ESP-NOW.

# main.h
File ini adalah header utama, berisi:
- Konstanta: MAC_ADDRESS_TOTAL, MAC_ADDRESS_LENGTH, BUFFER_SIZE
- Array MAC: mac_addresses (alamat ESP), mac_names (nama ESP)
- Enum: ADDRESS_ASSIGNMENT (index MAC), COMMAND (HALO, CEK, JAWAB)
- Deklarasi Fungsi: inisialisasi ESP-NOW, baca serial, proses perintah, callback

# main.cpp
File ini berfungsi sebagai entry point program. Tugas utamanya:
- Menentukan identitas ESP 
- Menginisialisasi ESP-NOW melalui fungsi `mulai_esp_now()`
- Memanggil fungsi untuk menunggu perintah dari Serial
- Melakukan loop utama untuk terus memeriksa input Serial

**KODE**
```cpp
#include "main.h"

// TODO: Ganti dengan MAC index perangkat pengguna
const int mac_index_ku = 4; 
void setup() {
    Serial.begin(115200);  
    Serial.println("Menyalakan ESP-NOW");
    mulai_esp_now(mac_index_ku);  

    // TODO 1: Cetak nama pengguna sesuai MAC index
    Serial.println(mac_index_to_names(mac_index_ku));  
    Serial.println("Menunggu perintah...");
}

void loop() {
    if (Serial.available()) {  
        baca_serial(callback_data_serial);  
    }
}
```
### TODO 1 - Cetak Nama Pengguna
1. Di dalam file main.h, terdapat array mac_addresses yang memuat daftar alamat MAC untuk setiap perangkat. Setiap alamat MAC dikaitkan dengan nama pemilik atau identitas perangkat melalui array mac_names.
2. Pilih alamat MAC yang digunakan untuk perangkat ini yaitu alamat MAC ke-4 dengan nilai
   `{0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x11}`yang sesuai dengan nama saya yaitu "Africha Sekar Wangi".
3. Panggil fungsi `mulai_esp_now(mac_index_ku)` untuk menginisialisasi ESP-NOW dengan alamat MAC yang telah dipilih.
4. Gunakan fungsi mac_index_to_names(int mac_index) untuk mengonversi indeks MAC menjadi nama perangkat
5. Tampilkan nama perangkat di serial monitor.
   
```cpp
Serial.println(mac_index_to_names(mac_index_ku));
```
Output yang diharapkan berupa
`Africha Sekar Wangi`

## utility.cpp
- Di dalam file ini mengandung fungsi pendukung utama untuk:
- Membaca data dari Serial
- Memproses perintah yang diterima baik dari Serial maupun ESP-NOW
- Mengatur callback penerimaan dan pengiriman data ESP-NOW

### TODO 2 - Baca Serial
1. Gunakan `static uint8_t buffer[BUFFER_SIZE]` untuk menyimpan data yang masuk.
2. Gunakan `static int index` untuk menandai posisi byte berikutnya dalam buffer.
3. Cek header paket: 3 byte pertama harus `0xFF, 0xFF, 0x00.`
4. Byte ke-4 menandai panjang payload data.
5. Setelah seluruh paket diterima, panggil callback dengan payload data.
6. Reset buffer jika header salah atau buffer penuh (overflow).

**KODE**
```cpp
void baca_serial(void (*callback)(const uint8_t *data, int len)) {
    // TODO 2: implementasi kode buat nerima perintah dari serial
    static uint8_t buffer[BUFFER_SIZE];   //menyimpan data yang masuk dari Serial
    static int index = 0;   //Index saat ini dalam buffer (berapa byte yang sudah diterima)

    while (Serial.available()) {
        uint8_t byte_in = Serial.read();  
        buffer[index++] = byte_in; 

        if (index >= 4) {
            // Cek kalau sudah ada minimal 4 byte (header 3 byte + panjang data 1 byte)
            if (buffer[0] == 0xFF && buffer[1] == 0xFF && buffer[2] == 0x00) {
                // Cek apakah 3 byte pertama adalah HEADER yang benar
                uint8_t panjang_data = buffer[3]; 
                int total_paket = 4 + panjang_data; // Total paket = 3 byte header + 1 byte panjang data + byte data 

                if (index >= total_paket) {
                    callback(&buffer[4], panjang_data);
                    index = 0;
                }
            } else {
                index = 0;
            }
        }

        if (index >= BUFFER_SIZE) index = 0;
    }
}
```
### TODO 3 - Proses Perintah
1. `data[0]` → perintah: HALO, CEK, JAWAB
2. `data[1]` → index MAC tujuan (jika tersedia)
3. Tentukan `nama_asal`:
   - Dari ESP → gunakan `mac_index_to_names(index_mac_address_asal)`
   - Dari Serial → tampilkan "Laptop"
4. Tentukan `nama_tujuan`:
   - Jika index valid → gunakan `mac_names[index_tujuan]`
   - Jika tidak valid → "Unknown"
5. Gunakan `strcpy((char *)&kirim[1], pesan.c_str())` untuk menyalin string ke buffer ESP-NOW.
6. Kirim paket menggunakan `esp_now_send()`, panjang paket = `1 + pesan.length()` (+1 untuk byte perintah di index 0)
7. Tampilkan pesan di Serial jika perintah adalah JAWAB.

**KODE**
```cpp
void process_perintah(const uint8_t *data, int len, int index_mac_address_asal) {
    // TODO 3: implementasi kode buat processing perintah
    uint8_t command = data[0];  // byte pertama = jenis perintah
    int index_tujuan = (len > 1) ? data[1] : -1; // byte kedua = tujuan

    String nama_asal = (index_mac_address_asal == -1) ? "Laptop" : mac_index_to_names(index_mac_address_asal);
    String nama_tujuan = (index_tujuan >= 0 && index_tujuan < MAC_ADDRESS_TOTAL) ? mac_names[index_tujuan] : "Unknown";

    switch (command) {
        case HALO: {
            if (index_mac_address_asal == -1) {
                // Dari serial (Laptop)
                String pesan = "Halo " + nama_tujuan + " Aku " + String(mac_names[mac_index_ku]);
                uint8_t kirim[BUFFER_SIZE];
                kirim[0] = HALO;
                strcpy((char *)&kirim[1], pesan.c_str());
                esp_now_send(mac_addresses[index_tujuan], kirim, 1 + pesan.length());
            } else {
                // Dari ESP-NOW
                String pesan = "Halo Juga " + nama_asal + " Aku " + String(mac_names[mac_index_ku]);
                uint8_t kirim[BUFFER_SIZE];
                kirim[0] = JAWAB;
                strcpy((char *)&kirim[1], pesan.c_str());
                esp_now_send(mac_addresses[index_mac_address_asal], kirim, 1 + pesan.length());
            }
            break;
        }

        case CEK: {
            if (index_mac_address_asal == -1) {
                // Dari Serial
                String pesan = nama_tujuan + String(" ini ") + String(mac_names[mac_index_ku]) + " apa kamu disana?";
                uint8_t kirim[BUFFER_SIZE];
                kirim[0] = CEK;
                strcpy((char *)&kirim[1], pesan.c_str());
                esp_now_send(mac_addresses[index_tujuan], kirim, 1 + pesan.length());
            } else {
                // Dari ESP-NOW
                String pesan = "Iya Aku " + nama_asal + " Disini - " + String(mac_names[mac_index_ku]);
                uint8_t kirim[BUFFER_SIZE];
                kirim[0] = JAWAB;
                strcpy((char *)&kirim[1], pesan.c_str());
                esp_now_send(mac_addresses[index_mac_address_asal], kirim, 1 + pesan.length());
            }
            break;
        }

        case JAWAB: {
            // Hanya dari ESP-NOW, tampilkan di Serial
            String pesan = String((char *)&data[1]);
            Serial.println("Pesan diterima: " + pesan);
            break;
        }

        default:
            Serial.println("Perintah tidak dikenali!");
            break;
    }
}
```

