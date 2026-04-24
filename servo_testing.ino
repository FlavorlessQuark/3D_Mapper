#include <Servo.h>

Servo servo;

void setup() {
  // put your setup code here, to run once:
  servo.attach(8);
  servo.write(0);
  delay(200);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int angle = 10; angle < 180; angle++) {
    servo.write(angle);
    delay(15);
  }
  for (int angle = 180; angle > 10; angle--) {
    servo.write(angle);
    delay(15);
  }
}
