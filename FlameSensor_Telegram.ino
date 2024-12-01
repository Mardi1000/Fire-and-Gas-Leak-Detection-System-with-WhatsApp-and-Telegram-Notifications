#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Pin Konfigurasi
#define Buzzer 32        // Pin untuk buzzer
#define MQ2 34           // Pin untuk sensor gas MQ2
#define Sensor_Api 33    // Pin untuk sensor api
#define RED_LED 26       // Pin untuk LED merah
#define GREEN_LED 27     // Pin untuk LED hijau

// Konfigurasi WiFi
const char* ssid = "Denays"; // Ganti dengan nama WiFi
const char* password = "tiganya4limanya2"; // Ganti dengan password WiFi

// Konfigurasi Telegram Bot
const String botToken = "7711789052:AAFSsa6VT8oJKDLvMQphsQimK90t2r9KzWs"; // Ganti dengan token bot Telegram
const String chatID = "1832575595"; // Ganti dengan chat ID Telegram

// Thresholds and debounce times
const int gasThreshold = 2000; // Set threshold higher
const int debounceDelay = 3000; // Delay between alerts

// Konstanta kalibrasi
#define RL 1             // Nilai RL dalam kilo ohm
#define m -0.44721       // Konstanta kalibrasi m
#define b 1.23257        // Konstanta kalibrasi b
#define Ro 2.7           // Nilai Ro hasil kalibrasi

unsigned long lastAlertTime = 0;

void setup() {
    Serial.begin(115200);
    pinMode(Sensor_Api, INPUT);  // Pin sensor api sebagai input
    pinMode(MQ2, INPUT);          // Pin sensor gas MQ2 sebagai input
    pinMode(Buzzer, OUTPUT);      // Pin buzzer sebagai output
    pinMode(RED_LED, OUTPUT);     // Pin LED merah sebagai output
    pinMode(GREEN_LED, OUTPUT);   // Pin LED hijau sebagai output

    // Hubungkan ke WiFi
    Serial.print("Menghubungkan ke WiFi ");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Terhubung ke WiFi!");
}

void sendTelegramNotification(String message) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure(); // Abaikan sertifikat SSL

        HTTPClient http;
        String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;

        http.begin(client, url);
        int httpCode = http.GET();
        
        if (httpCode > 0) {
            Serial.println("Pesan Terkirim ke Telegram");
        } else {
            Serial.println("Gagal Mengirim Pesan ke Telegram");
        }
        http.end();
    } else {
        Serial.println("WiFi Tidak Terhubung");
    }
}

void loop() {
    int sensor_api = digitalRead(Sensor_Api); // Baca kondisi sensor api
    int gasValue = analogRead(MQ2);           // Baca nilai analog dari sensor gas MQ2

    // Menghitung nilai konsentrasi gas dalam PPM
    float VRL = gasValue * (5 / 4095.0);       // Menghitung tegangan sensor, 3.3V untuk ESP32
    float sensorRs = ((5 * RL / VRL) - RL);
    float ratio = sensorRs/Ro;     // Menghitung nilai resistansi sensor Rs
    float ppm = pow(10, ((log10(ratio)-b)/m));           // Menghitung PPM berdasarkan kalibrasi

    Serial.print("Nilai Sensor Api: ");
    Serial.println(sensor_api);
    Serial.print("Konsentrasi Gas (ppm): ");
    Serial.println(ppm, 2);  // Tampilkan nilai ppm dengan dua desimal

    // Deteksi Api
    if (sensor_api == LOW) { // Api terdeteksi (LOW berarti api terdeteksi)
        Serial.println("Api Terdeteksi!");
        digitalWrite(Buzzer, HIGH);       // Aktifkan buzzer
        digitalWrite(RED_LED, HIGH);      // Nyalakan LED merah
        digitalWrite(GREEN_LED, LOW);     // Matikan LED hijau
        sendTelegramNotification("Peringatan! Api Terdeteksi!");
    }
    // Deteksi Gas dengan Threshold dan Debounce
    else if (ppm > gasThreshold && millis() - lastAlertTime > debounceDelay) { 
        Serial.println("Gas Terdeteksi!");
        digitalWrite(Buzzer, HIGH);       // Aktifkan buzzer
        digitalWrite(RED_LED, HIGH);      // Nyalakan LED merah
        digitalWrite(GREEN_LED, LOW);     // Matikan LED hijau
        sendTelegramNotification("Peringatan! Gas Terdeteksi!");
        lastAlertTime = millis();         // Update last alert time
    } 
    else {
        Serial.println("Tidak Ada Api atau Gas");
        digitalWrite(Buzzer, LOW);        // Matikan buzzer
        digitalWrite(RED_LED, LOW);       // Matikan LED merah
        digitalWrite(GREEN_LED, HIGH);    // Nyalakan LED hijau
    }

    delay(1000); // Jeda untuk menghindari pesan terlalu sering
}