#include "Arduino.h"
#include "UDmega.h"

UDmega::UDmega(uint8_t num){
  keys = num;
}

//char型をリセット
void UDmega::syokiChar(char be[CHARSIZE]){
  char re[CHARSIZE] = {""};
  for(int j=0;j<CHARSIZE;j++){
    be[j] = re[j];
  }
}

//パソコンから指示を受けて仕事する関数
void UDmega::taskGUI(char serial[]){
  Serial.println("startTask");
  switch(svc.timing)
  {
    case 0:
    svc.com[0] = serial[0];
    break;
    case 1:
    if(svc.com[0] == 'w' || svc.com[0] == 'a')
    {
      svc.svPort = serial;
    }
    else
    {
      svc.svString = serial;
    }
    break;
    case 2:
    if(svc.com[0] == 'w')
    {
      svc.svPin = serial;
    }
    else
    {
      svc.svStep = serial;
    }
    break;
    case 3:
    if(svc.com[0] == 'w')
    {
      svc.svDeg = serial;
    }
    else
    {
      svc.motionNum = serial;
    }
    break;
    case 4:
    svc.sum = serial;
    break;
  }
  svc.timing++;
  if(serial[0] == 'f')
  {
    svc.timing=0;
    if(svc.com[0] == 'w')//指定されたサーボを動かす
    {
      int svport = svc.svPort.toInt();
      int svpin = svc.svPin.toInt();
      int svdeg = svc.svDeg.toInt();
      int sumAll = svc.sum.toInt();
      if(sumAll = svport+svpin+svdeg)
      {
        backApp[svport][svpin] = svdeg;
      }
    }
    else if(svc.com[0] == 'h')//指定されたサーボのホームポジションを書き換える
    {
      remakeHome();
    }
    else if(svc.com[0] == 'm')//指定されたモーションファイルを書き換える
    {
      //Serial.println("モーション書き込み");
      remakeMotion();
    }
    else if(svc.com[0] == 't')//指定されたモーションファイルを再生
    {
      int svport = svc.svPort.toInt();
      moveServo(svport);
    }
  }
}

//ホームポジションを書き換える関数
void UDmega::remakeHome(){
  char val[1000];
  if((svc.svStep.toInt())==0)
  {
    SD.remove("home.udh");
  }
  File homefile = SD.open("home.udh",FILE_WRITE);
  homefile.seek(homefile.size());
  homefile.println(svc.svString);
  delay(WAIT);
  homefile.close();
  Serial.println("Finish write to sdcard on homedata");
}
//モーションファイルの書き込み
void UDmega::remakeMotion(){
  if((svc.svStep.toInt())== 0)
  {
    SD.remove("mv" + String(svc.motionNum.toInt()) + ".udm");
  }
  String motion = {"mv" + String(svc.motionNum.toInt()) + ".udm"};
  File motionfile = SD.open(motion,FILE_WRITE);
  motionfile.seek(motionfile.size());
  if(portFile >= SVPORT){
    motionfile.println(svc.svString);
    portFile = 0;
  }
  else{
    motionfile.print(svc.svString);
    portFile++;
  }
  delay(WAIT);
  motionfile.close();
  Serial.println("Finish write to sdcard on motiondata");
}

//ホームポジションの角度を読んで動かす関数
void UDmega::homeRePosition(){
  homeFile = SD.open("home.udh",FILE_READ);
  if(homeFile == true)//ファイルがあったら
  {
    Serial.println("Moving to homepotsion");
    int maxSIZE = homeFile.size();
    String provisional;
    int mvFin=0,times=0,port=0,pin=0,how=0;
    char chr[CHARSIZE];
    char reads[100];

    while(times<maxSIZE){
      /*Serial.print("mvFin");
      Serial.println(mvFin);
      Serial.flush();
      Serial.print("maxSIZE");
      Serial.println(maxSIZE);
      Serial.flush();*/
      reads[times] = homeFile.read();
      if(reads[times] == ',')//区切り
      {
        provisional = chr;
        syokiChar(chr);
        homePos[port][pin] = provisional.toInt();
        /*Serial.print("pin=");
        Serial.println(pin);
        Serial.flush();*/
        how = 0;
        pin++;
      }
      else if(reads[times] == '\n')//改行
      {
        //provisional = chr;
        syokiChar(chr);
        //homePos[port][pin] = provisional.toInt();
        /*Serial.print("port=");
        Serial.println(port);
        Serial.flush();*/
        pin = 0;
        port++;
        how = 0;
      }
      else//読み途中ならば
      {
        chr[how] = reads[times];
        how++;
      }
      times++;
    }
    for(int i=0;i<SVPORT;i++)
    {
      for(int j=0;j<SVPIN;j++)
      {
        servo[i][j].write(homePos[i][j]);
        Serial.print(homePos[i][j]);
        //Serial.println(homePos[i][j]);
      }
    }
    delay(10);
    homeFile.close();
    Serial.println("close");
    Serial.flush();
  }
  else//ホームポジションのファイルがなかったら
  {
    Serial.println("###error to read homepotsion file###");
    for(int i=0;i<SVPORT;i++){
      for(int j=0;j<SVPIN;j++){
        homePos[i][j] = 90;
      }
    }
  }
}

//ホームポジションの角度を動かす関数
void UDmega::homePosition(int port,int pin,int deg,boolean juge){
  if(juge == true)//変数の変更があれば
  {
    homePos[port][pin] = deg;
  }
  Serial.println("###Moving to homepotsion###");
  for(int i=0;i<5;i++)
  {
    for(int j=0;j<6;j++)
    {
      servo[i][j].write(homePos[i][j]);
      backApp[i][j] = homePos[i][j];
    }
  }
  delay(10);
}

//サーボモータを動かす関数
void UDmega::moveServo(int motNum){
  String provisional;
  int mvFin=0,times=0,port=0,pin=0,how=0,filePos=0,svTimes=0,delayValue;
  char chr[CHARSIZE];
  char reads[100];
  int svPosArr[SVPORT][SVPIN];
  int resvPosArr[SVPORT][SVPIN];
  String motName = {"mv" + String(motNum) + ".udm"};
  File motion = SD.open(motName,FILE_READ);
  if(motion == true)//ファイルがあったら
  {
    Serial.print(motNum);
    Serial.println("MoveStart!");
    int maxSIZE = motion.size();

    while(times<maxSIZE){
      reads[times] = motion.read();
      delay(WAIT);
      if(reads[times] == ',')//区切り
      {
        provisional = chr;
        syokiChar(chr);
        svPosArr[port][pin] = provisional.toInt();
        pin++;
        how=0;
        //Serial.print("svPosArr[0][0]フェーズ4:");
        //Serial.println(svPosArr[0][0]);
      }
      else if(reads[times] == '|')//ポートチェンジ
      {
        provisional = chr;
        syokiChar(chr);
        svPosArr[port][pin] = provisional.toInt();
        pin=0;
        port++;
        how=0;
        //Serial.print("svPosArr[0][0]フェーズ5:");
        //Serial.println(svPosArr[0][0]);
      }
      else if(reads[times] == '\n')//改行
      {
        //時間読み込み
        provisional = chr;
        syokiChar(chr);
        delayValue = provisional.toInt();
        pin = 0;
        port= 0;
        how = 0;
        //サーボを実際動かす
        Serial.print("時間:");
        Serial.print(delayValue);
        Serial.print(" ");
        for(int i=0;i<SVPORT;i++)
        {
          for(int j=0;j<SVPIN;j++)
          {
            Serial.print(homePos[i][j] + svPosArr[i][j]);
            Serial.print(",");
            Serial.flush();
          }
          Serial.print("|"); 
        }
        Serial.println("");
        //Serial.print("svPosArr[0][0]フェーズ6:");
        //Serial.println(svPosArr[0][0]);
        if(svTimes == 0)//はじめだけ
        {
          for(int t=0;t<=delayValue;t=t+timeSet)
          {
            for(int i=0;i<SVPORT;i++)
            {
              for(int j=0;j<SVPIN;j++)
              {
                servo[i][j].write(homePos[i][j]+(svPosArr[i][j]*t/delayValue));
              }
            }
            //Serial.print("svPosArr[0][0]フェーズ7:");
            //Serial.println(svPosArr[0][0]);
            delay(timeSet);
          }
        }
        else//初回起動以外
        {
          for(int t=0;t<=delayValue;t=t+timeSet)
          {
            for(int i=0;i<SVPORT;i++)
            {
              for(int j=0;j<SVPIN;j++)
              {
                servo[i][j].write((homePos[i][j]+resvPosArr[i][j])+((svPosArr[i][j]-resvPosArr[i][j])*t/delayValue));
              }
            }
            //Serial.print("svPosArr[0][0]フェーズ8:");
            //Serial.println(svPosArr[0][0]);
            delay(timeSet);
          }
        }
        //Serial.print("svPosArr[0][0]フェーズ1:");
        //Serial.println(svPosArr[0][0]);
        for(int i=0;i<SVPORT;i++)
        {
          for(int j=0;j<SVPIN;j++)
          {
            resvPosArr[i][j]=svPosArr[i][j];
          }
        }
        //Serial.print("svPosArr[0][0]フェーズ2:");
        //Serial.println(svPosArr[0][0]);
        svTimes++;
      }
      else//読み途中ならば
      {
        chr[how] = reads[times];
        how++;
      }
      //Serial.print("svPosArr[0][0]フェーズ3:");
      //Serial.println(svPosArr[0][0]);
      times++;
    }
    motion.close();
    Serial.println("close");
    Serial.flush();
  }
  else//ホームポジションのファイルがなかったら
  {
    Serial.println("###failed read homepositon file###");
  }
}

//UDLinkSystemで必要な処理を始める関数
boolean UDmega::udBegin(){
  boolean ans;
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(100);
  Serial.begin(9600);
  Serial3.begin(115200);
  pinMode(SS,OUTPUT);
  pinMode(MODE,INPUT_PULLUP);
  pinMode(MODELED,OUTPUT);
  SD.begin(chipSelect);
  for(int i=0;i<SVPORT;i++)
  {
    for(int j=0;j<SVPIN;j++)
    {
      servo[i][j].attach(pin[i][j]);
    }
  }
   homeRePosition();
   for(int i=0;i<SVPORT;i++)
  {
    for(int j=0;j<SVPIN;j++)
    {
      Serial.print(homePos[i][j]);
      Serial.print(",");
    }
    Serial.println();
  }
  Serial.println("setupFinish");
  homePosition(0,0,0,false);
  if(digitalRead(MODE)==LOW){
    ans = true;
  }else{
    ans = false;
  }
  
  if(ans == false){
    digitalWrite(MODELED,LOW);
  }
  digitalWrite(13,LOW);
  delay(10);
  return ans;
}

//全部のサーボを一気に動かす関数
void UDmega::allPostion(){
  for(int po=0;po<SVPORT;po++){
    for(int pi=0;pi<SVPIN;pi++){
      servo[po][pi].write(backApp[po][pi]);
    }
  }
  delay(5);
}
