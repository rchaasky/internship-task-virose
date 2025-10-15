#include "main.h"  

// TODO: Ganti dengan mac index mu  
const int mac_index_ku = 4;  
// ESP akan menggunakan alamat MAC ke-4 dari daftar di main.h yaitu "Africha Sekar Wangi"

void setup() {
    Serial.begin(115200);  
    Serial.println("Menyalakan ESP-NOW");  
    mulai_esp_now(mac_index_ku);  

    // TODO 1: Cetak nama mu gunakan fungsi mac_index_to_names
    Serial.println(mac_index_to_names(mac_index_ku));  
    Serial.println("Menunggu perintah...");  
}


void loop() {
    if (Serial.available()) {  
        baca_serial(callback_data_serial);  
    }
}
