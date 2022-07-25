#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <Arduino.h>
#include <Wire.h>

const char *ssid = "Mi 10";
const char *password = "12345678";
const char *mqttServer = "test.ranye-iot.net";

// olde12864显示屏初始化
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/D4, /* data=*/D3, /* reset=*/U8X8_PIN_NONE); // All Boards without Reset of the Display

Ticker ticker;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int count;

void setup()
{
  Serial.begin(9600);
  u8g2.begin();
  //设置ESP8266工作模式为无线终端模式
  WiFi.mode(WIFI_STA);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D8, OUTPUT);
  u8g2.enableUTF8Print();
  // 连接WiFi
  connectWifi();

  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 1883);

  // 连接MQTT服务器
  connectMQTTServer();

  // Ticker定时对象
  ticker.attach(1, tickerCount);
}

void loop()
{
  if (mqttClient.connected())
  { // 如果开发板成功连接服务器
    // 每隔3秒钟发布一次信息
    if (count >= 3)
    {
      pubMQTTmsg();
      count = 0;
    }
    // 保持心跳
    mqttClient.loop();
  }
  else
  {                      // 如果开发板未能成功连接服务器
    connectMQTTServer(); // 则尝试连接服务器
  }
}

void tickerCount()
{
  count++;
}

void connectMQTTServer()
{
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "esp8266-" + WiFi.macAddress();

  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str()))
  {
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);

    u8g2.clearDisplay();
    u8g2.firstPage();
    do
    {

      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(0, 24, "MQTT Server");
      u8g2.drawStr(14, 40, " Connected");
      u8g2.drawStr(30, 55, "Success!!!");

    } while (u8g2.nextPage());
  }
  else
  {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }
}

// 发布信息
void pubMQTTmsg()
{
  static int value, svalue; // 客户端发布信息用数字
  value = analogRead(A0);
  svalue = 100 - map(value, 0, 1023, 100, 0);
  //把电压值按照[0,1023]映射到[100,0]
  // 建立发布主题。主题名称以Taichi-Maker-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP8266客户端名称各不相同，
  String topicString = "shui" + WiFi.macAddress();
  char publishTopic[topicString.length() + 1];
  strcpy(publishTopic, topicString.c_str());

  String topic = "tong" + WiFi.macAddress();
  char Topic1[topicString.length() + 1];
  strcpy(Topic1, topic.c_str());

  // 建立发布信息。信息内容以Hello World为起始，后面添加发布次数。
  String messageString = String(svalue);
  char publishMsg[messageString.length() + 1];
  strcpy(publishMsg, messageString.c_str());

  // 实现ESP8266向主题发布信息
  if (svalue >= 45)
  {

    digitalWrite(D0, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, HIGH);
    digitalWrite(D6, HIGH);
    digitalWrite(D8, HIGH);
    u8g2.clearDisplay();
    u8g2.setFont(u8g2_font_unifont_t_chinese3);
    u8g2.firstPage();
    do
    {

      u8g2.setCursor(0, 15);
      u8g2.print("请注意!!!!");
      u8g2.setCursor(0, 35);
      u8g2.print("水位达到一级警告");
      u8g2.setCursor(0, 60);
      u8g2.print("当前水位:");
      u8g2.setCursor(80, 60);
      u8g2.print(String(svalue) + "ml");

    } while (u8g2.nextPage());

    String m = "危险!水位已达一级警告!";
    char Msg[m.length() + 1];
    strcpy(Msg, m.c_str());
    if (mqttClient.publish(Topic1, Msg))
    {
      Serial.println("Publish Topic:");
      Serial.println(publishTopic);
      Serial.println("Publish message:");
      Serial.println(publishMsg);
    }
    else
    {
      Serial.println("Message Publish Failed.");
    }
  }
  else if (svalue >= 20)
  {
    //水位达到二级警告
    digitalWrite(D0, LOW);
    digitalWrite(D1, HIGH);
    digitalWrite(D2, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D8, HIGH);
    u8g2.clearDisplay();
    u8g2.setFont(u8g2_font_unifont_t_chinese3);
    u8g2.firstPage();
    do
    {

      u8g2.setCursor(0, 15);
      u8g2.print("请注意!");
      u8g2.setCursor(0, 35);
      u8g2.print("水位达到二级警告");
      u8g2.setCursor(0, 60);
      u8g2.print("当前水位:");
      u8g2.setCursor(80, 60);
      u8g2.print(String(svalue) + "ml");

    } while (u8g2.nextPage());

    String m = "注意!水位已达二级警告!";
    char Msg[m.length() + 1];
    strcpy(Msg, m.c_str());
    if (mqttClient.publish(Topic1, Msg))
    {
      Serial.println("Publish Topic:");
      Serial.println(publishTopic);
      Serial.println("Publish message:");
      Serial.println(publishMsg);
    }
    else
    {
      Serial.println("Message Publish Failed.");
    }
  }
  else if (svalue < 20)
  {
    //水位正常!
    digitalWrite(D0, HIGH);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D8, LOW);
    u8g2.clearDisplay();
    u8g2.setFont(u8g2_font_unifont_t_chinese3);
    u8g2.firstPage();
    do
    {

      u8g2.setCursor(0, 15);
      u8g2.print("水位正常!");
      u8g2.setCursor(0, 35);
      u8g2.print("请放心出行");
      u8g2.setCursor(0, 60);
      u8g2.print("当前水位:");
      u8g2.setCursor(80, 60);
      u8g2.print(String(svalue) + "ml");

    } while (u8g2.nextPage());

    String m = "水位正常!可以放心出行";
    char Msg[m.length() + 1];
    strcpy(Msg, m.c_str());
    if (mqttClient.publish(Topic1, Msg))
    {
      Serial.println("Publish Topic:");
      Serial.println(publishTopic);
      Serial.println("Publish message:");
      Serial.println(publishMsg);
    }
    else
    {
      Serial.println("Message Publish Failed.");
    }
  }

  if (mqttClient.publish(publishTopic, publishMsg))
  {
    Serial.println("Publish Topic:");
    Serial.println(publishTopic);
    Serial.println("Publish message:");
    Serial.println(publishMsg);
  }
  else
  {
    Serial.println("Message Publish Failed.");
  }
}

// ESP8266连接wifi
void connectWifi()
{
  u8g2.setFont(u8g2_font_unifont_t_chinese3);
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(0, 35);
    u8g2.print("城市水深报警系统");
  } while (u8g2.nextPage());
  delay(2000);
  u8g2.clearDisplay();
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_unifont_t_chinese2);
    u8g2.drawStr(10, 25, "program start");

  } while (u8g2.nextPage());

  WiFi.begin(ssid, password);

  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");
  Serial.println("");
  delay(2000);
  u8g2.clearDisplay();
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 24, "WiFi Connected");
    u8g2.drawStr(14, 40, "Success!!!!");

  } while (u8g2.nextPage());
  delay(1000);
}
