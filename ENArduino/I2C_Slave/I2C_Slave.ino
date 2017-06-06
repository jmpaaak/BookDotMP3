#include <Wire.h>
#include <Servo.h>
#define SLAVE 4
char buf[20];
volatile byte pos = 0; // 수신 버퍼에 문자를 기록할 위치
volatile boolean process_it = false; // 수신 데이터 종료 알림


/**서보 모터 관련 변수 선언**/
Servo underServo, upperServo;
int underServoPin = 8;
int upperServoPin = 7;
int underServoAngle = 0;
int upperServoAngle = 0;
#define upperServoAngleLimit 17

/****모드 설정****/
boolean Rflag = false;
boolean Pflag = false;
boolean Uflag = false;
boolean Sflag = false;
boolean ReturnFlag = false;
boolean Dflag=false;
int currentMillis;
int previousMillis = 0;



/**PIN_DC 관련 변수 선언**/
#define IN1 4
#define IN2 3
#define ENA 5

/***SETUP & LOOP ***/
void setup() {
  Serial.begin(9600);
  Serial.println("I2C SLAVE");
  // Wire 라이브러리 초기화
  // 슬레이브로 참여하기 위해서는 주소를 지정해야 한다.
  Wire.begin(SLAVE);
  // 마스터의 데이터 전송 요구가 있을 때 처리할 함수 등록
  Wire.onReceive(receiveHandler);
  Wire.onRequest(requestHandler);

  underServo.attach(underServoPin);
  upperServo.attach(upperServoPin);
  underServo.write(0);
  upperServo.write(0);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
}

void receiveHandler()//마스터로 부터 플래그 도착시 슬레이브 플래그 설정
{
  char flag;
  flag = (char)Wire.read();
  Serial.println(flag);
  if (flag == 'T')
  {
    Serial.println("T received");
    Uflag = true;
    process_it = true;
  }
  if (flag == 'R')
  {
    Serial.println("R received");
    Rflag = true;
    previousMillis = millis();
  }
  if (flag == 'U')
  {
    Serial.println("U received");
    Uflag = true;
    previousMillis = millis();
  }
  if (flag == 'S')
  {
    Serial.println("S received");
    Sflag = true;
    previousMillis = millis();
  }
}

void requestHandler()//Done 메시지를 보내주기 위한 함수, Return_Servo까지 정상수행시 Dflag ON
{
  Serial.println("request received");
  if(Dflag)
  {
    Serial.println("Done Sent");
    char data='D';
    Wire.write(data);  
    Dflag=false;
  }
}



void Turn_Servo()//터너 정회전
{
  for (; underServoAngle<110; underServoAngle++)
  {
    delay(10);
    underServo.write(underServoAngle);
    if (underServoAngle > 90)
      for (; upperServoAngle < upperServoAngleLimit; upperServoAngle++) {
        upperServo.write(upperServoAngle);
      }
  }
}

void Pin_DC()//핀
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 180);
}

void Return_Servo()//터너 되돌아오기
{
  for (; upperServoAngle > 0; upperServoAngle--) {
    upperServo.write(upperServoAngle);
  }
  delay(500);
  for (; underServoAngle > 0; underServoAngle--) {
    underServo.write(underServoAngle);
    delay(10);
  }
}

void loop() {
  if (Rflag)
  {
    currentMillis = millis();
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 150);
    if ((currentMillis - previousMillis)>600)
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, 0);
      Rflag = false;
    }
  }
  if (Uflag)
  {
    currentMillis = millis();
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 150);
    if ((currentMillis - previousMillis)>600)
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, 0);
      Uflag = false;
    }
  }
  if (process_it) { // 데이터 수신이 종료된 경우
    Turn_Servo();
    process_it = false;
    Pflag = true;
    previousMillis = millis() + 500;
  }
  if (Pflag)
  {
    currentMillis = millis();
    Pin_DC();
    if ((currentMillis - previousMillis)>1000)
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, 0);
      Pflag = false;
      ReturnFlag = true;
    }
  }
  if (ReturnFlag)
  {
    delay(2000);
    Return_Servo();
    ReturnFlag = false;
    Dflag=true;
  }

  if (Sflag)
  {
    currentMillis = millis();
    Pin_DC();
    if ((currentMillis - previousMillis)>1000)
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, 0);
      Sflag = false;
    }
  }
}
