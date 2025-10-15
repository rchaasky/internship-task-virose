#include "main.h"

esp_now_peer_info_t peer_info;

uint8_t mac_addresses[MAC_ADDRESS_TOTAL][MAC_ADDRESS_LENGTH] = {
    {0x24, 0x0A, 0xC4, 0x0A, 0x21, 0x11},  // BISMA
    {0x24, 0x0A, 0xC4, 0x0A, 0x21, 0x10},  // JSON
    {0x24, 0x0A, 0xC4, 0x0A, 0x20, 0x11},  // FARUG
    {0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x10},  // Fauzan Firdaus
    {0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x11},  // Africha Sekar wangi
    {0x24, 0x0A, 0xC4, 0x0A, 0x11, 0x10},  // Rafaina Erin Sadia
    {0x24, 0x0A, 0xC4, 0x0A, 0x11, 0x11},  // Antonius Michael Yordanis Hartono
    {0x24, 0x0A, 0xC4, 0x0A, 0x12, 0x10},  // Dinda Sofi Azzahro
    {0x24, 0x0A, 0xC4, 0x0A, 0x12, 0x11},  // Muhammad Fahmi Ilmi
    {0x24, 0x0A, 0xC4, 0x0A, 0x13, 0x10},  // Dhanishara Zaschya Putri Syamsudin
    {0x24, 0x0A, 0xC4, 0x0A, 0x13, 0x11},  // Irsa Fairuza
    {0x24, 0x0A, 0xC4, 0x0A, 0x14, 0x10},  // Revalinda Bunga Nayla Laksono

};

const char *mac_names[MAC_ADDRESS_TOTAL] = {
    "BISMA",                               // 0
    "JSON",                                // 1
    "FARUG",                               // 2
    "Fauzan Firdaus",                      // 3
    "Africha Sekar Wangi",                 // 4
    "Rafaina Erin Sadia",                  // 5
    "Antonius Michael Yordanis Hartono",   // 6
    "Dinda Sofi Azzahro",                  // 7
    "Muhammad Fahmi Ilmi",                 // 8
    "Dhanishara Zaschya Putri Syamsudin",  // 9
    "Irsa Fairuza",                        // 10
    "Revalinda Bunga Nayla Laksono",       // 11
};

esp_err_t mulai_esp_now(int index_mac_address) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    /* Init ESP-NOW */
    esp_err_t result = esp_now_init();
    if (result != ESP_OK)
        return result;

    /* Set callback function to handle received data */
    result = esp_now_register_recv_cb(callback_data_esp_now);
    if (result != ESP_OK)
        return result;

    result = esp_now_register_send_cb(callback_pengiriman_esp_now);
    //     if (result != ESP_OK)
    //         return result;

    /* Set MAC Address */
    uint8_t mac[MAC_ADDRESS_LENGTH];
    memcpy(mac, mac_addresses[index_mac_address], MAC_ADDRESS_LENGTH);
    result = esp_wifi_set_mac(WIFI_IF_STA, mac);
    if (result != ESP_OK)
        return result;

    /* Initialize peer_info and set fields*/
    memset(&peer_info, 0, sizeof(esp_now_peer_info_t));
    peer_info.channel = 0;
    peer_info.encrypt = false;

    /* Add All MAC to peer list  */
    for (int i = 0; i < MAC_ADDRESS_TOTAL; i++) {
        memcpy(peer_info.peer_addr, mac_addresses[i], MAC_ADDRESS_LENGTH);
        result = esp_now_add_peer(&peer_info);
        if (result != ESP_OK)
            return result;
    }

    return ESP_OK;
}

int cari_mac_index(const uint8_t *mac) {
    for (int i = 0; i < MAC_ADDRESS_TOTAL; i++) {
        // Compare the MAC address
        if (memcmp(mac, mac_addresses[i], MAC_ADDRESS_LENGTH) == 0)
            return i;
    }

    // if not found return -1
    return -1;
}

String mac_index_to_names(int mac_index) {
    if ((mac_index < 0 || mac_index >= MAC_ADDRESS_TOTAL) || (mac_index == -1)) {
        return "Unknown";
    }
    return mac_names[mac_index];
}

void callback_data_esp_now(const uint8_t *mac, const uint8_t *data, int len) {
    int index_mac_asal = cari_mac_index(mac);
    process_perintah(data, len, index_mac_asal);
}
void callback_pengiriman_esp_now(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.printf("\nStatus pengiriman ESP-NOW: %s\n", esp_err_to_name(status));
}
void callback_data_serial(const uint8_t *data, int len) {
    process_perintah(data, len);
}

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