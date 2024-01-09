#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include <stdlib.h>

#define DHTTYPE DHT11
uint8_t DHTPin = 17;

const int mqp = 34;
#define Photoresistor 4
const char* ssid = "ssid";
const char* password =  "pass";
String serverName = "https://ecsv.onrender.com/api/edge/CreateDevice";
uint64_t chipId;  
DHT dht(DHTPin, DHT11);

void connectWifi();
void calculateHumTem();
void addDevice();
void generateKey();
void addSensor();
void saveSensor();
void sendValue(String unit, float value, int sensorID);

void setup()
{
  Serial.begin(9600);
  Serial.println("Serial is on...");
  pinMode(mqp, INPUT);
  pinMode(15, INPUT);
  connectWifi();
  addDevice();
  saveSensor();
}

void loop()
{
  calculateHumTem();
  delay(10000);
}

void connectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network");
}

void addDevice()
{
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.println(chipId);
  String my_Api_key = "1234567";
  if(WiFi.status()== WL_CONNECTED){
   String url = "https://ecsv.onrender.com/api/edge/CreateDevice";
   String response;
   HTTPClient http;
   StaticJsonDocument<200> buff;
   String jsonParams;
   buff["deviceID"] = chipId;
   serializeJson(buff, jsonParams);
   http.begin(url);
   http.addHeader("Content-Type", "application/json");
   http.POST(jsonParams);
   response = http.getString();
   Serial.println(response);
  }
}

void addSensor(int sensorID, String type)
{
   if(WiFi.status()== WL_CONNECTED){
      String url = "https://ecsv.onrender.com/api/edge/SaveDevice/8138108";
      String response;
      HTTPClient http;
      StaticJsonDocument<200> buff;
      String jsonParams;
      buff["sensorID"] = sensorID;
      buff["type"] = type;
      
      http.begin(url);
      http.addHeader("Content-Type", "application/json");
      
      serializeJson(buff, jsonParams);
      http.PUT(String(jsonParams));
      response = http.getString();
      Serial.println(response);
  }
}

void encrypt_string(const char *input, uint8_t *key, uint8_t *iv, unsigned char* output) 
{
  mbedtls_aes_context aes; //aes is simple variable of given type
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 128);//set key for encryption
  //this is cryption function which encrypt data
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, strlen(input), iv, (unsigned char *)input, output);
}

void sendValue(String unit, int value, int sensorID)
{
  char result[8];
  dtostrf(value, 6, 2, result); // Leave room for too large numbers!
  /*char* key = "38108esp32device";
  char outputString[16];

  char* enc_key = "38108esp32device";
  char* enc_iv =  "1234567890123456";

  char result[8]; // Buffer big enough for 7-character float
  dtostrf(value, 6, 2, result); // Leave room for too large numbers!
  mbedtls_aes_context aes;
  unsigned char output[16];
  // encrypt
  encrypt_string(result, (uint8_t *)enc_key, (uint8_t *)enc_iv, output);

  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb( &aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)result, output);
  mbedtls_aes_free( &aes );

  int count = 0;
  for (int i = 0; i < 16; i++) {
 
    char str[3];
 
    sprintf(str, "%02x", (int)output[i]);
    if(count == 0)
      {
        strcpy(outputString,str);
      }
      else
      {
        strcat(outputString,str);
      }
      count++;
  }
  Serial.println();
  Serial.println(outputString);

  char testOutput[16];*/
  Serial.println(result);

  if(WiFi.status()== WL_CONNECTED){
      String url = "https://ecsv.onrender.com/api/edge/UpdateReading/" + String(sensorID);
      String response;
      HTTPClient http;
      StaticJsonDocument<200> buff;
      String jsonParams;
      buff["unit"] = unit;
      buff["value"] = result;

      http.begin(url);
      http.addHeader("Content-Type", "application/json");
      serializeJson(buff, jsonParams);
      http.PUT(String(jsonParams));
      response = http.getString();
      Serial.println(response);
  }
}

void saveSensor()
{ 
  addSensor(321,"temperature");
  addSensor(322,"humidity");
  addSensor(323,"pollution");
}

void calculateHumTem()
{ 
  
  pinMode(DHTPin, INPUT);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  dht.begin();
  //float humidityValue =dht.readHumidity();
  //float temperatureValue = dht.readTemperature();
  //int airValue = analogRead(mqp);

  float humidityValue = ((float)rand() / RAND_MAX) * 100.0;
  int temperatureValue = 50;
  int airValue = (rand() / RAND_MAX) * 100;

  //int limit = digitalRead(DOUTpin);
  Serial.print("Temperature: ");
  Serial.print(temperatureValue);
  Serial.print("ºC ");
  Serial.print("Humidity: ");
  Serial.println(humidityValue);
  Serial.print("Pollution: ");
  Serial.println(airValue);
  if(temperatureValue > 20)
  {
    digitalWrite(15, HIGH);
  }
  else
  {
    digitalWrite(15, LOW);
  }
  sendValue("celcius", temperatureValue, 125);
  //sendValue("g.m^-3", humidityValue, 126);
  //sendValue("ppm", airValue, 127);
}
