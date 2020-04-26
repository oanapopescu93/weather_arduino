#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include "time.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "DHT.h"
#define DHTPIN 21
#define DHTTYPE DHT21

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const char* ssid       = "UPCFB8C751";
const char* password   = "npz6jzMpbjzw";

//const char* ssid       = "Idrive";
//const char* password   = "idrive@2019";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

DHT dht(DHTPIN, DHTTYPE);

void setup(void) {
//  pinMode(9, OUTPUT);
//  digitalWrite(9, 0); // default output in I2C mode for the SSD1306 test shield: set the i2c adr to 0
  u8g2.begin();
  Serial.begin(115200); 
  dht.begin();

  int retries = 0;
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if (retries == 10) {
      break;
    }
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  
  
  printLogo();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void printLogo(void){
    u8g2.clearBuffer();
    u8g2.setFontMode(1);  // Transparent

    u8g2.setFontDirection(0);
    //u8g2.setFont(u8g2_font_inb16_mf);
    u8g2.setFont(u8g2_font_unifont_t_extended);
    u8g2.drawStr(0,20, "Weather");
    u8g2.drawUTF8(0,40, "Pârț");

    u8g2.sendBuffer();  
    delay(1000);
}

void printCalendar(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo);
  //Serial.println("%A, %B %d %Y %H:%M:%S");
  //https://docs.oracle.com/cd/E19857-01/817-6248-10/crtime.html

  char myCalendar[15];
  strftime(myCalendar, sizeof(myCalendar), "%a, %b %d %y %H:%M", &timeinfo);
  
  //Serial.println(myCalendar);
  u8g2.setFont(u8g2_font_chroma48medium8_8u);
  u8g2.drawStr(0, 15, myCalendar);  
}

void printTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  
  char myTime[15];
  strftime(myTime, sizeof(myTime), "%H:%M:%S", &timeinfo);
  
  //Serial.println(myTime);
  u8g2.setFont(u8g2_font_chroma48medium8_8u);
  u8g2.drawStr(0, 25, myTime);  
}

void printWeather(){  
//  char temperature[15];
//  char humidity[15];
//
//  strcpy(temperature,"Temperature: ");
//  strcpy(humidity,"Humidity: ");
//  
//  Serial.println(temperature);
//  Serial.println(humidity);
//  
//  u8g2.drawStr(0, 50, temperature);  
//  u8g2.drawStr(0, 65, humidity); 

  u8g2.setFont(u8g2_font_chroma48medium8_8u);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.println(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));


char h_char[6];
char t_char[6];
char f_char[6];
char hic_char[6];
char hif_char[6];

String h_string;
String t_string;
String f_string;
String hic_string;
String hif_string;

snprintf (h_char, sizeof(h_char), "%f", h);
h_string = String("H: " + h_string + h_char);


snprintf (t_char, sizeof(t_char), "%f", t);
t_string = String("T: " + t_string + t_char);

snprintf (f_char, sizeof(f_char), "%f", f);
f_string = String(f_string + f_char);
t_string = String(t_string + "°C " + f_string + "°F");


snprintf (hic_char, sizeof(hic_char), "%f", hic);
hic_string = String("HT: " + hic_string + hic_char);

snprintf (hif_char, sizeof(hif_char), "%f", hif);
hif_string = String(hif_string + hif_char);
hic_string = String(hic_string + "°C " + f_string + "°F");

u8g2.setCursor(0, 42);
u8g2.print(t_string);

u8g2.setCursor(0, 52);
u8g2.print(h_string);

u8g2.setCursor(0, 62);
u8g2.print(hic_string);
}

void loop() {
  u8g2.clearBuffer();
  
  printCalendar();
  printTime();
  printWeather();
  
  u8g2.sendBuffer();  
  delay(1000);
}
