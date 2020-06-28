#include "UDmega.h"

const uint8_t keyNums = 0x00;//ロボットのID
const uint8_t keys = 0x01;
const uint8_t moves = 0x01;
const uint8_t action = 0x02;
const uint8_t gos = 0x01;
const uint8_t back = 0x02;
const uint8_t left = 0x03;
const uint8_t right = 0x04;
const uint8_t stops = (uint8_t)0x00;
const uint8_t triger = 0x01;
const uint8_t sw0 = 0x02;

boolean programMode;
int chars;
char sent[64];

struct information{
  uint8_t keyNum;
  uint8_t type;
  uint8_t commond;
  uint8_t sum;
};
struct information info;
struct information trans;

UDmega udmega(keyNums);

void writeSerialUD(uint8_t k,uint8_t t,uint8_t c,uint8_t l){
  Serial3.write(k);
  Serial3.flush();
  Serial3.write(t);
  Serial3.flush();
  Serial3.write(c);
  Serial3.flush();
  Serial3.write(k+t+c);
  Serial3.flush();
  Serial3.write(l);
  Serial3.flush();
}

void readSerialUD(struct information *data){
  data->keyNum = Serial3.read();
  delay(WAIT);
  data->type = Serial3.read();
  delay(WAIT);
  data->commond = Serial3.read();
  delay(WAIT);
  data->sum = Serial3.read();
  delay(WAIT);
  Serial3.end();
  delay(WAIT);
  Serial3.begin(115200);
}

//コントローラの指示からモーションを再生させる関数
void motionPlay(uint8_t ty, uint8_t com){
  Serial3.end();
  switch(ty){
    case (uint8_t)0x00:
    switch(com){
      case (uint8_t)0x00:
        Serial.println("re conection");
        Serial3.begin(115200);
        writeSerialUD(keyNums,(uint8_t)0x00,(uint8_t)0x00,(uint8_t)0x0a);
        Serial3.end();
        break;

      default:
        break;
    }
    Serial.println("finish");
    break;
    
    case moves:
    switch (com) {
      case gos:
        Serial.println("go");
        udmega.moveServo(0);
        break;
      case back:
        Serial.println("back");
        udmega.moveServo(1);
        break;
      case left:
        Serial.println("turn left");
        udmega.moveServo(2);
        break;
      case right:
        Serial.println("turn right");
        udmega.moveServo(3);
        break;
      case stops:
        Serial.println("homeposition");
        udmega.homePosition(0,0,0,false);
        delay(100);
        break;
      default:
        break;
    }
    break; 

    case action:
    switch (com) {
      case triger:
        Serial.println("triger action");
        udmega.moveServo(4);
        break;
      case sw0:
        Serial.println("sw0 action");
        udmega.moveServo(5);
        break;
    }
    break; 

    default:
      break;
  }
  Serial3.begin(115200);
}

void setup() {
  programMode = udmega.udBegin();  
  if(programMode == false){
    boolean LEDSignal;
    while(info.keyNum != keys){
      while(Serial3.available() <= 4){
        digitalWrite(MODELED,LEDSignal);
        LEDSignal = !LEDSignal;
        Serial.println("wait authentication");
        delay(300);
      }
      readSerialUD(&info);
      int kn = info.keyNum;
      Serial.print("ID:");
      Serial.println(kn);
    }
    writeSerialUD(keys,(uint8_t)0x00,(uint8_t)0x00,(uint8_t)0x0a);
  }
}

void loop() {
  if(programMode == true)
  {
    digitalWrite(MODELED,HIGH);
    if(Serial.available())
    {
      char stay = Serial.read();
      delay(WAIT);
      if(stay == '\n' || chars > 1000)
      {
        udmega.taskGUI(sent);
        chars=0;
        udmega.syokiChar(sent);
        Serial.println("@next");//この文は必須
      }else
      {
        sent[chars] = stay;
        chars++;
      }
    }
    else{
      udmega.allPostion();
    }
  }
  else
  {
    digitalWrite(MODELED,LOW);
    if(Serial3.available() > 4){
      readSerialUD(&info);
      if(info.keyNum == keys && info.sum == info.keyNum + info.type + info.commond){
        motionPlay(info.type,info.commond);
      }
    }
  }
}
