#include <Servo.h>

Servo myservo;
int servoPin = 9; // 서보 모터 연결 핀

void setup() {
  myservo.attach(servoPin);
  Serial.begin(9600);
  Serial.println("Enter angle (0-180):");
}

void loop() {
  if (Serial.available() > 0) {
    int angle = Serial.parseInt();
    
    if (angle >= 0 && angle <= 180) {
      myservo.write(angle);
      Serial.print("Servo rotated to ");
      Serial.print(angle);
      Serial.println(" degrees");
    } else {
      Serial.println("Invalid angle. Enter 0-180");
    }
    
    while (Serial.available() > 0) {
      Serial.read();
    }
    
    Serial.println("Enter next angle (0-180):");
  }
}