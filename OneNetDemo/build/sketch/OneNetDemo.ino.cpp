#include <Arduino.h>
#line 1 "d:\\arduino\\OneNetDemo\\OneNetDemo.ino"
#line 1 "d:\\arduino\\OneNetDemo\\OneNetDemo.ino"
/**
 * author: topthemaster
 * time: 2021.1.6
 * 
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#define PRODUCT_ID    "512027" //产品名
#define API_KEY    "z7Y54y4QQr3fkA=bV4Ajv3yS1qM="//产品密钥
#define DEVICE_ID "935987631"//设备名
#define TOPIC     "yyf123"//订阅主题

int n=10;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
const char* ssid = "Mi 10";
const char* password = "582975413";
const char* mqttServer = "183.230.40.39";//onenet地址
const uint16_t mqttPort = 6002;//mqtt接口端口
Ticker ticker;
int count;  
char msgJson[75];
char msg_buf[200];
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // 设置板上LED引脚为输出模式
  digitalWrite(LED_BUILTIN, HIGH);  // 启动后关闭板上LED
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  connectWifi();
  mqttClient.setServer(mqttServer,mqttPort);
  // 设置MQTT订阅回调函数
  mqttClient.setCallback(receiveCallback);
  connectMQTTServer();
  ticker.attach(1, tickerCount);  
}

void loop() {

  if (mqttClient.connected()) { // 如果开发板成功连接服务器
    // 每隔3秒钟发布一次信息  
    // 保持心跳
    if (count >= 3){
      pubMQTTmsg();
      count = 0;
    }  
    mqttClient.loop();
  } else {                  // 如果开发板未能成功连接服务器
    connectMQTTServer();    // 则尝试连接服务器
  }
}
void tickerCount(){
  count++;
}
void connectMQTTServer(){
  String clientId = DEVICE_ID;
  String productId=PRODUCT_ID;
  String apiKey=API_KEY;
  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str(),productId.c_str(),apiKey.c_str())) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);
    subscribeTopic(); // 订阅指定主题
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }   
}
// 订阅指定主题
void subscribeTopic(){
 
  // 建立订阅主题。主题名称以Taichi-Maker-Sub为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同设备使用同一个MQTT服务器测试消息订阅时，所订阅的主题名称不同
  String topicString = "temperature";
  char subTopic[topicString.length() + 1];  
  strcpy(subTopic, topicString.c_str());
  
  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if(mqttClient.subscribe(subTopic)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic);
  } else {
    Serial.print("Subscribe Fail...");
  }  
}
 void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
 //测试下发数据
  if ((char)payload[0] == '1') {     // 如果收到的信息以“1”为开始
    digitalWrite(BUILTIN_LED, LOW);  // 则点亮LED。
    Serial.println("LED ON");
  } else if((char)payload[0] == '0'){                           
    digitalWrite(BUILTIN_LED, HIGH); // 否则熄灭LED。
    Serial.println("LED OFF");
  }
}
void pubMQTTmsg(){
  //onenet数据点上传系统主题
  String topicString = "$dp";
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());
  //json数据转换为数组
  DynamicJsonDocument doc(16);
n++;
  doc["temperature"] = n;

  serializeJson(doc, Serial);
  // 建立发布信息。温度
  String jsonCode;  
  serializeJson(doc, jsonCode);
  Serial.print("json Code: ");Serial.println(jsonCode); 
  String messageString = jsonCode; 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  int json_len=strlen(publishMsg);
  memset(msg_buf,0,200);
  msg_buf[0]=char(0x03);
  msg_buf[1]=char(json_len>>8);
  msg_buf[2]=char(json_len &0xff);
  memcpy(msg_buf+3,publishMsg,json_len);
  // 实现ESP8266向主题发布信息
  if(mqttClient.publish(publishTopic, (uint8_t*)msg_buf,3+json_len)){
    Serial.println("Publish Topic:");Serial.println(publishTopic);
    String msg_bufTotal;
    for(int i=0;i<sizeof(msg_buf)/sizeof(msg_buf[0]);i++)
    {
      msg_bufTotal+=msg_buf[i];
    }
    Serial.println("Publish message:");Serial.println(msg_bufTotal);    
  } else {
    Serial.println("Message Publish Failed."); 
  }
}

void connectWifi(){
 
  WiFi.begin(ssid, password);
 
  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");  
  Serial.println(""); 
}


