#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* WIFI_SSID="YOUR_WIFI_SSID";
const char* WIFI_PASSWORD="YOUR_WIFI_PASSWORD";
const char* API_URL="http://192.168.1.14:8765/api/v1/status";
const char* API_TOKEN="";

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_SDA D2
#define OLED_SCL D1
#define REFRESH_MS 2000
#define PAGE_MS 5000

Adafruit_SSD1306 display(OLED_WIDTH,OLED_HEIGHT,&Wire,-1);
unsigned long lastFetch=0,lastPage=0;
uint8_t page=0;
bool apiOnline=false;
float cpu=0,ram=0,tempC=NAN,rx=0,tx=0,diskUsed=0,diskTotal=0;
String hostIp="--",eth0State="--",eth1State="--",uptimeText="--";

void header(const char* label){
  display.clearDisplay(); display.setTextColor(SSD1306_WHITE); display.setTextSize(1);
  display.setCursor(0,0); display.print("AMOEBA  "); display.print(label);
  display.drawLine(0,10,127,10,SSD1306_WHITE);
}
void centerText(const String& value,int y,uint8_t size){
  display.setTextSize(size); int16_t x1,y1; uint16_t w,h;
  display.getTextBounds(value,0,y,&x1,&y1,&w,&h);
  display.setCursor(max(0,(128-(int)w)/2),y); display.print(value);
}
String formatUptime(unsigned long s){
  unsigned long d=s/86400; s%=86400; unsigned long h=s/3600; s%=3600;
  return String(d)+"d "+String(h)+"h "+String(s/60)+"m";
}
void fetchStatus(){
  if(WiFi.status()!=WL_CONNECTED){apiOnline=false;return;}
  WiFiClient client; HTTPClient http; http.setTimeout(1500);
  if(!http.begin(client,API_URL)){apiOnline=false;return;}
  if(strlen(API_TOKEN)) http.addHeader("Authorization",String("Bearer ")+API_TOKEN);
  int code=http.GET();
  if(code==HTTP_CODE_OK){
    StaticJsonDocument<2048> doc;
    if(!deserializeJson(doc,http.getStream())){
      cpu=doc["cpu_percent"]|0.0; ram=doc["memory"]["percent"]|0.0;
      tempC=doc["temperature_c"].isNull()?NAN:doc["temperature_c"].as<float>();
      diskUsed=doc["storage"]["used_gb"]|0.0; diskTotal=doc["storage"]["total_gb"]|0.0;
      hostIp=String((const char*)(doc["ip"]|"--"));
      uptimeText=formatUptime(doc["uptime_seconds"]|0UL);
      eth0State=String((const char*)(doc["network"]["eth0"]["state"]|"n/a"));
      eth1State=String((const char*)(doc["network"]["eth1"]["state"]|"n/a"));
      rx=doc["network"]["eth1"]["rx_mbps"]|0.0; tx=doc["network"]["eth1"]["tx_mbps"]|0.0;
      apiOnline=true;
    }else apiOnline=false;
  }else apiOnline=false;
  http.end();
}
void drawSystem(){
  header("SYSTEM"); centerText(isnan(tempC)?"-- C":String(tempC,1)+" C",15,2);
  display.setTextSize(1); display.setCursor(0,40); display.printf("CPU  %4.1f%%",cpu);
  display.setCursor(0,52); display.printf("RAM  %4.1f%%",ram);
  display.setCursor(82,52); display.print("ONLINE"); display.display();
}
void drawNetwork(){
  header("NETWORK"); display.setTextSize(1);
  display.setCursor(0,15); display.print("STB IP: "); display.print(hostIp);
  display.setCursor(0,29); display.printf("DOWN %5.2f Mbps",rx);
  display.setCursor(0,41); display.printf("UP   %5.2f Mbps",tx);
  display.setCursor(0,53); display.print("eth0 "); display.print(eth0State);
  display.setCursor(75,53); display.print("eth1 "); display.print(eth1State); display.display();
}
void drawStorage(){
  header("STORAGE"); display.setTextSize(1); display.setCursor(0,17); display.print("HDD USED");
  centerText(String(diskUsed,1)+" / "+String(diskTotal,1)+" GB",31,2);
  display.setCursor(0,55); display.print("/mnt/amoeba"); display.display();
}
void drawStatus(){
  header("STATUS"); display.setTextSize(1);
  display.setCursor(0,16); display.print("API: ONLINE");
  display.setCursor(0,29); display.print("WiFi: CONNECTED");
  display.setCursor(0,42); display.print("Uptime: "); display.print(uptimeText);
  display.setCursor(0,55); display.print("Node IP: "); display.print(WiFi.localIP()); display.display();
}
void drawDisplay(){
  if(!apiOnline){
    header("CONNECTING"); display.setCursor(0,20);
    display.print(WiFi.status()==WL_CONNECTED?"API unavailable":"Joining WiFi...");
    display.setCursor(0,36); display.print("Check router / API"); display.display(); return;
  }
  switch(page){case 0:drawSystem();break;case 1:drawNetwork();break;case 2:drawStorage();break;default:drawStatus();}
}
void setup(){
  Serial.begin(115200); Wire.begin(OLED_SDA,OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC,OLED_ADDR)){Serial.println("SSD1306 init failed");while(true)delay(1000);}
  display.clearDisplay(); centerText("AMOEBA",20,2); centerText("STB MONITOR",42,1); display.display();
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID,WIFI_PASSWORD); lastFetch=millis()-REFRESH_MS;
}
void loop(){
  unsigned long now=millis();
  if(now-lastFetch>=REFRESH_MS){lastFetch=now;fetchStatus();drawDisplay();}
  if(now-lastPage>=PAGE_MS){lastPage=now;page=(page+1)%4;drawDisplay();}
  delay(20);
}
