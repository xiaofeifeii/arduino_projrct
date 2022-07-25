//控制电机，显示水位，水位曲线
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT //支持小爱同学
#define BLINKER_MIOT_SENSOR   //定义为语音控制传感器设备 

#include <Blinker.h>
#include <Stepper.h>
#include <Q2HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h> //ds18b20库

#include <DHT.h>
DHT dht(2, DHT11);
int32_t humi_read = 0;//湿度 小爱同学只能查整数的数据 
float temp_read = 0; //温度
 

//#define Onewire_bus 2 // ds18b20连接2引脚D4
//OneWire oneWire(Onewire_bus);
//DallasTemperature sensors(&oneWire);
/*
 *压力初始化
 */
const byte hx711_data_pin = 12;  // D5
const byte hx711_clock_pin = 14; // D6
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);
#define waterPIN A0 //水位传感器的引脚

#define BUTTON_1 "food"
bool LED1_Flag = false;
bool LED2_Flag = false;
bool LED0_Flag = false;
bool auto_Flag = true;
int valwater;                 //读取水位数据
int kg = 0;                   //压力
int val;                      //人体红外
int tempe;                    //水温
char auth[] = "ab1e3f1215ab"; // caiyq52-改成自己在点灯app中生成的key码
char ssid[] = "Mi 10";        // caiyq52-改成自己的WIFI名称
char pswd[] = "582975413";    // caiyq52-改成自己的WIFI密码

BlinkerNumber water("waterVolume");
BlinkerNumber food("foodVolume");
BlinkerNumber infrared("infrared"); //红外
BlinkerButton Button1("food");      //步进电机的按钮
BlinkerButton Button2("water");      //继电器的按钮
BlinkerButton Button3("auto");      //自动切换的按钮
BlinkerNumber Temp("temp");         //水温


const int stepsPerRevolution = 200;
Stepper myStepper(stepsPerRevolution, 16, 4, 5, 0); // D0 D1 D2 D3

void heartbeat() //向APP发送数据，这个是心跳包
{
  water.print(valwater);
  food.print(kg);
  infrared.print(val);
  Temp.print(tempe);
}

void dataStorage()
{
  String str;
  Blinker.dataStorage("waterVolume", valwater);
  Blinker.dataStorage("foodVolume", kg);
  Blinker.dataStorage("infrared", val);
  Blinker.dataStorage("temp", tempe);
}

void button1_callback(const String &state)
{
  Blinker.vibrate();
  Serial.println(state);
  if (state == "on")
  {
    myStepper.step(1024);
    Button1.print("on");
  }
  else if (state == "off")
  {

    Button1.print("off");
   // myStepper.step(-1024);
  }
}

void button2_callback(const String &state)
{
   Blinker.vibrate();
  Serial.println(state);
  if (state == "on")
  {
  digitalWrite(D8, LOW);
  delay(2000);
    Button2.print("on");
  }
  else if (state == "off")
  {

    Button2.print("off");
  digitalWrite(D8, HIGH);
  }
}

void button3_callback(const String &state)
{
   if (state == "on")
  {
    auto_Flag =true;
    Button3.print("on");
  }
  else if (state == "off")
  {

    auto_Flag =false;
 Button3.print("off");

  }
}

void miotMode(uint8_t mode)
{
    BLINKER_LOG("need set mode: ", mode);

    if (mode == BLINKER_CMD_MIOT_DAY) {
        LED0_Flag = !LED0_Flag;
        if(LED0_Flag)
        {
            Button1.print("on");
            myStepper.step(-1024);
        }
        else
        {
            Button1.print("off");
            //digitalWrite(LED0, HIGH);
        }
    }
    else if (mode == BLINKER_CMD_MIOT_NIGHT) {
        LED1_Flag = !LED1_Flag;
        if(LED1_Flag)
        {
            Button1.print("on");
            
            digitalWrite(D8, LOW);
            delay(2000);
        }
        else
        {
            Button2.print("off");
           // digitalWrite(D8, HIGH);
        }
    }
   
    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
}

void miotQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes: ", queryCode);

  switch (queryCode)
    {
        //同时查询传感器数据
        case BLINKER_CMD_QUERY_ALL_NUMBER :
            BLINKER_LOG("MIOT Query All");
            BlinkerMIOT.temp(temp_read);
            BlinkerMIOT.humi(humi_read);
            BlinkerMIOT.pm25(20);
            BlinkerMIOT.co2(20);
            BlinkerMIOT.print();
            break;
        //查询pm2.5
        case BLINKER_CMD_QUERY_PM25_NUMBER :
            BLINKER_LOG("MIOT Query PM25");
            BlinkerMIOT.pm25(20);
            BlinkerMIOT.print();
            break;
        //查询湿度
        case BLINKER_CMD_QUERY_HUMI_NUMBER :
            BLINKER_LOG("MIOT Query HUMI");
            BlinkerMIOT.humi(humi_read);
            BlinkerMIOT.print();
            break;
        //查询温度
        case BLINKER_CMD_QUERY_TEMP_NUMBER :
            BLINKER_LOG("MIOT Query TEMP");
            BlinkerMIOT.temp(temp_read);
            BlinkerMIOT.print();
            break;
        //查询CO2
        case BLINKER_CMD_QUERY_TIME_NUMBER :
            BLINKER_LOG("MIOT Query co2");
            BlinkerMIOT.co2(20);
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.temp(20);
            BlinkerMIOT.humi(20);
            BlinkerMIOT.pm25(20);
            BlinkerMIOT.co2(20);
            BlinkerMIOT.print();
            break;
    }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D7, INPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(D8, OUTPUT);
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);
  BLINKER_DEBUG.debugAll();
  Blinker.begin(auth, ssid, pswd);
  Button1.attach(button1_callback);
  Button2.attach(button2_callback);
  Button3.attach(button3_callback);
  BlinkerMIOT.attachMode(miotMode);  //多路
   
  BlinkerMIOT.attachQuery(miotQuery); //小爱同学 数据反馈
 
  Blinker.attachHeartbeat(heartbeat);
  Blinker.attachDataStorage(dataStorage);
  

  myStepper.setSpeed(120);
  Blinker.attachHeartbeat(heartbeat);
   dht.begin();
}

void loop()
{
  Blinker.run();
  valwater = analogRead(waterPIN);
   float h = dht.readHumidity();
   float t = dht.readTemperature();
  kg = (hx711.read() / 100.0) - 81277;
 // val = digitalRead(D7);          //红外检测模块
  val=1;
  //sensors.requestTemperatures(); //获取温度
  //tempe = sensors.getTempCByIndex(0);
   if (isnan(h) || isnan(t))
    {
        BLINKER_LOG("Failed to read from DHT sensor!");
    }
    else
    {
       
        humi_read = h;
        temp_read = t;
        tempe = t;
    } 
  Blinker.delay(1000);
  if(auto_Flag){//是否自动
     BLINKER_LOG("Humidity: ", h, " %");
     BLINKER_LOG("Temperature: ", t, " *C");
  if (val > 0)//检测到宠物
  {
    if (valwater < 40)
    {
      //打开水泵、加热（led）
      digitalWrite(D8,LOW);
    }else{
      digitalWrite(D8,HIGH);
    }
    if ((kg <=10))
    {
      //启动步进电机
      while(kg <500){//加到500停止
      myStepper.step(1024);
      kg = (hx711.read() / 100.0) - 81277;
      }
    }
  }
  }
}


