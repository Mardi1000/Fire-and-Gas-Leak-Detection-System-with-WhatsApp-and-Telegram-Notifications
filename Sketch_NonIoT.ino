#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define RL 1
#define m -0.44721
#define b 1.23257
#define Ro 2.5
#define MQ_SENSOR_PIN A0

int redLed = 12;
int greenLed = 11;
int buzzer = 10;
int flameSensorPin = A1;
int sensorThreshold = 400;

float sensorRs;
float VRL;
float ppm;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(MQ_SENSOR_PIN, INPUT);
  pinMode(flameSensorPin, INPUT);
}

void loop() {
  int gasValue = analogRead(MQ_SENSOR_PIN);
  VRL = gasValue * (5.0 / 1023);
  sensorRs = ((5.0 / VRL) - 1) * RL;
  ppm = (Ro / sensorRs) * 250; 

  int flameState = digitalRead(flameSensorPin);

  Serial.print("Sensor Gas (A0): ");
  Serial.println(gasValue);
  Serial.print("Konsentrasi (ppm): ");
  Serial.println(ppm);
  Serial.print("Sensor Api (A1): ");
  Serial.println(flameState == LOW ? "Api Terdeteksi" : "Tidak Ada Api");

  // Menampilkan hasil di LCD
  lcd.clear(); // Bersihkan layar sebelum menampilkan informasi baru

  // Memeriksa kondisi deteksi gas
  if (gasValue > sensorThreshold) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
    digitalWrite(buzzer, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("GAS terdeteksi!");
  } else if (flameState == LOW) {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
    digitalWrite(buzzer, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("API terdeteksi!");
  } else {
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed, HIGH);
    digitalWrite(buzzer, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Tidak ada GAS  ");
  }

  // Menampilkan konsentrasi gas di baris kedua LCD
  lcd.setCursor(0, 1);
  lcd.print("PPM: ");
  lcd.print(ppm, 2); // Tampilkan 2 digit desimal

  delay(100);
}