#include <Wire.h>
#define SLAVE 4 // 슬레이브 주소

/**DC모터 구동 관련 변수선언***/
#define IN1 4
#define IN2 3
#define ENA 5//ROLL_DC
#define IN3 8
#define IN4 9
#define ENB 10//SET_DC

/** 모드 설정 **/
boolean Tflag = false;//turn (i2c_pin up -> roll -> i2c_turn-> i2c_set)
boolean Rflag = false;//ready (i2c_pin up -> i2c up)
boolean Uflag = false;//unfix (i2c_pin up -> roll up)
boolean Sflag = false;//set (i2c_pin down -> roll down)
boolean TUflag = false;// (i2c_pin up)
boolean i2cFlag = false;
int previousMillis = 0;
int currentMillis;
byte request;



/***SETUP & LOOP ****/
void setup() {
  Wire.begin(); // Wire 마스터 설정
  Serial.begin(9600); // 직렬 통신 초기화
  Serial.println("I2C MASTER");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);//롤러 DC모터 핀 설정

}

void loop() {
  if (Tflag) // Turn : 롤러DC 회전 -> SPI -> 터닝 -> 피닝 (페이지 넘김)
  {
    if (TUflag)
    {
      Tflag = false;
      WIREcom();
      Tflag = true;
    }
    currentMillis = millis();
    Roll_DC();
    if ((currentMillis - previousMillis)>800)
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, 0); // DC OFF
      WIREcom();
      Serial.write(request);
      Serial.println("");
    }
  }
  if (Rflag)//Ready : 핀 올림 & 롤러 서보 올림 (책 대기 상태)
  {
    currentMillis=millis();
    if (i2cFlag)
    {
      WIREcom();
      i2cFlag = false;
    }
    rollUP_DC();
    if ((currentMillis - previousMillis) > 1300)
    {
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(ENB, 0);
      Rflag = false;
    }
  }
  if (Sflag)//Setting : 핀 내림 & 롤러 DC 내림 (최초 책 안착 상태)
  {
    currentMillis = millis();
    if (i2cFlag)
    {
      WIREcom();
      i2cFlag = false;
    }
    rollDOWN_DC();
    if ((currentMillis - previousMillis) > 1300)
    {
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(ENB, 0);
      Sflag = false;
    }

  }
  if (Uflag)//Unfix : 마스터는 롤러 들어주고, 슬레이브에서 핀 올림
  {
    currentMillis = millis();
    if (i2cFlag)
    {
      WIREcom();
      i2cFlag = false;
    }
    rollUP_DC();
    if ((currentMillis - previousMillis) > 2000)
    {
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite(ENB, 0);
      Uflag = false;
    }
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 'R')
    {
      i2cFlag = true;
      Rflag = true;
      previousMillis = millis();
    }
    if (inChar == 'T')
    {
      Tflag = true;
      TUflag = true;
      previousMillis = millis();
    }
    if (inChar == 'U')
    {
      i2cFlag = true;
      Uflag = true;
      previousMillis = millis();
    }
    if (inChar == 'S')
    {
      previousMillis = millis();
      i2cFlag = true;
      Sflag = true;
    }
  }
}
void Roll_DC(void)
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 150);
}
void rollUP_DC(void)
{
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 250);//속도를 너무 낮게 설정하면 아예 안돌아감!!
}
void rollDOWN_DC(void)
{
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 150);//속도를 너무 낮게 설정하면 아예 안돌아감!!
}
void WIREcom(void)
{
  const char Turn = 'T';
  const char Ready = 'R';
  const char Unfix = 'U';
  const char Setting = 'S';
  Wire.beginTransmission(SLAVE);//슬레이브 주소 설정
  if (Tflag)
  {
    Wire.write(Turn);
    Wire.endTransmission();
    Serial.println("Turn Data sent");
    delay(7000);
    if (Wire.requestFrom(SLAVE, 1))
    {
      request = Wire.read();
    }
    Tflag = false;
  }
  if (Rflag)
  {
    Wire.write(Ready);
    Wire.endTransmission();
    Serial.println("Ready Data sent");
  }
  if (Uflag || TUflag)
  {
    Wire.write(Unfix);
    Wire.endTransmission();
    if (!TUflag)
      Serial.println("Unfix Data sent");
    TUflag = false;
  }
  if (Sflag)
  {
    Wire.write(Setting);
    Wire.endTransmission();
    Serial.println("Setting Data sent");
  }
}
