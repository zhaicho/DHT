/*
ESP01,DHT11,DHT22 to Thingspeak;
DHT部分來自程式庫towsensor範例;
修改本來兩個DHT11為DHT11+DHT22;
只需要修改DHT.read及byte&float定義就可以修改sensor類型;
31/07/2022 zhaicho;
18/08/2022增加WIFI睡眠;
*/
#include <SimpleDHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "Youer WIFI Name"
#define STAPSK "Youer password"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

//資料庫地址;
const char *apiKey = "Youer APIKEY";
const char *resource = "/update?api_key=";

// Thing Speak API server
const char *server = "api.thingspeak.com";

int dataPinSensor1 = 2;
int dataPinSensor2 = 0;
SimpleDHT11 dht1(dataPinSensor1);
SimpleDHT22 dht2(dataPinSensor2);

void setup()
{
  Serial.begin(115200);
  //網絡連接;
  Serial.print("Connecting to:");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("CONNECTED!!!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  //讀取溫度並輸出終端機;
  Serial.println("=================================");
  Serial.println("Getting data from sensor 1...");

  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht1.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
  {
    Serial.print("Read Sensor 1 failed, err=");
    Serial.print(SimpleDHTErrCode(err));
    Serial.print(",");
    Serial.println(SimpleDHTErrDuration(err));
    delay(1000);
    // return;//返回重複連接Sensor;
  }

  byte t = temperature;
  byte h = humidity;
  Serial.print("DHT11: ");
  Serial.print((int)temperature);
  Serial.print(" 'C, ");
  Serial.print((int)humidity);
  Serial.println(" %");
  delay(1000);

  Serial.println("Getting data from sensor 2...");

  float temperature2 = 0;
  float humidity2 = 0;
  if ((err = dht2.read2(&temperature2, &humidity2, NULL)) != SimpleDHTErrSuccess)
  {
    Serial.print("Read DHT22 failed, err=");
    Serial.print(SimpleDHTErrCode(err));
    Serial.print(",");
    Serial.println(SimpleDHTErrDuration(err));
    delay(2000);
    // return;//返回重複連接Sensor;
  }

  float t2 = (int)temperature2 - 2;
  float h2 = (int)humidity2 + 15;
  Serial.print("DHT22: ");
  Serial.print(t2);
  Serial.print("'C, ");
  Serial.print(h2);
  Serial.println("%");
  Serial.println("=================================");
  Serial.println("");

  //開始傳輸到資料庫;
  WiFiClient client;
  if (client.connect(server, 80))
  {
    Serial.println(F("connected"));
  }
  else
  {
    Serial.println(F("connection failed"));
    return;
  }
  Serial.print("Request resource: ");
  Serial.println(resource);
  //防止sensor錯誤時上傳信息為0;
  if (h != 0 && h2 > 30)
  {
    client.print(String("GET ") + resource + apiKey + "&field1=" + t + "&field2=" + h + "&field3=" + t2 + "&field4=" + h2 +
                 " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
  }
  else if (h == 0 && h2 > 30)
  {
    client.print(String("GET ") + resource + apiKey + "&field3=" + t2 + "&field4=" + h2 +
                 " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
  }
  else if (h2 <= 30 && h != 0)
  {
    client.print(String("GET ") + resource + apiKey + "&field1=" + t + "&field2=" + h +
                 " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
  }
  else
  //如果兩個sensor都錯誤,全部上傳"0"信息;
  {
    client.print(String("GET ") + resource + apiKey + "&field1=" + t + "&field2=" + h + "&field3=" + t2 + "&field4=" + h2 +
                 " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
  }

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0))
  {
    delay(100);
  }

  if (!client.available())
  {
    Serial.println("No response, going back to sleep");
  }
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("\nclosing connection");
  Serial.println("=================================");
  client.stop();
  //調節器睡眠無需檢查WIFI狀態,WAKE命令後會自動重連;
  WiFi.forceSleepBegin(); // WIFI睡眠;
  Serial.println("WIFI going to Sleep");
  delay(1000 * 5*60);
  WiFi.forceSleepWake(); // WIFI睡眠結束;
  Serial.println("WIFI Wake up");
  Serial.println("");
}
