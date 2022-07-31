#include <SimpleDHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "Hou lan ging"
#define STAPSK "houyaoying"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

//資料庫地址;
const char *apiKey = "NWRBFUSVAPMZAT6A";
const char *resource = "/update?api_key=";

// Thing Speak API server
const char *server = "api.thingspeak.com";

int dataPinSensor1 = 0;
int dataPinSensor2 = 2;
SimpleDHT11 dht1(dataPinSensor1);
SimpleDHT22 dht2(dataPinSensor2);

void setup()
{
  Serial.begin(115200);
  //網絡連接;
  Serial.print("準備WIFI連綫.名稱:");
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
  // Reading data from sensor 1...
  Serial.println("=================================");

  // Reading data from sensor 1...
  Serial.println("Getting data from sensor 1...");

  // read without samples.
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

  // converting Celsius to Fahrenheit
  // byte f = temperature * 1.8 + 32;
  byte t = temperature;
  byte h = humidity;
  Serial.print("DHT11: ");
  Serial.print((int)temperature);
  Serial.print(" 'C, ");
  // Serial.print((int)f); Serial.print(" *F, ");
  Serial.print((int)humidity);
  Serial.println(" %");
  delay(1000);

  // Reading data from sensor 2...
  // ============================
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

  // converting Celsius to Fahrenheit
  // float fb = temperature2-6 * 1.8 + 32;
  float t2 = (int)temperature2 - 5;
  float h2 = (int)humidity2 + 30;
  Serial.print("DHT22: ");
  Serial.print(t2);
  Serial.print("'C, ");
  // Serial.print((int)fb); Serial.print(" *F, ");
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
  Serial.println("");
  client.stop();
  delay(1000 * 60);
}
