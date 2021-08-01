#include <WiFi.h>
#include "time.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "SPI.h"
#include <Arduino_GFX_Library.h>
#include "FreeSansBold18ptClock.h"
#include "FreeSans10pt7b.h"
#include "pf10pt.h"
#include "bg.h"
#include "qrcode.h"

Arduino_DataBus *bus = new Arduino_ESP32SPI(25 /* RS */, 15 /* CS */,  14/* SCK */, 13 /* MOSI */, -1 /* MISO */, HSPI /* spi_num */);

Arduino_GFX *gfx =new Arduino_ILI9225(bus, 26 /* RST */,1);

#define BACKGROUND 0x0000



// #define TFT_RST 26  // IO 26
// #define TFT_RS  25  // IO 25
// #define TFT_CLK 14  // HSPI-SCK
// #define TFT_SDI 13  // HSPI-MOSI
// #define TFT_CS  15  // HSPI-SS0
// #define TFT_LED 0   // 0 if wired to +5V directly

int y=0;
/*
'rh': '相对湿度(%)',
'temp': '温度',
'created': '发布时间',
'pvdrCap': '天气条件',
'pvdrWindDir': '风向',
'pvdrWindSpd': '风级',
'aqi': '空气质量指数',
'aqiSeverity': '空气质量',
*/


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 28800;
const int   daylightOffset_sec = 0;



float lat = 0.0;
float lon = 0.0;
String city = "";
int temp = 0;
String pvdrCap = "";
int rh = 0;
float aqi = 0.0;
String aqiSeverity = "";
float windSpd=0.0;


void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %Y-%m-%d %H:%M:%S");
}

void showtime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  char datestr[20];
  char tmstr[20];
  time_t rawtime;
  struct tm *info;
  time(&rawtime);
  info = localtime( &rawtime );

  strftime(datestr, 20, "%Y-%m-%d %A", info);
  strftime(tmstr, 20, "%H:%M", info);
  // gfx->draw24bitRGBBitmap(0,0,,220,176)
  gfx->draw24bitRGBBitmap(0,0,bmp_bg,220,176);
  gfx->setCursor(22, 70);
  gfx->setFont(&FreeSansBold18pt7b);
  gfx->setTextColor(BLACK);
  gfx->println(tmstr);
  gfx->setCursor(2,170);
  gfx->setFont(&FreeSans10pt7b);
  gfx->setTextColor(BLACK);
  gfx->println(datestr);
  Serial.println(datestr);
  Serial.println(tmstr);
  showWeather();
}
void printcn(String txt) {
  Serial.println(txt);
  txt.replace("东", "\x7f");
  txt.replace("严", "\x80");
  txt.replace("中", "\x81");
  txt.replace("云", "\x82");
  txt.replace("优", "\x83");
  txt.replace("伴", "\x84");
  txt.replace("冰", "\x85");
  txt.replace("冷", "\x86");
  txt.replace("冻", "\x87");
  txt.replace("劲", "\x88");
  txt.replace("北", "\x89");
  txt.replace("南", "\x8a");
  txt.replace("卷", "\x8b");
  txt.replace("和", "\x8c");
  txt.replace("多", "\x8d");
  txt.replace("大", "\x8e");
  txt.replace("天", "\x8f");
  txt.replace("夹", "\x90");
  txt.replace("小", "\x91");
  txt.replace("少", "\x92");
  txt.replace("尘", "\x93");
  txt.replace("市", "\x94");
  txt.replace("带", "\x95");
  txt.replace("平", "\x96");
  txt.replace("并", "\x97");
  txt.replace("度", "\x98");
  txt.replace("强", "\x99");
  txt.replace("微", "\x9a");
  txt.replace("扬", "\x9b");
  txt.replace("晴", "\x9c");
  txt.replace("暴", "\x9d");
  txt.replace("有", "\x9e");
  txt.replace("朗", "\x9f");
  txt.replace("极", "\xa0");
  txt.replace("染", "\xa1");
  txt.replace("毛", "\xa2");
  txt.replace("气", "\xa3");
  txt.replace("江", "\xa4");
  txt.replace("污", "\xa5");
  txt.replace("沙", "\xa6");
  txt.replace("浓", "\xa7");
  txt.replace("浮", "\xa8");
  txt.replace("清", "\xa9");
  txt.replace("烈", "\xaa");
  txt.replace("热", "\xab");
  txt.replace("爆", "\xac");
  txt.replace("特", "\xad");
  txt.replace("狂", "\xae");
  txt.replace("疾", "\xaf");
  txt.replace("省", "\xb0");
  txt.replace("空", "\xb1");
  txt.replace("端", "\xb2");
  txt.replace("细", "\xb3");
  txt.replace("良", "\xb4");
  txt.replace("西", "\xb5");
  txt.replace("轻", "\xb6");
  txt.replace("重", "\xb7");
  txt.replace("镇", "\xb8");
  txt.replace("间", "\xb9");
  txt.replace("阳", "\xba");
  txt.replace("阴", "\xbb");
  txt.replace("阵", "\xbc");
  txt.replace("降", "\xbd");
  txt.replace("雨", "\xbe");
  txt.replace("雪", "\xbf");
  txt.replace("雷", "\xc0");
  txt.replace("雹", "\xc1");
  txt.replace("雾", "\xc2");
  txt.replace("霾", "\xc3");
  txt.replace("静", "\xc4");
  txt.replace("风", "\xc5");
  txt.replace("飓", "\xc6");
  txt.replace("龙", "\xc7");

  gfx->println(txt);
}


void showWeather(){
  gfx->setFont(&pf_min_ys10pt8b);
  gfx->setTextColor(BLACK);

  gfx->setCursor(28,16);
  printcn(city);

  gfx->setCursor(79,16);
  printcn(pvdrCap);

  gfx->setCursor(28,100+12);
  gfx->println(String(temp));

  gfx->setCursor(90,100+12);
  gfx->println(String(rh));

  gfx->setCursor(28,128+12);
  printcn(aqiSeverity);

  gfx->setCursor(110,128+12);
  gfx->println(String(int(windSpd))+"km/h");

}
void SmartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  // tftprint("Please go to: http://wx.ai-thinker.com/api/old/wifi/config");
  gfx->fillScreen(BACKGROUND);
  gfx->drawXBitmap(10,10,gImage_code,100,100,WHITE);
  gfx->setCursor(10,120);
  gfx->setFont();
  gfx->printf("Please use WeChat and scan the QRcode to config WIFI.");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    delay(1000);                       // wait for a second
    if (WiFi.smartConfigDone())
    {
      gfx->fillScreen(BACKGROUND);
      y=0;
      Serial.println("\r\nSmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      WiFi.setAutoConnect(true);  // 设置自动连接。
      tftprint("Connected to :" + String((WiFi.SSID().c_str())));
      tftprint(String(WiFi.localIP()));
      delay(1000);
      break;
    }
  }
}

bool AutoConfig()
{
  WiFi.begin();
  for (int i = 0; i < 25; i++)
  {
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED )
    {
      Serial.println("\r\nsmartConfig success");
      Serial.printf("SSID:%s", WiFi.SSID().c_str());
      Serial.printf(",PWS:%s\r\n", WiFi.psk().c_str());
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      tftprint("Connected to :" + String((WiFi.SSID().c_str())));
      tftprint(String(WiFi.localIP()));
      delay(1000);
      return true;

    }
    else
    {
      Serial.println("WIFI AutoConfig Waiting ...");
      tftprint("Waiting for connect...");
      // Serial.println(wstatus);
      delay(1000);
    }

  }
  Serial.println("Wifi autoconfig faild!");
  tftprint("AutoConfig faild!");
  return false;


}
void tftprint(String txt) {
  // int offset=sizeof(txt);
  gfx->setCursor(10, 10+y);
  gfx->setTextColor(WHITE);
  gfx->println(txt);
  y = y + 10;
  if (y + 10 > 140) {
    y = 0;
    gfx->fillScreen(BACKGROUND);
  }
}
void setup() {
  Serial.begin(115200);
  gfx->begin();
  gfx->fillScreen(BACKGROUND);
  // w = gfx->width();
  // h = gfx->height();
  delay(100);
  tftprint("Welcome!");
  if (!AutoConfig())
  {
  	SmartConfig();
  }
  tftprint("Configing time...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  tftprint("Getting location...");
  getlocation();
  tftprint("Getting weather...");
  getweather();
}
void getlocation() {
  DynamicJsonDocument doc(1024);
  HTTPClient http;
  http.begin("http://api.bilibili.com/x/web-interface/zone");
  int httpCode = http.GET();
  if (httpCode == 200)
  {
    Serial.println("Get OK");
    String resBuff = http.getString();
    Serial.println(resBuff);
    deserializeJson(doc, resBuff);
    JsonObject root = doc.as<JsonObject>();
    city = root["data"]["city"].as<String>();
    lat = root["data"]["latitude"];
    lon = root["data"]["longitude"];
    tftprint("City:"+city);
    Serial.print(city);
    Serial.print(lat);
    Serial.println(lon);
  }
  else {
    Serial.println("Location get error!");
  }


  http.end(); // 结束当前连接
}
void getweather() {
  DynamicJsonDocument doc(1024);
  HTTPClient http;
  String url="https://api.msn.com/weather/overview?locale=zh-cn&lat="+String(lat)+"&lon="+String(lon)+"&units=C&market=China&region=cn&appId=4de6fc9f-3262-47bf-9c99-e189a8234fa2&apiKey=UhJ4G66OjyLbn9mXARgajXLiLw6V75sHnfpU60aJBB&ocid=weather-peregrine&wrapOData=false";
  http.begin("https://api.msn.cn/weather/current?latLongList=33.13815%2C111.49078&locale=zh-cn&units=C&appId=9e21380c-ff19-4c78-b4ea-19558e93a5d3&apiKey=j5i4gDqHL6nGYwx5wi5kRhXjtf2c5qgFX9fzfk0TOo&ocid=msftweather&wrapOData=false");
  // http.begin(url);
  Serial.println(url);
  int httpCode = http.GET();
  if (httpCode == 200)
  {
    Serial.println("Get OK");
    String resBuff = http.getString();
    Serial.println(resBuff);
    deserializeJson(doc, resBuff);
    JsonObject root = doc.as<JsonObject>();
    JsonObject weather=root["responses"][0]["weather"][0]["current"];
    pvdrCap=weather["cap"].as<String>();
    temp=weather["temp"];
    rh=weather["rh"];
    aqi=weather["aqi"];
    aqiSeverity=weather["aqiSeverity"].as<String>();
    windSpd=weather["windSpd"];
    Serial.println(weather);
  }
  else {
    Serial.println("Location get error!");
  }


  http.end(); // 结束当前连接
}
void loop() {
  delay(5000);
  printLocalTime();
  showtime();
}
