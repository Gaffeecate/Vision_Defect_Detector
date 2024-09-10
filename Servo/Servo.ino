#include <Servo.h>

Servo myservo;
int servoPin = 9; // 서보 모터 연결 핀 PWM 핀에 꽃힌 번호
int angle = 0; // 현재 각도
bool isRunning = false; // 동작 여부를 나타내는 플래그

void setup() {
  myservo.attach(servoPin);
  Serial.begin(9600); // BaudRate = 9600
  myservo.write(0); // 초기 위치를 0도로 설정
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "START") {
      isRunning = true;
      Serial.println("Servo motor started");
    } else if (command == "STOP") {
      isRunning = false;
      Serial.println("Servo motor stopped");
    } else if (command == "MOVE") {
      if (isRunning) {
        angle += 15; // 15도씩 증가
        if (angle > 180) {
          angle = 0; // 180도를 초과하면 0도로 초기화
        }
        myservo.write(angle);
        Serial.print("Servo rotated to ");
        Serial.print(angle);
        Serial.println(" degrees");
      }
    }
  }
}