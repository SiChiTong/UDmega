#ifndef _UDmega_h
#define _UDmega_h

#include "Arduino.h"

#include <Servo.h>
#include <SD.h>

#define chipSelect 53
#define CHARSIZE 64//serial.available()が64byteまで保持できないから
#define FILEAll 50
#define SVPORT 5
#define SVPIN 6
#define MAXstep 20
#define MODE 24
#define MODELED 22
#define WAIT 5
#define timeSet 5

struct svCod{
  int timing;
  String sum;
  char com[1];
  String svString;
  String svPort;
  String svPin;
  String svDeg;
  String svStep;
  String motionNum;
};

class UDmega{
  public:
    UDmega(uint8_t num);
    void syokiChar(char be[CHARSIZE]);//char型をリセット
    void taskGUI(char serial[]);//パソコンから指示を受けて仕事する関数
    void remakeHome();//ホームポジションを書き換える関数
    void remakeMotion();//モーションファイルの書き込み
    void homeRePosition();//ホームポジションの角度を読んで動かす関数
    void homePosition(int port,int pin,int deg,boolean juge);//ホームポジションの角度を動かす関数
    void moveServo(int motNum);//サーボモータを動かす関数
    boolean udBegin();//UDLinkSystemで必要な処理を始める関数
    void allPostion();//全部のサーボを一気に動かす関数

  private:
    uint8_t keys = 0x00;
    
    //メインボードのサーボピン一覧
    const int pin[SVPORT][SVPIN] = {
                           {26,28,30,29,31,27},
                           {32,34,36,38,40,42},
                           {A12,A14,44,46,48,47},
                           {37,39,41,43,45,49},
                           {A9,A11,A13,A15,33,35}
                           };
    //const float timeSet = 20.0;
    char sent[CHARSIZE];
    
    int homePos[SVPORT][SVPIN];//ホームポジションを記録しておく配列
    
    int ma[20][5][6];
    
    File homeFile;
    
    svCod svc = {0,{""},{""},{""},{""},{""}};
    
    int motionArray[MAXstep][SVPORT][SVPIN];//ステップごとに各関節の角度を記録しておく配列
    int backApp[SVPORT][SVPIN];//GUIから送られてきた関節の角度を記録しておく配列
    int delayArray[MAXstep];
    boolean programMode;
    int portFile = 0;
    
    Servo servo[SVPORT][SVPIN];
};

#endif
