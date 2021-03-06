                                   //SMART IRRIGATOR FOR TWO WATER CIRCUITS
//PROGRAMMING STARTED ON 1/2/2021
//USES ARDUINO UNO WITH HC-06 AND DHT 22 SENSOR
//USER TICKER LIBRARY FOR THREADING
//LOGIC FOR RELAY BOARD CHANGED TO ACTIVE LOW

//CHANGES ON 11/4/2021
//CONTER CYCLE TIME CHANGED TO MINUTES
//THE CYCLE TIME TO BE ENTERED IN DAYS
//REMOVED THE SERIAL OUT PROGRAM WHICH WAS FOR TESTING ONLY
//CHANGED THE PUMP -OUTPUT LOGIC TO HIGH ACTIVE
//CHANGED ALL PUMP WAITING VRAIBLES TO LONG
//ONBOARD LED 13 ADDED FOR SUPERVISORY CIRCUIT
//CHANGES ON 9/5/2021
//MODIFIED THE BT SERIAL SIGNAL TO GET 11 BYTES.
//WORKING OK
//CHANGED THE CABINET TEMPERATURE SEQUENCE TO RUN 1 MIN EVERY 60 MINS OR WHEN TEMP IS HIGH


//LIBRARY ICLUSIONS
#include<EEPROM.h>
#include<SD.h>
#include <SoftwareSerial.h>
#include "DHT.h"
#include <Ticker.h>

//Pin Definition
#define DHTTYPE DHT22
#define Supp_LED 2
#define BT_Read_Write 3
#define Pump1_OP 4
#define Pump2_OP 5
#define Tank_Level_OK 6
#define Tank_Level_Sw 7
#define Tank_Level_LO 8
#define BT_RX 9
#define BT_TX 10
#define DHT22_Pin 11
#define Cab_Fan 12
#define OnBoard_LED 13

//SUB ROUTINE DEFINITIONS
void Super_LED();
void Time_Check();
void Pump_Out();
void Pump1_Off();
void Pump2_Off();
void Temp_Hum_Read();
void BT_Send();
void BT_Read();
void Pump1_Off_Man();
void Pump2_Off_Man();
void Check_Level();
void Read_Control_Cab_Temp();

//EEPROM Values
byte Water_Level_Ok;
long Pump1_CycleTime;
long Pump1_WaterTime;
long Pump2_CycleTime;
long Pump2_WaterTime;
byte Pump1_Water_Time_Man;
byte Pump2_Water_Time_Man;

//FOR BT READ
long BT_Read_In1;
long BT_Read_In2;
long BT_Read_In3;
long BT_Read_In4;
long BT_Read_In5;
long Previous_BT_Data;
long Present_BT_Data;
int BT_Data1;
int BT_Data2;
int BT_Data3;
int BT_Data4;
int BT_Data5;
int BT_Data6;
int BT_Data7;
int BT_Data8;
int BT_Data9;
int BT_Data10;
int BT_Data11;

byte BT_Counter;

//VARIABLES FOR COUNTERS PUMP AND LEVELS
long Counter1, Counter2;
bool Pump1_On, Pump2_On;
long LED_Counter;
byte Level_Ok;
int Temp_C;
int Hum;
float Cabinet_Temp;
int Cab_Temp;
byte Cabinet_Count;

//CLASS DEFINITION FOR SOFTWARE SERIAL AND DTH22
SoftwareSerial BT(BT_TX,BT_RX); 
DHT Temp_Hum(DHT22_Pin, DHTTYPE);

//ENABLING TICKER SUB ROUTINES
Ticker SUPLED(Super_LED, 1000); 
Ticker COUNTER_MAIN(Time_Check, 60000);
Ticker PUMP_CONTROL(Pump_Out, 500);
Ticker PUMP1_OFF(Pump1_Off,Pump1_WaterTime*1000);
Ticker PUMP2_OFF(Pump2_Off, Pump2_WaterTime*1000);
Ticker TEMP_HUM(Temp_Hum_Read, 3000);
Ticker BT_SEND(BT_Send, 1000);
Ticker BT_GET(BT_Read,2500);
Ticker PUMP1_OFF_MAN(Pump1_Off_Man,Pump1_Water_Time_Man*1000);
Ticker PUMP2_OFF_MAN(Pump2_Off_Man,Pump2_Water_Time_Man*1000);
Ticker TANK_LEVEL(Check_Level,4000);
Ticker CABINET_T(Read_Control_Cab_Temp,60000);

//SETUP LOOP
void setup() 
{
 //BEGIN THE DTH 22
 Temp_Hum.begin();
 
 //PIN MODE SETTING
 pinMode(Pump1_OP, OUTPUT); 
 pinMode(Pump2_OP, OUTPUT);
 pinMode(Tank_Level_Sw, INPUT);
 pinMode(Supp_LED, OUTPUT);
 pinMode(Tank_Level_OK, OUTPUT);
 pinMode(Tank_Level_LO, OUTPUT);
 pinMode(BT_Read_Write,OUTPUT);
 pinMode(OnBoard_LED,OUTPUT);
 digitalWrite(Pump1_OP, LOW);
 digitalWrite(Pump2_OP, LOW);
 pinMode(Cab_Fan,OUTPUT

 );
 
 //READING EEPROM VALUE
 Pump1_CycleTime = EEPROM.read(1);
 Pump1_WaterTime = EEPROM.read(2);
 Pump2_CycleTime = EEPROM.read(3);
 Pump2_WaterTime = EEPROM.read(4);



//INITILISING THE SOFTWARE SERIAL CLASSES
BT.begin(9600);

//START THE SERIAL OUTPUT FOR PC
Serial.begin(9600);

//START TICHER TIMERS
SUPLED.start();
COUNTER_MAIN.start();
PUMP_CONTROL.start();
TEMP_HUM.start();
BT_SEND.start();
BT_GET.start();
TANK_LEVEL.start();
CABINET_T.start();
}


//MAIN LOOP
void loop() 
{
//UPDATE ALL THE TICKER LOOPS
SUPLED.update(); 
COUNTER_MAIN.update();
PUMP_CONTROL.update();
PUMP1_OFF.update();
PUMP2_OFF.update();
TEMP_HUM.update();
BT_SEND.update();
BT_GET.update();
PUMP1_OFF_MAN.update();
PUMP2_OFF_MAN.update();
TANK_LEVEL.update();
CABINET_T.update();


}

//SUPERVISORY LED BLINKER
void Super_LED()
{
if(digitalRead(Supp_LED) == 1)
   {
    digitalWrite(Supp_LED,LOW);
     }
 else
   {
    digitalWrite(Supp_LED,HIGH);
     }

//ONBOARD LED
if(digitalRead(OnBoard_LED) == 1)
     {
    digitalWrite(OnBoard_LED,LOW);
     }
 else
     {
    digitalWrite(OnBoard_LED,HIGH);
     }
}

//THE SENSOR INPUT IS HIGH ON LEVEL OK AND LOW ON LOW
void Check_Level()
{
  Water_Level_Ok = digitalRead(Tank_Level_Sw);
  if (Water_Level_Ok ==1)
     {
      Level_Ok =1;
      digitalWrite(Tank_Level_OK,HIGH);
      digitalWrite(Tank_Level_LO,LOW);
     }
  else
  {
      Level_Ok =0;
      digitalWrite(Tank_Level_OK,LOW);
      digitalWrite(Tank_Level_LO,HIGH);
  }
}

//START AND RUN THE ONE SECOND COUNTER 
void Time_Check()
{
  Counter1=Counter1+1;
  Counter2=Counter2+1;
  if(Counter1>=345600)
    {
      Counter1=0;
    }
  if(Counter2>=345600)
    {
      Counter2=0;
    }
  if(Counter1>=(Pump1_CycleTime*1440))
    {
      Pump1_On = true;
      Counter1=0;
    }
  if(Counter2>=(Pump2_CycleTime*1440))
    {
      Pump2_On = true;
      Counter2=0;
    }
  if(Counter1>=(Pump1_CycleTime*1440) && Counter2>=(Pump2_CycleTime*1440))
    {
     Pump1_On = true;
     Pump2_On = true;
     Counter1=0;
     Counter2=0;
    }
}

//PUMP OUTPUT
void Pump_Out()
{
  
  if (Pump1_On == true)
     {
      if(Water_Level_Ok==1)
      {digitalWrite(Pump1_OP, HIGH);}
         
      Pump1_On = false;
      PUMP1_OFF.start();
       }
  if (Pump2_On == true)
     {
      if(Water_Level_Ok==1)
      {digitalWrite(Pump2_OP, HIGH);}
            
      Pump2_On = false;
      PUMP2_OFF.start();
      }
}

//SWITCH OFF PUMPS AFTER TIMING
void Pump1_Off()
{
digitalWrite(Pump1_OP, LOW);
PUMP1_OFF.stop();  
}

//SWITCH OFF PUMP-2 AFTER TIMING
void Pump2_Off()
{
digitalWrite(Pump2_OP, LOW);
PUMP2_OFF.stop();    
}


//SEND OUT BLUETOOTH SIGNALS FOR ANDROID DEVICE
void BT_Send()
{
  BT.print(Temp_C);
  BT.print(",");
  BT.print(Hum);
  BT.print(",");
  BT.print(Counter1);
  BT.print(",");
  BT.print(Counter2);
  BT.print(",");
  BT.print(Pump1_WaterTime); 
  BT.print(",");
  BT.print(Pump2_WaterTime);
  BT.print(",");
  BT.print(Pump1_CycleTime); 
  BT.print(",");
  BT.print(Pump2_CycleTime);
  BT.print(",");
  BT.print(digitalRead(Tank_Level_OK));
  BT.print(",");
  BT.print(digitalRead(4));
  BT.print(",");
  BT.print(digitalRead(5));
  BT.print(",");
  BT.print(Cab_Temp);
}

//TEMPERATURE AND HUMIDITY FROM DTH22
void Temp_Hum_Read()
{
  Temp_C = Temp_Hum.readTemperature();
  Hum = Temp_Hum.readHumidity();
}

//READ FROM BT DEVICE FOR SETTING AND MANUAL OPERATIONS
void BT_Read()
{
Present_BT_Data=BT.available();
//READ AND FLUSH THE INPUT BUFFER
    while(BT.available() > 0) 
      {
    BT_Data1 = BT.read();
    BT_Data2 = BT.read();
    BT_Data3 = BT.read();
    BT_Data4 = BT.read();
    BT_Data5 = BT.read();
    BT_Data6 = BT.read();
    BT_Data7 = BT.read();
    BT_Data8 = BT.read();
    BT_Data9 = BT.read();
    BT_Data10 = BT.read();
    BT_Data11= BT.read();
   
      }
    
if (Present_BT_Data-Previous_BT_Data==11)
    {
    digitalWrite(BT_Read_Write,1);
    BT_Read_In1=(BT_Data1-48);
    BT_Read_In2=(BT_Data2-48)*100+(BT_Data3-48)*10+(BT_Data4-48);
    BT_Read_In3=(BT_Data5-48)*10+(BT_Data6-48);
    BT_Read_In4=(BT_Data7-48)*100+(BT_Data8-48)*10+(BT_Data9-48);
    BT_Read_In5=(BT_Data10-48)*10+(BT_Data11-48);
       
    }
//FOR DTA SETTING SEQUENCE
if(BT_Read_In1==1)
       {
      Pump1_CycleTime= BT_Read_In2;
      Pump1_WaterTime= BT_Read_In3;
      Pump2_CycleTime= BT_Read_In4;
      Pump2_WaterTime= BT_Read_In5;
      //WRITE EEPROM 
    EEPROM.write(1,Pump1_CycleTime);
    EEPROM.write(2,Pump1_WaterTime);
    EEPROM.write(3,Pump2_CycleTime);
    EEPROM.write(4, Pump2_WaterTime);
    PUMP1_OFF.interval(Pump1_WaterTime*1000);
    PUMP2_OFF.interval(Pump2_WaterTime*1000);
    Counter1=0;
    Counter2=0; 
       }
//FOR MANUAL CONTROLS
if(BT_Read_In1==2)
       {
       PUMP1_OFF_MAN.interval(BT_Read_In3*1000);
       PUMP1_OFF_MAN.start();
       if(Water_Level_Ok==1)
       {digitalWrite(Pump1_OP, HIGH);}
              
       PUMP2_OFF_MAN.interval(BT_Read_In5*1000);
       PUMP2_OFF_MAN.start();
       if(Water_Level_Ok==1)
       {digitalWrite(Pump2_OP, HIGH);}
        }
BT_Read_In1=0;
digitalWrite(BT_Read_Write,LOW);
Previous_BT_Data=Present_BT_Data;
Present_BT_Data=0;
 }

//MANUAL PUMP-1 OFF CONTROL
void Pump1_Off_Man()
{
digitalWrite(Pump1_OP, LOW);
PUMP1_OFF_MAN.stop();
}


//MANUAL PUMP-2 OFF CONTROL
void Pump2_Off_Man()
{
digitalWrite(Pump2_OP, LOW);
PUMP2_OFF_MAN.stop();
}

//CABINET FAN CONTROL
void Read_Control_Cab_Temp()
{
Cabinet_Count=Cabinet_Count+1;
if (Cabinet_Count>=254){Cabinet_Count=0;}

//COUNTER BASED CABINET COOLING FAN RUN
if (Cabinet_Count==60)
{
digitalWrite(Cab_Fan,HIGH);
}

if (Cabinet_Count==61)
{
digitalWrite(Cab_Fan,LOW);
Cabinet_Count=0;
}


//TEMPERATURE CONTROLLED CABINET TEMPERATURE
Cabinet_Temp=(analogRead(A0)/1023.0)*500;
Cab_Temp=Cabinet_Temp;
if (Cabinet_Temp>=39.0)
 {
  digitalWrite(Cab_Fan,HIGH);
 }

if (Cabinet_Temp<=25.0)
  {
  digitalWrite(Cab_Fan,LOW);
  }

}
