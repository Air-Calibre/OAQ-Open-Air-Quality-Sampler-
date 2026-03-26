#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ----------- PINS -----------
#define RX_PIN 4
#define TX_PIN 5
#define DHTPIN 7
#define DHTTYPE DHT22

// ----------- OLED -----------
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ----------- SENSORS --------
HardwareSerial zhSerial(1);
DHT dht(DHTPIN, DHTTYPE);

// ----------- DATA -----------
float pm25 = 0, pm10 = 0;
float temperature = 0, humidity = 0;

// ----------- UI FUNCTION ----
void drawCenter(String s, int y, int size)
{
  display.setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, y);
  display.print(s);
}

// ----------- ZH03 READ ------
void readZH03()
{
  while (zhSerial.available() >= 2)
  {
    if (zhSerial.read() == 0xFF && zhSerial.read() == 0x86)
    {
      uint8_t buf[22];
      zhSerial.readBytes(buf, 22);

      pm25 = ((buf[2] << 8) | buf[3]) / 10.0;
      pm10 = ((buf[4] << 8) | buf[5]) / 10.0;
      return;
    }
  }
}

// ----------- SETUP ----------
void setup()
{
  Serial.begin(115200);

  zhSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  dht.begin();

  Wire.begin(8, 9);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Force ZH03 active mode
  uint8_t cmd[] = {0xFF,0x01,0x78,0x40,0x00,0x00,0x00,0x00,0x47};
  zhSerial.write(cmd, 9);

  display.clearDisplay();
}

// ----------- LOOP -----------
void loop()
{
  // Read sensors
  readZH03();
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // -------- UI -------------
  display.clearDisplay();

  // Header
  drawCenter("AIR MONITOR", 0, 1);
  display.drawLine(0, 10, 128, 10, WHITE);

  // PM2.5
  display.drawRect(0, 12, 64, 26, WHITE);
  drawCenter("PM2.5", 14, 1);
  drawCenter(String(pm25,1), 24, 1);

  // PM10
  display.drawRect(64, 12, 64, 26, WHITE);
  drawCenter("PM10", 14, 1);
  drawCenter(String(pm10,1), 24, 1);

  // Temperature
  display.drawRect(0, 40, 64, 24, WHITE);
  drawCenter("TEMP", 42, 1);
  drawCenter(String(temperature,1) + "C", 52, 1);

  // Humidity
  display.drawRect(64, 40, 64, 24, WHITE);
  drawCenter("HUM", 42, 1);
  drawCenter(String(humidity,1) + "%", 52, 1);

  display.display();

  // Serial debug
  Serial.printf("PM2.5=%.1f PM10=%.1f T=%.1f H=%.1f\n",
                pm25, pm10, temperature, humidity);

  delay(500);
}