// #include <Servo.h>

// pin connections
// const int servoPin = 7;
const int trigPin = 9;
const int echoPin = 10;

float duration; // variable to store pulse duration
float distanceCM; // variable to store distance in CM
float distanceIN; // variable to store distance in IN

// Servo myServo; // create servo object to control the servo

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // myServo.attach(servoPin); // attaches servo on servoPin to servo object
}

void loop() {
  // put your main code here, to run repeatedly:

  // start with a clean signal
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  //send trigger signal
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // return pulse duration in microseconds
  // if set to HIGH, pulseIn() waits for pin to go from LOW to HIGH
  // stops timing when pin goes back LOW
  duration = pulseIn(echoPin, HIGH);
  // convert m/s to in/microsec
  // 343 m/s = 0.034 cm/microseconds
  distanceCM = (duration * 0.034) / 2;
  // convert to inches, 1in = 2.54cm
  distanceIN = distanceCM / 2.54;

  // print distance
  Serial.print("Distance: ");
  Serial.print(distanceCM);
  Serial.print(" cm | ");
  Serial.print(distanceIN);
  Serial.println(" in");

  // if (distanceIN <= 6) {
  //   // rotate servo to 90 degrees
  //   myServo.write(90);
  // }

  // else {
  //   // rotate / keep servo at 0 degrees
  //   myServo.write(0);
  // }
}
