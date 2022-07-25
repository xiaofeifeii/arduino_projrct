#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h> //ds18b20库温度传感器

#define Onewire_bus 14 // ds18b20连接14引脚
#define button 17      // A3引脚 启动按钮
#define button2 7      // 7引脚  舵机按钮

OneWire oneWire(Onewire_bus);
DallasTemperature sensors(&oneWire);

Servo myservo;
LiquidCrystal_I2C lcd(0x27, 16, 2); //配置LCD地址及行列

unsigned long s = 0;
int Miao = 0;
int Fen = 0;
int sign = 0;
int Before = 0;
int state = 1;
void setup()    
{
  Serial.begin(9600);
  pinMode(A3, INPUT);
  // digitalWrite(A3, HIGH);
  // pinMode(A2, OUTPUT);
  pinMode(2, OUTPUT);
  myservo.attach(A2, 544, 2700);//初始化舵机
  myservo.write(0);
  lcd.init();      //初始化LCD
  lcd.backlight(); //打开背光

  lcd.setCursor(0, 0);  //列 行
  lcd.print("Welcome to the  ");
  lcd.setCursor(0, 1);
  lcd.print("smart pill box!");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("                ");
}

void loop()
{

  lcd.setCursor(0, 0);
  lcd.print("Ready start ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  int Bstate = digitalRead(button);

  // Serial.println(Bstate);
  if (digitalRead(button) == HIGH)
  {
    delay(500);
    if (digitalRead(button) == HIGH)
    {
      lcd.setCursor(0, 0);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      while (1)  
      {
      
        int minuteMax = 1;
        if (Fen < minuteMax) //等待延时
        {
          /* code */
          if (Miao > 59)
          {
            Miao = 0;
            Fen++;
            lcd.setCursor(0, 1);
            lcd.print("");
            if (Fen > 59)
            {
              Fen = 0;
            }
          }
          else
          {
            delay(10);
            Miao++;
          }

          Serial.print(Fen);
          Serial.print(":");
          Serial.println(Miao);
          //计时中、添加显示时间、加热中···
          static int value, svalue;
          value = analogRead(15);
          svalue = 100 - map(value, 0, 1023, 100, 0);
          if (svalue < 30) //判液位是否达到阈值
          {

            speech3(); //语音提示液位不够
            delay(1000);
          }

          lcd.setCursor(0, 0); // 将LCD光标移动到第一位0，0(列，行)
          lcd.print("temp:");
          sensors.requestTemperatures();         //读取温度
          lcd.print(sensors.getTempCByIndex(0)); // 传感器温度值
          lcd.print(" C");

          lcd.setCursor(0, 1);
          lcd.print(Fen);
          lcd.print(":");
          if (Miao < 10)
          {
            lcd.print("0");
            lcd.print(Miao);
          }
          else
          {
            lcd.print(Miao);
          }
          lcd.print("     waiting");
        }
        else
        {
          //时间到,
          Fen  = 0;
          Miao = 0;
          digitalWrite(2, HIGH);  //启动加热

          while (1)
          {
           startTime:
            int minuteMax = 1;
            if (Fen < minuteMax)
            {
              /* code */
              if (Miao > 59)
              {
                Miao = 0;
                Fen++;
                lcd.setCursor(0, 1);
                lcd.print("");
                if (Fen > 59)
                {
                  Fen = 0;
                }
              }
              else
              {
                delay(10);
                Miao++;
              }

              Serial.print(Fen);
              Serial.print(":");
              Serial.println(Miao);
              //计时中、添加显示时间、加热中···
              static int value, svalue;
              value = analogRead(15);
              svalue = 100 - map(value, 0, 1023, 100, 0);
              if (svalue < 30) //判液位是否达到阈值
              {

                speech3(); //语音提示液位不够
              }

              lcd.setCursor(0, 0); // 将LCD光标移动到第一位0，0(列，行)
              lcd.print("temp:");
              sensors.requestTemperatures();         //读取温度
              lcd.print(sensors.getTempCByIndex(0)); // 传感器温度值
              lcd.print(" C");

              lcd.setCursor(0, 1);
              lcd.print(Fen);
              lcd.print(":");
              if (Miao < 10)
              {
                lcd.print("0");
                lcd.print(Miao);
              }
              else
              {
                lcd.print(Miao);
              }
              lcd.print("     Heating");
            }else{
            Fen = 0;
            Miao = 0;
            break;
            }

          }
          // lcd.setCursor(0, 1);
          // lcd.print("                ");
          // lcd.print("Time is up!!    ");
          //读取液位
          static int value, svalue;
          value = analogRead(15);
          svalue = 100 - map(value, 0, 1023, 100, 0);
          Serial.println(svalue);
          if (svalue > 30) //判液位是否达到阈值
          {
            sensors.requestTemperatures(); //读取温度
            float templ = sensors.getTempCByIndex(0);
            lcd.setCursor(0, 0); // 将LCD光标移动到第一位0，0(列，行)
            lcd.print("temp:");
            lcd.print(sensors.getTempCByIndex(0)); // 传感器温度值
            lcd.print(" C");
            lcd.setCursor(0, 1); // 将LCD光标移动到第一位0，0(列，行)
            lcd.print("                ");
            lcd.print("Liquid level:");
            lcd.print(svalue);

            if (templ < 30)
            {
             
              speech2(); //语音提示水温不够，请稍等
              digitalWrite(2, HIGH);   //开始加热
              goto startTime;
            }
            else
            {
              speech1(); //语言提示水温合适

              digitalWrite(2, LOW); //关闭加热
              //myservo.write(90);    //舵机打开
              lcd.setCursor(0, 1);  // 将LCD光标移动到第一位0，0(列，行)
                                    // lcd.print("                ");
              lcd.print("Heating succeed!!");

              delay(3000);
              break;
            }
          }
          else
          {
            speech3(); //语音提示液位不够
          }
          break;
        }
      }
      state = !state;
      Serial.println("start");
      Before = !Before;
    }
    while (digitalRead(button) == HIGH);
  }
  else if (Bstate == 0 && Before == 1)
  {
    Before = !Before;
   // myservo.write(0); //舵机默认关闭
  }

    if (digitalRead(button2) == HIGH)
  {
    delay(500);
    if (digitalRead(button2) == HIGH)
    {
       myservo.write(90); 
       delay(2000);
    }
    while (digitalRead(button2) == HIGH);
  }else{
     myservo.write(0); //舵机默认关闭
  }
}

void speech1()
{ //语音：水温合适请及时服用
  unsigned char i = 0;
  unsigned char head[26];

  head[0] = 0xFD;
  head[1] = 0x00;
  head[2] = 0x17;
  head[3] = 0x01;
  head[4] = 0x00;
  head[5] = 0xCB;
  head[6] = 0xAE;
  head[7] = 0xCE;
  head[8] = 0xC2;
  head[9] = 0xBA;
  head[10] = 0xCF;
  head[11] = 0xCA;
  head[12] = 0xCA;
  head[13] = 0xA3;
  head[14] = 0xAC;
  head[15] = 0xC7;
  head[16] = 0xEB;
  head[17] = 0xBC;
  head[18] = 0xB0;
  head[19] = 0xCA;
  head[20] = 0xB1;
  head[21] = 0xB7;
  head[22] = 0xFE;
  head[23] = 0xD3;
  head[24] = 0xC3;
  head[25] = 0xFA;

  for (i = 0; i < 26; i++)
  {
    Serial.write(head[i]);
  }
}

void speech2()
{ //水温不够，请稍等
  unsigned char i = 0;
  unsigned char head[22];

  head[0] = 0xFD;
  head[1] = 0x00;
  head[2] = 0x13;
  head[3] = 0x01;
  head[4] = 0x00;
  head[5] = 0xCB;
  head[6] = 0xAE;
  head[7] = 0xCE;
  head[8] = 0xC2;
  head[9] = 0xB2;
  head[10] = 0xBB;
  head[11] = 0xB9;
  head[12] = 0xBB;
  head[13] = 0xA3;
  head[14] = 0xAC;
  head[15] = 0xC7;
  head[16] = 0xEB;
  head[17] = 0xC9;
  head[18] = 0xD4;
  head[19] = 0xB5;
  head[20] = 0xC8;
  head[21] = 0xCE;

  for (i = 0; i < 22; i++)
  {
    Serial.write(head[i]);
  }
}

void speech3()
{ //水位不够请加水
  unsigned char i = 0;
  unsigned char head[20];

  head[0] = 0xFD;
  head[1] = 0x00;
  head[2] = 0x11;
  head[3] = 0x01;
  head[4] = 0x00;
  head[5] = 0xCB;
  head[6] = 0xAE;
  head[7] = 0xCE;
  head[8] = 0xBB;
  head[9] = 0xB2;
  head[10] = 0xBB;
  head[11] = 0xB9;
  head[12] = 0xBB;
  head[13] = 0xC7;
  head[14] = 0xEB;
  head[15] = 0xBC;
  head[16] = 0xD3;
  head[17] = 0xCB;
  head[18] = 0xAE;
  head[19] = 0xD0;

  for (i = 0; i < 20; i++)
  {
    Serial.write(head[i]);
  }
}



