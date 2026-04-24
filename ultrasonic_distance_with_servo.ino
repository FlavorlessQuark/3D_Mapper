#include <Servo.h>

// pin connections
#define servoPin 9
#define trigPin A0
#define echoPin A1
#define STEP_IN1 4
#define STEP_IN2 5
#define STEP_IN3 6
#define STEP_IN4 7

float duration; // variable to store pulse duration
float distanceCM; // variable to store distance in CM
float distanceIN; // variable to store distance in IN

Servo myServo; // create servo object to control the servo

// Stepper / Scan Tuning
const long HALFSTEPS_PER_REV = 4096L; // Approx # of halfsteps for one full revolution

const int AZIMUTH_SAMPLE_HALFSTEPS = 64; // After every 64 half steps, stop and take ultrasonic reading

const unsigned int HALFSTEP_DELAY_US = 2000; // Delay between each halfstep
const unsigned int STEPPER_SETTLE_MS = 10; // Small wait after stepper stops so vibrations die down before taking reading

// Scan Behavior
const bool SERPENTINE_AZIMUTH = false; // back and forth scan to avoid twisting wires
long totalHalfsteps = 0; // Keeps counting upward forever

// Full step sequence for a 4-wire bipolar stepper on dual H-bridges
const uint8_t HALFSTEP_SEQ[8][4] = {
  {1, 0, 0, 0}, // Step state 0: energyze coil pattern 1000
  {1, 1, 0, 0}, // Step state 1: energyze coil pattern 1100
  {0, 1, 0, 0}, // Step state 2: energyze coil pattern 0100
  {0, 1, 1, 0}, // Step state 3: energyze coil pattern 0110
  {0, 0, 1, 0}, // Step state 4: energyze coil pattern 0010
  {0, 0, 1, 1}, // Step state 5: energyze coil pattern 0011
  {0, 0, 0, 1}, // Step state 6: energyze coil pattern 0001
  {1, 0, 0, 1} // Step state 7: energyze coil pattern 1001
};

int halfStepIndex = 0; // Tracks which row of halfstep_seq currently using

long azimuthHalfstepsIntoSweep = 0;  // Counts how many halfsteps moved through the current horizontal sweep
int sweepDir = +1;                   // +1=rotate one direction or -1=rotate other direction

float wrap360(float deg) { // Force any angle into range 0 to <360
  while (deg < 0.0f) deg += 360.0f; // If negative, add 360 until non-negative
  while (deg >= 360.0f) deg -= 360.0f; // If more than 360, subtract 360 until below 360
  return deg;
}

void applyHalfStepState(int idx) { // Sends one motor state to driver module inputs
  digitalWrite(STEP_IN1, HALFSTEP_SEQ[idx][0]); // Write the first value of the selected step state to IN1
  digitalWrite(STEP_IN2, HALFSTEP_SEQ[idx][1]); // Write the second value of the selected step state to IN2
  digitalWrite(STEP_IN3, HALFSTEP_SEQ[idx][2]); // Write the third value of the selected step state to IN3
  digitalWrite(STEP_IN4, HALFSTEP_SEQ[idx][3]); // Write the fourth value of the selected step state to IN4
}

void releaseStepper() { // Turn off all motor coils when done
  digitalWrite(STEP_IN1, LOW);
  digitalWrite(STEP_IN2, LOW);
  digitalWrite(STEP_IN3, LOW);
  digitalWrite(STEP_IN4, LOW);
}

void stepMotorOneHalfstep(int dir) {
  if (dir > 0) {
    halfStepIndex = (halfStepIndex + 1) & 0x07; // Move to the next row of the 8-step sequence, wrapping around after 7
  } else {
    halfStepIndex = (halfStepIndex + 7) & 0x07; // Sequence in reverse
  }

  applyHalfStepState(halfStepIndex);
  delayMicroseconds(HALFSTEP_DELAY_US);
}

void moveHalfsteps(int count, int dir) { // Move stepper several halfsteps in a row
  for (int i = 0; i < count; i++) {
    stepMotorOneHalfstep(dir);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(servoPin); // attaches servo on servoPin to servo object

  pinMode(STEP_IN1, OUTPUT);
  pinMode(STEP_IN2, OUTPUT);
  pinMode(STEP_IN3, OUTPUT);
  pinMode(STEP_IN4, OUTPUT);

  delay(500);
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

  if (distanceIN <= 6) {
    // rotate servo to 90 degrees
    myServo.write(90);
  }

  else {
    // rotate / keep servo at 0 degrees
    myServo.write(0);
  }

  long remaining = HALFSTEPS_PER_REV - azimuthHalfstepsIntoSweep; // Compute how many halfsteps remain in sweep row

  // Move and sample while there is still more than one sample-chunk left
  if (remaining > AZIMUTH_SAMPLE_HALFSTEPS) {
    moveHalfsteps(AZIMUTH_SAMPLE_HALFSTEPS, sweepDir); // Rotate horizontally by one chunk
    azimuthHalfstepsIntoSweep += AZIMUTH_SAMPLE_HALFSTEPS;
    return;
  }

  // Finish the remainder of the revolution without taking a duplicate 360-degree sample
  if (remaining > 0) {
    moveHalfsteps((int)remaining, sweepDir); // Move final leftover halfsteps to finish row exactly
    azimuthHalfstepsIntoSweep = 0;
  }

  // Row complete, reverse if serpentine
  if (SERPENTINE_AZIMUTH) {
    sweepDir = -sweepDir; // Reverse horizontal direction for next row
  }

  delay(500);
}
