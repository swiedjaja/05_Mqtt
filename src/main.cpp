/*
Test MQTT:
- subcribe ke topik "esp32_test/sub
- publish message ke topik "esp32_test/pub"

cara test program:
- Install mosquitto di Windows/Linux
- jalankan di CMD 1 mqtt client untuk subsribe ke topic yg di publish oleh ESP
   mosquitto_sub -h broker.emqx.io -t "esp32_test/pub"
- jalankan di CMD 2 mqtt client untuk publish message ke  ESP
   mosquitto_pub -h broker.emqx.io -t "esp32_test/sub" -m "Hello"
*/
#include <Arduino.h>
#include <Ticker.h>
#include <PubSubClient.h>
#if defined(ESP32)  
  #include <WiFi.h>
#endif  
#include "device.h"

const char* ssid = "Steff-IoT";
const char* password = "steffiot123";

#define MQTT_BROKER  "broker.emqx.io"
// #define MQTT_BROKER  "52.32.182.17" 
#define MQTT_TOPIC_PUBLISH   "esp32_test/pub"
#define MQTT_TOPIC_SUBSCRIBE "esp32_test/sub"
WiFiClient wifiClient;
PubSubClient  mqtt(wifiClient);

Ticker timerPublish;
int nMsgCount=0;

char g_szDeviceId[30];

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}

void onPublishMessage()
{
  char szMsg[50];
  sprintf(szMsg, "Hello from %s - %d", g_szDeviceId, nMsgCount++);
  mqtt.publish(MQTT_TOPIC_PUBLISH, szMsg);
}

boolean mqttConnect() {
#if defined(ESP32)  
  sprintf(g_szDeviceId, "esp32_%08X",(uint32_t)ESP.getEfuseMac());
#endif  
  mqtt.setServer(MQTT_BROKER, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.printf("Connecting to %s clientId: %s\n", MQTT_BROKER, g_szDeviceId);

 // Connect to MQTT Broker
  // Or, if you want to authenticate MQTT:
  boolean status = mqtt.connect(g_szDeviceId);

  if (status == false) {
    Serial.print(" fail, rc=");
    Serial.print(mqtt.state());
    return false;
  }
  Serial.println(" success");

  mqtt.subscribe(MQTT_TOPIC_SUBSCRIBE);
  Serial.printf("Subcribe topic: %s\n", MQTT_TOPIC_SUBSCRIBE);
  onPublishMessage();
  return mqtt.connected();
}

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
#if defined(ESP32)  
  Serial.printf("Free Memory: %d\n", ESP.getFreeHeap());
#endif 
  WifiConnect();
  mqttConnect();
  timerPublish.attach_ms(3000, onPublishMessage);
}

void loop() {
    mqtt.loop();
}