#include <Servo.h>

// Pin Assignments
#define STEP_IN1 4
#define STEP_IN2 5
#define STEP_IN3 6
#define STEP_IN4 7

#define SERVO_PIN 9

#define TRIG1_PIN A0
#define ECHO1_PIN A1

// Stepper Tuning
const long HALFSTEPS_PER_REV = 4096L; // Approx # of halfsteps for one full revolution

const int AZIMUTH_SAMPLE_HALFSTEPS = 64; // After every 64 half steps, stop and take ultrasonic reading

const unsigned int HALFSTEP_DELAY_US = 2000; // Delay between each halfstep
const unsigned int STEPPER_SETTLE_MS = 10; // Small wait after stepper stops so vibrations die down before taking reading

// Tilt Scan Tuning
const int TILT_MIN_DEG = 40;
const int TILT_MAX_DEG = 140;
const int TILT_STEP_DEG = 8;
const unsigned int SERVO_SETTLE_MS = 250; // Wait after moving servo so it physically reaches target angle
const int SERVO_LEVEL_DEG = 90; // Raw servo angle that's treated as 0 degrees elevation (vertical angle correction)
const int ELEVATION_SIGN = +1; 

// Sonar Tuning
const unsigned long ECHO_TIMEOUT_US = 30000UL; // Stop waiting for echo after 30,000ms
const unsigned long PING_GAP_MS = 60UL; // Wait at least 60ms between pings

// Scan Behavior
const bool SERPENTINE_AZIMUTH = true; // back and forth scan to avoid twisting wires
const bool BOUNCE_TILT = false; // false=tile upward once then stop when top is reached

// Globals
Servo tiltServo;

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

int tiltDeg = TILT_MIN_DEG;
int tiltDir = +1; // +1=tilt upward, -1=tilt downward

bool scanDone = false;

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

unsigned long pingMicroseconds(byte trigPin, byte echoPin) { // Triggers on ultrasonic module and measures echo pulse length
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  return pulseIn(echoPin, HIGH, ECHO_TIMEOUT_US);
}

float readDistanceCm(byte trigPin, byte echoPin) { // Convert ultrasonic module reading into cm
  unsigned long duration = pingMicroseconds(trigPin, echoPin);

  if (duration == 0) {
    return -1.0f;
  }
  return duration / 58.0f;
}

float currentYawDeg() { // Calculate current horizontal angle of stepper scan
  float yaw = (360.0f * azimuthHalfstepsIntoSweep) / HALFSTEPS_PER_REV;

  if (SERPENTINE_AZIMUTH && sweepDir < 0) {
    yaw = 360.0f - yaw;
  }
  return wrap360(yaw);
}

float currentElevationDeg() { // Convert raw servo angle into corrected elevation angle
  return ELEVATION_SIGN * (tiltDeg - SERVO_LEVEL_DEG); // 0 = level with ground
}

float horizontalProjectedDistanceCm(float distanceCm, float elevationDeg) { // Flattened radius for debugging
  if (distanceCm < 0.0f) { // If the sonar reading was invalid
    return -1.0f; // Return invalid here
  }

  return distanceCm * cos(radians(elevationDeg)); // Line-of-sight distance onto the horizontal plane
}

void sendSample(float yawDeg, int servoDeg, float elevationDeg, float distanceCm) { // Print one CSV data line
  float horizCm = horizontalProjectedDistanceCm(distanceCm, elevationDeg); // Compute projected horizontal distance

  Serial.print("P,");
  Serial.print(yawDeg, 2); // Print yaw angle in degrees
  Serial.print(",");
  Serial.print(servoDeg); // Print raw servo angle
  Serial.print(",");
  Serial.print(elevationDeg, 2); // Print corrected elevation angle
  Serial.print(",");
  Serial.print(distanceCm, 2); // Print raw line-of-sight distance
  Serial.print(",");
  Serial.println(horizCm, 2); // Print horizontal projected distance
}

void takeSamplesAtCurrentPose() { // Take one reading at the current yaw + tilt
  delay(STEPPER_SETTLE_MS);

  float yawDeg = currentYawDeg(); // Compute the current horizontal angle
  float elevDeg = currentElevationDeg(); // Compute the corrected vertical angle
  float distanceCm = readDistanceCm(TRIG1_PIN, ECHO1_PIN); // Read the sonar distance

  sendSample(yawDeg, tiltDeg, elevDeg, distanceCm); // Print the measurement to Serial

  delay(PING_GAP_MS); // Wait before the next sonar ping
}

bool advanceTilt() { // Move servo to next position
  int nextTilt = tiltDeg + (tiltDir * TILT_STEP_DEG);

  if (nextTilt > TILT_MAX_DEG || nextTilt < TILT_MIN_DEG) { // If next row would exceed allowed tilt range
    if (!BOUNCE_TILT) { // If don't want to bounce back downward
      return false; // Tell loop scan is finished
    }

    tiltDir = -tiltDir; // Reverse tilt direction
    nextTilt = tiltDeg + (tiltDir * TILT_STEP_DEG); // Recompute tilt in new direction

    if (nextTilt > TILT_MAX_DEG) nextTilt = TILT_MAX_DEG; // Clamp to max if needed
    if (nextTilt < TILT_MIN_DEG) nextTilt = TILT_MIN_DEG; // Clamp to min if needed
  }

  tiltDeg = nextTilt; // Save new raw servo angle
  tiltServo.write(tiltDeg); // Command servo to move there
  delay(SERVO_SETTLE_MS); // Wait for servo to reach angle
  return true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(STEP_IN1, OUTPUT);
  pinMode(STEP_IN2, OUTPUT);
  pinMode(STEP_IN3, OUTPUT);
  pinMode(STEP_IN4, OUTPUT);

  pinMode(TRIG1_PIN, OUTPUT);
  pinMode(ECHO1_PIN, INPUT);

  digitalWrite(TRIG1_PIN, LOW);

  tiltServo.attach(SERVO_PIN);
  tiltServo.write(tiltDeg);

  delay(500);

  Serial.println("# P,yaw_deg,servo_deg,elev_deg,distance_cm,horiz_cm");
  takeSamplesAtCurrentPose(); // first sample at starting pose
}

void loop() {
  // put your main code here, to run repeatedly:
  if (scanDone) {
    return;
  }

  long remaining = HALFSTEPS_PER_REV - azimuthHalfstepsIntoSweep; // Compute how many halfsteps remain in sweep row

  // Move and sample while there is still more than one sample-chunk left
  if (remaining > AZIMUTH_SAMPLE_HALFSTEPS) {
    moveHalfsteps(AZIMUTH_SAMPLE_HALFSTEPS, sweepDir); // Rotate horizontally by one chunk
    azimuthHalfstepsIntoSweep += AZIMUTH_SAMPLE_HALFSTEPS;
    takeSamplesAtCurrentPose(); // Take distance readings
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

  // Advance tilt
  if (!advanceTilt()) { // Move servo up to next tilt row
    scanDone = true;
    releaseStepper();
    Serial.println("# SCAN_DONE");
    return;
  }

  // Sample at the start of the new tilt row
  takeSamplesAtCurrentPose();
}
