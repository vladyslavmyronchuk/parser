#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_HMC5883_U.h>

// Сенсори
TinyGPSPlus gps;
Adafruit_BMP280 bmp;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// Піни
const int trigPin = 9;
const int echoPin = 10;
//led піни
const int redpin = 13;
const int greenpin = 12;

// Таймер
unsigned long lastRead = 0;
const unsigned long interval = 1000;

// Змінні
float distance = -1;
float altitude = 0;
float lat = 0, lng = 0;
uint8_t sats = 0;
uint16_t errorFlags = 0;

float measureDistance();

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  if (!bmp.begin(0x77))
    errorFlags |= (1 << 1); // NO_BMP
  if (!mag.begin())
    errorFlags |= (1 << 3); // NO_COMPASS
}

void loop()
{
  // Читання GPS постійно
  while (Serial1.available())
  {
    gps.encode(Serial1.read());
  }

  if (millis() - lastRead >= interval)
  {
    lastRead = millis();
    errorFlags = 0; // скидаємо перед новим циклом

    // Ультразвук
    distance = measureDistance();
    if (distance < 0)
      errorFlags |= (1 << 0); // NO_ULTRASOUND

    // BMP280
    if (bmp.begin(0x77))
    {
      altitude = bmp.readAltitude(1013.25);
    }
    else
    {
      errorFlags |= (1 << 1); // NO_BMP
    }

    // GPS
    if (gps.location.isValid())
    {
      lat = gps.location.lat();
      lng = gps.location.lng();
      sats = gps.satellites.value();
    }
    else
    {
      errorFlags |= (1 << 2); // NO_GPS
      lat = 0;
      lng = 0;
      sats = 0;
    }
    //показ помилки на лед
    if (errorFlags  == 0){
      digitalWrite(greenpin,LOW);
      digitalWrite(redpin,HIGH);
    }
    else{
      digitalWrite(redpin,LOW);
      digitalWrite(greenpin,LOW);
    }
    // Вивід JSON
    Serial.print(F("{\"distance\":"));
    Serial.print(distance);
    Serial.print(F(",\"altitude\":"));
    Serial.print(altitude);
    Serial.print(F(",\"lat\":"));
    Serial.print(lat, 6);
    Serial.print(F(",\"lng\":"));
    Serial.print(lng, 6);
    Serial.print(F(",\"sats\":"));
    Serial.print(sats);
    Serial.print(F(",\"errorFlags\":"));
    Serial.print(errorFlags);
    Serial.println(F("}"));
  }
}

// Ультразвук
float measureDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000); // 25 мс
  if (duration == 0)
    return -1;
  return duration * 0.034 / 2.0;
}