#define echoPin A1 // Echo Pin
#define trigPin A0 // Trigger pin

int maximumRange = 400; // Maximum range needed
int minimumRange = 2; // Minimum range needed
long duration, distance; // Duration used to calculate distance

int readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  distance = duration/58.2;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  readDistance();

  if(distance>minimumRange && distance<maximumRange) {
    Serial.println(distance); // in cm
  } else {
    Serial.println("Out of range...");
  }
  delay(50);
}
