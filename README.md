# Projek_UAS_Microprocessor

Sistem Control dan Monitoring LED Menggunakan MQTT  

Nama : Yeremia Adrianto S  
NIM : 23552011227  
Mata Kuliah : Microprocessor  
Nama Program : Sistem Control dan Monitoring LED dengan memakai MQTT  

---

## Deskripsi Program

Program ini merupakan sistem kontrol dan monitoring LED berbasis protokol MQTT (Message Queuing Telemetry Transport). Sistem memungkinkan pengguna untuk mengontrol LED secara remote melalui broker MQTT serta memonitor status LED secara real-time.

Perangkat mikrokontroler akan:

- Terhubung ke jaringan WiFi  
- Menghubungkan diri ke MQTT Broker  
- Subscribe ke topik tertentu untuk menerima perintah  
- Publish status LED ke topik monitoring  

---

## Konsep Sistem

Sistem bekerja menggunakan arsitektur komunikasi berbasis publish-subscribe dengan MQTT.

Komponen utama sistem:

- Mikrokontroler (ESP8266/ESP32)  
- MQTT Broker  
- Client (HP/Laptop via MQTT Dashboard/App)  

---

## Alur Sistem

1. Mikrokontroler melakukan koneksi ke WiFi.  
2. Setelah terhubung, sistem menghubungkan diri ke MQTT Broker.  
3. Mikrokontroler melakukan subscribe ke topik kontrol LED.  
4. User mengirimkan perintah ON/OFF melalui MQTT client.  
5. Mikrokontroler menerima pesan dan memprosesnya.  
6. Status LED dikirim kembali ke broker melalui topik monitoring.  

---

## Manajemen Proses

Manajemen proses pada sistem ini meliputi:

### 1. Proses Inisialisasi
- Inisialisasi pin LED sebagai OUTPUT.  
- Inisialisasi koneksi WiFi.  
- Setup koneksi MQTT dan callback function.  

### 2. Proses Koneksi
- Jika WiFi terputus → sistem mencoba reconnect.  
- Jika MQTT terputus → sistem melakukan reconnect ke broker.  

### 3. Proses Monitoring Loop
Pada fungsi `loop()`:
- Mengecek koneksi MQTT.  
- Menjalankan `client.loop()` untuk membaca pesan.  
- Mengeksekusi perintah berdasarkan pesan yang diterima.  

---

## Manajemen Memori

Manajemen memori pada sistem ini mencakup:

- Penggunaan variabel global untuk koneksi WiFi dan MQTT.  
- Buffer pesan MQTT menggunakan tipe data `char`.  
- Pengolahan string menggunakan konversi seperlunya untuk menghindari pemborosan memori.  
- Tidak menggunakan alokasi dinamis (`malloc/free`), sehingga memori lebih stabil.  

Karena berjalan pada mikrokontroler dengan RAM terbatas, penggunaan variabel dibuat sesederhana mungkin.

---

## Manajemen Komunikasi

Komunikasi menggunakan protokol MQTT dengan model publish-subscribe.

### 1. Publish
Digunakan untuk mengirim status LED ke broker.

Contoh:

```
topic: led/status
message: ON / OFF
```

### 2. Subscribe
Digunakan untuk menerima perintah kontrol.

Contoh:

```
topic: led/control
message: ON / OFF
```

### 3. Callback Function

```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message == "ON") {
    digitalWrite(LED_PIN, HIGH);
  } else if (message == "OFF") {
    digitalWrite(LED_PIN, LOW);
  }
}
```

---

## Diagram Blok Sistem

```
+-------------+        WiFi        +---------------+
|             |-------------------->|               |
| ESP8266/32  |                     | MQTT Broker  |
|             |<--------------------|               |
+-------------+     MQTT Pub/Sub   +---------------+
        |
        |
        v
      LED
```

---

## Potongan Kode Utama

### Inisialisasi WiFi

```cpp
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
```

### Koneksi MQTT

```cpp
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESPClient")) {
      client.subscribe("led/control");
    } else {
      delay(5000);
    }
  }
}
```

### Loop Utama

```cpp
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
```

---
