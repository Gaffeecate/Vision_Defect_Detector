#include <Servo.h>

Servo myservo;
int servoPin = 9; // 서보 모터 연결 핀
int angle = 0; // 시작 각도
unsigned long previousMillis = 0; // 이전에 각도를 변경한 시각을 저장
const long interval = 3000; // 각도 변경 간격 (밀리초 단위)

void setup() {
  myservo.attach(servoPin);
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (angle <= 180) {
      myservo.write(angle);
      Serial.print("Servo rotated to ");
      Serial.print(angle);
      Serial.println(" degrees");
      angle += 10; // 각도를 10도 증가
    } else {
      angle = 0; // 각도가 180도를 초과하면 0도로 초기화
    }
  }
}
