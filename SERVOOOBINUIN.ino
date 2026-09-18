#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <WiFiManager.h>

// =====================================================
// KONFIGURASI API LARAVEL
// =====================================================
String apiUrl = "http://bin.ihi.my.id/api/servo/status";

// =====================================================
// PIN SERVO & VARIABEL
// =====================================================
#define SERVO1_PIN 26   // D26
#define SERVO2_PIN 25   // D27

Servo servo1;
Servo servo2;

int currentAngle1 = -1;
int currentAngle2 = -1;

unsigned long lastPollTime = 0;
const unsigned long pollInterval = 1500;

// =====================================================
// FUNGSI KONEKSI WIFI & WIFIMANAGER
// =====================================================
void setup_wifi() {
    Serial.println();
    Serial.println("Memulai WiFiManager...");

    // Cek apakah belum ada WiFi yang tersimpan sebelumnya di memori
    if (WiFi.SSID() == "") {
        Serial.println("Tidak ada riwayat WiFi. Mencoba koneksi bawaan (Default): Yoimo");
        WiFi.begin("Yoimo", "12344321");
        
        int retries = 0;
        // Tunggu maksimal sekitar 7.5 detik
        while (WiFi.status() != WL_CONNECTED && retries < 15) {
            delay(500);
            Serial.print(".");
            retries++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nBerhasil terhubung ke WiFi Bawaan (Yoimo)!");
        } else {
            Serial.println("\nGagal terhubung ke WiFi Bawaan.");
        }
    }

    // Jalankan WiFiManager
    // Ini akan otomatis mencoba menyambung ke WiFi terakhir (entah itu Yoimo, atau yang baru)
    // Jika semua gagal, baru dia akan memancarkan sinyal WiFi bernama "ESPSERVO"
    WiFiManager wm;
    bool res = wm.autoConnect("ESPSERVO");

    if (!res) {
        Serial.println("Gagal terhubung dan kehabisan waktu, restart...");
        delay(3000);
        ESP.restart();
    } 
    else {
        Serial.println("Berhasil Terhubung ke WiFi!");
        Serial.print("SSID Saat Ini: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    // Setup Servo
    servo1.setPeriodHertz(50);
    servo1.attach(SERVO1_PIN, 500, 2400);
    servo1.write(0);

    servo2.setPeriodHertz(50);
    servo2.attach(SERVO2_PIN, 500, 2400);
    servo2.write(0);

    // Setup WiFi
    setup_wifi();
}

// =====================================================
// LOOP (POLLING)
// =====================================================
void loop() {
    if (millis() - lastPollTime >= pollInterval) {
        lastPollTime = millis();

        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            
            http.begin(apiUrl);
            int httpResponseCode = http.GET();
            
            if (httpResponseCode > 0) {
                String payload = http.getString();
                
                // Parsing JSON (kapasitas diperbesar sedikit untuk menampung data WiFi baru)
                StaticJsonDocument<300> doc;
                DeserializationError error = deserializeJson(doc, payload);
                
                if (!error) {
                    
                    // 1. CEK PERINTAH GANTI WIFI (HYBRID MODE)
                    if (doc.containsKey("new_wifi")) {
                        const char* new_ssid = doc["new_wifi"]["ssid"];
                        const char* new_pass = doc["new_wifi"]["password"];
                        
                        Serial.println("\n=================================");
                        Serial.print("MENERIMA PERINTAH GANTI WIFI: ");
                        Serial.println(new_ssid);
                        Serial.println("Menyimpan pengaturan & Restart...");
                        Serial.println("=================================\n");
                        
                        // Hapus memori WiFiManager & WiFi sistem
                        WiFi.disconnect(true, true);
                        delay(1000);
                        
                        // Simpan WiFi baru ke memori permanen ESP32
                        WiFi.begin(new_ssid, new_pass);
                        delay(2000); // Beri waktu untuk menyimpan
                        
                        // Restart ESP32 agar WiFiManager mengeksekusi koneksi baru
                        ESP.restart();
                    }

                    // 2. KONTROL SERVO
                    int targetAngle1 = doc["servo1"];
                    int targetAngle2 = doc["servo2"];
                    
                    if (targetAngle1 != currentAngle1) {
                        currentAngle1 = targetAngle1;
                        int reverseAngle1 = 180 - currentAngle1; 
                        servo1.write(reverseAngle1);
                        Serial.print("Servo 1 -> ");
                        Serial.println(reverseAngle1);
                    }

                    if (targetAngle2 != currentAngle2) {
                        currentAngle2 = targetAngle2;
                        servo2.write(currentAngle2);
                        Serial.print("Servo 2 -> ");
                        Serial.println(currentAngle2);
                    }
                } else {
                    Serial.print("Gagal parsing JSON: ");
                    Serial.println(error.c_str());
                }
            } else {
                Serial.print("Error HTTP: ");
                Serial.println(httpResponseCode);
            }
            
            http.end(); 
        } else {
            Serial.println("WiFi Terputus, mencoba menghubungkan ulang...");
            WiFi.reconnect();
        }
    }
}
