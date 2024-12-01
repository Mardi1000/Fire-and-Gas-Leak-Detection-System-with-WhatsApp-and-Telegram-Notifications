#include <WiFi.h>
#include <HTTPClient.h>

// Konfigurasi WiFi
const char* ssid = "Denays"; // Ganti dengan nama WiFi Anda
const char* password = "tiganya4limanya2"; // Ganti dengan password WiFi Anda

// Konfigurasi CallMeBot API untuk WhatsApp
String phoneNumber = "6285718181317"; // Nomor WhatsApp dalam format internasional
String apiKey = "6842093"; // API key CallMeBot

// Pin Konfigurasi
#define Buzzer 32       // Pin untuk buzzer
#define MQ2 34          // Pin untuk sensor gas MQ2
#define Sensor_Api 33   // Pin untuk sensor api
#define RED_LED 26      // Pin untuk LED merah
#define GREEN_LED 27    // Pin untuk LED hijau

// Konstanta kalibrasi sensor gas MQ2
#define RL 1            // Nilai RL dalam kilo ohm
#define m -0.44721      // Konstanta kalibrasi m
#define b 1.23257       // Konstanta kalibrasi b
#define Ro 2.7          // Nilai Ro hasil kalibrasi

// Thresholds dan debounce
const int gasThreshold = 4000; // Threshold deteksi gas
const int debounceDelay = 3000; // Delay antar notifikasi
unsigned long lastAlertTime = 0;

String urlencode(String str) {
    String encodedString = "";
    char c;
    char code0, code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encodedString += '+';
        } else if (isalnum(c)) {
            encodedString += c;
        } else {
            code1 = (c & 0xF) + '0';
            if ((c & 0xF) > 9) code1 = (c & 0xF) - 10 + 'A';
            c = (c >> 4) & 0xF;
            code0 = c + '0';
            if (c > 9) code0 = c - 10 + 'A';
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
    }
    return encodedString;
}

void sendMessage(String message) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + 
                     "&text=" + urlencode(message) + "&apikey=" + apiKey;
        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode == 200) {
            Serial.println("Notifikasi WhatsApp Berhasil Terkirim");
        } else {
            Serial.print("Gagal Mengirim Pesan. Kode: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi Tidak Terhubung");
    }
}

void setup() {
    Serial.begin(115200);

    // Inisialisasi pin
    pinMode(Sensor_Api, INPUT);
    pinMode(MQ2, INPUT);
    pinMode(Buzzer, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    // Hubungkan ke WiFi
    Serial.print("Menghubungkan ke WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nTerhubung ke WiFi!");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    int sensor_api = digitalRead(Sensor_Api);  // Membaca sensor api
    int gasValue = analogRead(MQ2);           // Membaca nilai sensor gas MQ2

    // Menghitung konsentrasi gas dalam PPM
    float VRL = gasValue * (5.0 / 4095.0);   // Tegangan sensor (asumsi ADC ESP32)
    float sensorRs = ((5 * RL / VRL) - RL);
    float ratio = sensorRs / Ro;
    float ppm = pow(10, ((log10(ratio) - b) / m));

    Serial.print("Nilai Sensor Api: ");
    Serial.println(sensor_api);
    Serial.print("Konsentrasi Gas (ppm): ");
    Serial.println(ppm, 2);

    // Deteksi Api
    if (sensor_api == LOW) { // LOW berarti api terdeteksi
        Serial.println("Api Terdeteksi!");
        digitalWrite(Buzzer, HIGH);
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        sendMessage("Peringatan! Ada api terdeteksi!");
    } 
    // Deteksi Gas
    else if (ppm > gasThreshold && millis() - lastAlertTime > debounceDelay) {
        Serial.println("Gas Terdeteksi!");
        digitalWrite(Buzzer, HIGH);
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        sendMessage("Peringatan! Kebocoran gas terdeteksi!");
        lastAlertTime = millis();
    } 
    // Tidak ada bahaya
    else {
        Serial.println("Tidak Ada Api atau Gas");
        digitalWrite(Buzzer, LOW);
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
    }

    delay(1000); // Jeda untuk menghindari pembacaan berlebihan
}
