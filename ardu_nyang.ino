#include <DHT11.h>
#include <SoftwareSerial.h>

#define BT_RXD 2
#define BT_TXD 3
SoftwareSerial bluetooth(BT_RXD, BT_TXD);


#define LED_RED  4
#define LED_GREEN  1
#define DHT_PIN  A0
#define buzzer  5
#define motor1  6
#define motor2  7
#define light_sensor  A1
#define dust_sensor  A2
#define vcc  5.0 // voltage
#define button  8
#define pulse1_trig 9
#define pulse2_trig 10
#define pulse1_echo 11
#define pulse2_echo 12
#define head_motor 13

DHT11 dht11(DHT_PIN);


float humi, temp;
int light; // 350 기준 
float dust_value;
float distance1, distance2;
int buttonState;
int bt_save = 0;
int day_check = 0; // 낮밤 체크 1초당 4 올라감
int environment = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(button, INPUT_PULLUP); // pullup 더 찾아 보기

}

float calc_distance (int trig, int echo){
  float cycletime;
  float distance;
  
  digitalWrite(trig, HIGH);
  delay(10); 
  digitalWrite(trig, LOW); 
  
  cycletime = pulseIn(echo, HIGH);
  
  distance = ((340 * cycletime) / 10000) / 2;
  return distance;
}
void drive(int dst1, int dst2){
  if ((dst1 > 10) && (dst2 > 10)){
    digitalWrite(motor1, HIGH); digitalWrite(motor2, HIGH);
  } else if ((dst1 > 10) && (dst2 < 10)){ // 우측 앞에 장애물
    analogWrite(motor1, 255); analogWrite(motor2, 100);
  } else if ((dst1 < 10) && (dst2 > 10)){ // 좌측 앞에 장애물
    analogWrite(motor2, 255); analogWrite(motor1, 100);    
  }
}

void loop() {
  char rec_data = 0;
  char charbuf = '0';
  int result = 0;
  if (bluetooth.available()) {
    rec_data = bluetooth.read();
    Serial.println(rec_data);
  }
  if ((rec_data == 's') || (buttonState = digitalRead(button))) { // s 받을 시, 버튼 누를 시 절전 해제
    bt_save = 0;
  }
  else if (rec_data == 'f') { // f 받을 시, 찾기 위해 부저
    tone(buzzer, 255, 255);
  }

  if (bt_save == 1){ // led 제어
    digitalWrite(LED_GREEN, LOW); digitalWrite(LED_RED, LOW);
  } else if (environment == 0){ 
    digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_RED, LOW); // 환경 적절하면 녹색 등
  } else {
    digitalWrite(LED_GREEN, LOW); digitalWrite(LED_RED, HIGH); // 환경 적합하지 않으면 적색 등 
  }

  if (rec_data == 's') { // s 받을 시 절전 해제
    bluetooth.write(charbuf);
    Serial.println(charbuf);
  }


  if (bt_save==0) { // 절전 아닐 때 작동
    humi = dht11.readHumidity();
    temp = dht11.readTemperature();

    light = analogRead(light_sensor); // 350 기준 

    dust_value = (0.17 * (analogRead(dust_sensor) * (5.0/1024)) - 0.1) * 1000;

    if ((dust_value >= 80) || (temp >= 28) || (temp <= 17) || (humi >= 61)) { // 미세먼지 나쁨 이상, 온도, 습도 문제 
      environment = 1;
    } else {
      environment = 0;
    }
    bluetooth.print("humi: ");
    bluetooth.print(humi);
    bluetooth.print(" Temp: ");
    bluetooth.print(temp);
    bluetooth.print(" Dust: ");
    bluetooth.print(dust_value);

    distance1 = calc_distance(pulse1_trig, pulse1_echo);
    distance2 = calc_distance(pulse2_trig, pulse2_echo);
    drive(distance1, distance2); // 측정한 값을 토대로 움직임

    
    if (light <= 350) { // 조도 센서 낮 밤 측정
      day_check += 1; 
    } else {
      day_check = 0;
    }
    if (day_check >= 480){ // 어두운 상태로 2분이 지나면
      bt_save = 1;
    }
    digitalWrite(head_motor, HIGH); // analogWrite(motor, 255);
  } else { // 절전 시
    digitalWrite(head_motor, LOW);
    digitalWrite(motor1, LOW);
    digitalWrite(motor2, LOW);
  }
  delay(250);
}

/// 주기적으로 조도 센서 측정해서 일정 유지시 절전
/// 버튼 누르거나 신호 받을 시 절전 취소 및 초기화


/// 블루투스로 센서 값 전송
/// led는 배터리나 환경 상태 따라 색 변경
