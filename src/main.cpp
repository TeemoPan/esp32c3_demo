#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_AHTX0.h>
#include "TM1637.h"
#include "NTPClient.h"
#include "WiFi.h"
#include "WiFiUdp.h"

TM1637 tm1637(8, 9);

int8_t timeZone = 8;
const PROGMEM char *ntpServer = "ntp1.aliyun.com";
int hour;
int minute;

const char *ssid = "ZW";
const char *password = "zhangwei..";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TFT_eSPI tft;
TFT_eSprite drawBuf(&tft);
Adafruit_AHTX0 aht;
sensors_event_t humidity_, temp_;

// 温湿度传感器引脚初始化
void initializeSensors()
{
  Wire.begin(4, 5);
  aht.begin(&Wire);
}

// 连接WiFi网络
void connectToWiFi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}

// 初始化时间客户端
void initializeTimeClient()
{
  timeClient.begin();
  timeClient.setPoolServerName(ntpServer);
  timeClient.setTimeOffset(timeZone * 3600);
}

// 初始化TM1637
void initializeTM1637()
{
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
}

// 显示温度和湿度
void displayTemperatureAndHumidity()
{
  aht.getEvent(&humidity_, &temp_);

  // 将温度和湿度转换为字符串，并打印到屏幕上
  String temperatureString = String(temp_.temperature, 1);
  String humidityString = String(humidity_.relative_humidity, 1);
  drawBuf.drawString("Temperature: " + temperatureString + " °C", 2, 0);
  drawBuf.drawString("Humidity:" + humidityString + " %", 2, 20);

  Serial.print("Temperature: ");
  Serial.print(temp_.temperature);
  Serial.println(" degrees C");
  Serial.print("Humidity: ");
  Serial.print(humidity_.relative_humidity);
  Serial.println("% rH");
  drawBuf.pushSprite(0, 0);
}

// 显示当前时间
void displayCurrentTime()
{
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());

  hour = timeClient.getHours();
  minute = timeClient.getMinutes();

  static bool pointState = true;
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis >= 500) {
    lastMillis = millis();
    pointState = !pointState;
  }

  tm1637.point(pointState);
  int8_t timeArray[4];
  timeArray[0] = hour / 10;
  timeArray[1] = hour % 10;
  timeArray[2] = minute / 10;
  timeArray[3] = minute % 10;
  tm1637.display(timeArray);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello ESP32C3!!");

  tft.begin();
  drawBuf.createSprite(TFT_WIDTH, TFT_HEIGHT);
  drawBuf.setTextFont(2);
  drawBuf.fontHeight(10);
  drawBuf.fillScreen(TFT_BLACK);

  initializeSensors();
  connectToWiFi();
  initializeTimeClient();
  initializeTM1637();
}

void loop()
{
  drawBuf.fillScreen(TFT_BLACK);
  displayTemperatureAndHumidity();
  displayCurrentTime();
  delay(1000);
}