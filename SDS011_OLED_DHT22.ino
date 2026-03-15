#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// -------------------- PINS --------------------
#define SDAPIN 8
#define SCKPIN 9
#define BUTTON_PIN 3
#define RX_PIN 20
#define TX_PIN 21
#define DHTPIN 7
#define DHTTYPE DHT22
// -------------------- OLED --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// -------------------- OBJECTS -----------------
DHT dht(DHTPIN, DHTTYPE);
HardwareSerial pmSerial(1);

// -------------------- SENSOR DATA -------------
float temperature = 0;
float humidity = 0;
float pm25 = 0;
float pm10 = 0;
float aqi;

// -------------------- SCREEN CONTROL ----------
volatile bool buttonPressed = false;
volatile unsigned long lastISRTime = 0;

uint8_t screenIndex = 0;
const uint8_t TOTAL_SCREENS = 6;

// -------------------- BUZZER ------------------
unsigned long lastBeep = 0;
unsigned long beepInterval = 0;
bool beepActive = false;

// -------------------- ISR ---------------------
void IRAM_ATTR handleButton()
{
  unsigned long now = millis();
  if (now - lastISRTime > 250) {   // debounce
    buttonPressed = true;
    lastISRTime = now;
  }
}

// -------------------- SETUP -------------------
void setup()
{
  Serial.begin(115200);
  dht.begin();
  
  pmSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  
  Wire.begin(SDAPIN, SCKPIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  
}

// -------------------- LOOP --------------------
void loop()
{ String data;
    data +=  String(temperature) + ",";
    data +=  String(humidity) + ",";
    data +=  String(pm25) + ",";
    data +=  String(pm10) + ",";
    data +=  String(aqi) ;
     Serial.println(data);
     
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

 readSDS011();
  aqi = finalAQI(pm25, pm10, temperature, humidity);

  if (buttonPressed) {
    screenIndex = (screenIndex + 1) % TOTAL_SCREENS;
    buttonPressed = false;
  }

  
  switch (screenIndex) {
    case 0: displayValues(); break;
    case 1: displayTemp(); break;
    case 2: displayHum(); break;
    case 3: displayPM25(); break;
    case 4: displayPM10(); break;
    case 5: displayQuality(); break;
  }

 }

float calcAQI(float C, float Clow, float Chigh, float Ilow, float Ihigh)
{
  return ((Ihigh - Ilow) / (Chigh - Clow)) * (C - Clow) + Ilow;
}
int AQI_PM25(float pm25)
{
  if (pm25 <= 15)    return calcAQI(pm25, 0, 15, 0, 50);
  if (pm25 <= 25)    return calcAQI(pm25, 15, 25, 51, 100);
  if (pm25 <= 37.5)  return calcAQI(pm25, 25, 37.5, 101, 200);
  if (pm25 <= 50)    return calcAQI(pm25, 37.5, 50, 201, 300);
  if (pm25 <= 75)    return calcAQI(pm25, 50, 75, 301, 400);
  return 500;
}
int AQI_PM10(float pm10)
{
  if (pm10 <= 45)    return calcAQI(pm10, 0, 45, 0, 50);
  if (pm10 <= 75)    return calcAQI(pm10, 45, 75, 51, 100);
  if (pm10 <= 100)   return calcAQI(pm10, 75, 100, 101, 200);
  if (pm10 <= 150)   return calcAQI(pm10, 100, 150, 201, 300);
  if (pm10 <= 250)   return calcAQI(pm10, 150, 250, 301, 400);
  return 500;
}



float humidityCorrectPM25(float pm25, float humidity)
{
  if (humidity <= 50) return pm25;
  return pm25 * (1.0 - 0.01 * (humidity - 50));
}
float humidityCorrectPM10(float pm10, float humidity)
{
  if (humidity <= 50) return pm10;
  return pm10 * (1.0 - 0.01 * (humidity - 50));
}

int finalAQI(float pm25, float pm10,
             float temperature, float humidity)
{
  // Optional PM correction
  float pm25_corr = humidityCorrectPM25(pm25, humidity);
  float pm10_corr = humidityCorrectPM10(pm10, humidity);

  int aqi_pm25 = AQI_PM25(pm25_corr);
  int aqi_pm10 = AQI_PM10(pm10_corr);
  

  // Final AQI = worst pollutant
  float aqi = max(aqi_pm25,aqi_pm10);

  return aqi;
  
}

// -------------------- SDS011 PARSER -----------
void readSDS011()
{
  static uint8_t buffer[10];
  static uint8_t index = 0;

  while (pmSerial.available()) {
    uint8_t b = pmSerial.read();

    // Sync to header
    if (index == 0 && b != 0xAA)
      continue;

    buffer[index++] = b;

    if (index == 10) {
      index = 0;

      // Validate frame
      if (buffer[0] != 0xAA || buffer[1] != 0xC0 || buffer[9] != 0xAB)
        return;

      // Verify checksum
      uint8_t checksum = 0;
      for (int i = 2; i <= 7; i++)
        checksum += buffer[i];

      if (checksum != buffer[8])
        return;

      uint16_t rawPM25 = (buffer[3] << 8) | buffer[2];
      uint16_t rawPM10 = (buffer[5] << 8) | buffer[4];

      pm25 = rawPM25 / 10.0;
      pm10 = rawPM10 / 10.0;
    }
  }
}

// -------------------- DISPLAY UTILS ----------
void drawCenter(String s, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(s);
}

// -------------------- SCREENS ----------------
void displayValues()
{
  display.clearDisplay();
  display.setTextSize(2);
  drawCenter("OAKS", 0);

  display.setTextSize(2);
  display.setCursor(20, 20);
  
  display.print("AQI:");
  display.print((int)aqi);
  

  display.setCursor(7,45);
  display.printf("PM2.5");

  display.setTextSize(1);
  display.setCursor(3, 3);
  display.printf("T:%.1f\n", temperature);
  display.drawLine(0, 12, 40, 12, WHITE);
  display.drawLine(40, 0, 40, 10, WHITE);

  display.setCursor(92, 3);
  display.printf("H:%.1f%%\n", humidity);
  display.drawLine(88, 12, 128,12, WHITE);
  display.drawLine(88, 0, 88, 12, WHITE);

  display.drawLine(0, 53, 128, 53, WHITE);
  display.drawLine(0, 43, 128, 43, WHITE);
  display.drawLine(50, 45, 50, 65, WHITE);
  display.drawLine(85, 45, 85, 65, WHITE);
  

  
  display.setCursor(95,45);
  display.printf("PM10");

  display.setCursor(20, 55);
  display.printf("%.0f",pm25);

  display.setCursor(105, 55);
  display.printf("%.0f",pm10);
  display.display();
}

void displayTemp()
{
  display.clearDisplay();
  display.setTextSize(1);
  drawCenter("TEMPERATURE", 0);

  display.setTextSize(3);
  drawCenter(String(temperature, 1) + " C", 25);
  display.display();
}

void displayHum()
{
  display.clearDisplay();
  display.setTextSize(1);
  drawCenter("HUMIDITY", 0);

  display.setTextSize(3);
  drawCenter(String(humidity, 1) + " %", 25);
  display.display();
}


void displayPM25()
{
  display.clearDisplay();
  display.setTextSize(1);
  drawCenter("PM2.5", 0);

  display.setTextSize(3);
  drawCenter(String(pm25, 0), 25);

  display.setTextSize(1);
  drawCenter("ug/m3", 55);
  display.display();
}

void displayPM10()
{
  display.clearDisplay();
  display.setTextSize(1);
  drawCenter("PM10", 0);

  display.setTextSize(3);
  drawCenter(String(pm10, 0), 25);

  display.setTextSize(1);
  drawCenter("ug/m3", 55);
  display.display();
}

void displayQuality()
{
  display.clearDisplay();
  display.setTextSize(1);

  String txt;
  if (aqi <= 50) txt = "GOOD AIR QUALITY";
  else if (aqi <= 100) txt = "DECENT AIR QUALITY";
  else if (aqi <= 135) txt = "UNHEALTHY (Sensitive)";
  else if (aqi <= 200) txt = "UNHEALTHY";
  else txt = "VERY UNHEALTHY!";

  drawCenter(txt, 26);
  display.display();
}
