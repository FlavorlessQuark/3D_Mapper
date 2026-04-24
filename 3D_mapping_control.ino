#include <Servo.h>
#include <string.h>

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
const int SAMPLES_PER_ROW = 64;

const unsigned int HALFSTEP_DELAY_US = 2000; // Delay between each halfstep
const unsigned int STEPPER_SETTLE_MS = 10; // Small wait after stepper stops so vibrations die down before taking reading

// Tilt Scan Tuning
const int TILT_START_DEG = 90;
const int TILT_MAX_DEG = 140;
const int TILT_STEP_DEG = 8;
const unsigned int SERVO_SETTLE_MS = 250; // Wait after moving servo so it physically reaches target angle
const int SERVO_LEVEL_DEG = 90; // Raw servo angle that's treated as 0 degrees elevation (vertical angle correction)
const int ELEVATION_SIGN = +1; 

// Sonar Tuning
const unsigned long ECHO_TIMEOUT_US = 30000UL; // Stop waiting for echo after 30,000 microseconds
const unsigned long PING_GAP_MS = 60UL; // Wait at least 60ms between pings

// Scan Behavior
const bool SERPENTINE_AZIMUTH = true;

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

int tiltDeg = TILT_START_DEG;
int rowIndex = 0;

float rowYawDeg[SAMPLES_PER_ROW];
float rowDistanceCm[SAMPLES_PER_ROW];
int rowSampleCount = 0;
int rowServoDeg = TILT_START_DEG;
float rowElevationDeg = 0.0f;

// State machine
enum ScannerState {
  WAITING_FOR_START,
  CAPTURING_ROW,
  SENDING_ROW,
  WAITING_FOR_READY,
  ADVANCING_OR_DONE,
  FINISHED
};

ScannerState state = CAPTURING_ROW;

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

float elevationFromServoDeg(int servoDeg) {
  return ELEVATION_SIGN * (servoDeg - SERVO_LEVEL_DEG);
}

float horizontalProjectedDistanceCm(float distanceCm, float elevationDeg) { // Flattened radius for debugging
  if (distanceCm < 0.0f) { // If the sonar reading was invalid
    return -1.0f; // Return invalid here
  }

  return distanceCm * cos(radians(elevationDeg)); // Line-of-sight distance onto the horizontal plane
}

float sampleDistanceAtCurrentPose() {
  delay(STEPPER_SETTLE_MS);
  float d = readDistanceCm(TRIG1_PIN, ECHO1_PIN);
  delay(PING_GAP_MS);
  return d;
}

bool readCommand(char *out, size_t outSize) { // Reads a newline-terminated command like START or READY from Serial
  static char buffer[24]; // Temporary storage for incoming command
  static size_t idx = 0; // Keeps track of where next incoming character goes in buffer

  while (Serial.available() > 0) {
    char c = Serial.read(); // Read one character from Serial

    if (c == '\r') { // Ignores carriage return
      continue;
    }

    if (c == '\n') { // Command is finished
      if (idx == 0) { // Ignore empty line
        continue;
      }

      buffer[idx] = '\0'; // Adds null terminator so buffer becomes proper C string
      strncpy(out, buffer, outSize); // Copies finished command into output array passed into function
      out[outSize - 1] = '\0'; // Guarantees copied string is null-terminated
      idx = 0; // Resets buffer index so next command can be read fresh
      return true;
    }

    if (idx < sizeof(buffer) - 1) {
      buffer[idx++] = c;
    }
  }

  return false;
}

bool gotCommand(const char *wanted) {
  char cmd[24]; // Creates local buffer to hold incoming command

  if (!readCommand(cmd, sizeof(cmd))) { // If no full command has been received yet
    return false;
  }

  return strcmp(cmd, wanted) == 0; // Returns true only if received command matches wanted
}

void captureOneFullRow() { // Row capture
  rowSampleCount = 0;
  azimuthHalfstepsIntoSweep = 0;

  rowServoDeg = tiltDeg;
  rowElevationDeg = elevationFromServoDeg(rowServoDeg);

  // Sample at the starting heading first
  rowYawDeg[rowSampleCount] = currentYawDeg();
  rowDistanceCm[rowSampleCount] = sampleDistanceAtCurrentPose();
  rowSampleCount++;

  // Sample the rest of the row
  for (int i = 1; i < SAMPLES_PER_ROW; i++) {
    moveHalfsteps(AZIMUTH_SAMPLE_HALFSTEPS, sweepDir);
    azimuthHalfstepsIntoSweep += AZIMUTH_SAMPLE_HALFSTEPS;

    rowYawDeg[rowSampleCount] = currentYawDeg();
    rowDistanceCm[rowSampleCount] = sampleDistanceAtCurrentPose();
    rowSampleCount++;
  }

  // Finish the last chunk so the motor completes the full 360 and returns to the same yaw reference
  moveHalfsteps(AZIMUTH_SAMPLE_HALFSTEPS, sweepDir);
  azimuthHalfstepsIntoSweep = 0;
}

void sendBufferedRow() {
  Serial.print("# ROW_BEGIN,");
  Serial.print(rowIndex);
  Serial.print(",");
  Serial.print(rowServoDeg);
  Serial.print(",");
  Serial.print(rowElevationDeg, 2);
  Serial.print(",");
  Serial.print(rowSampleCount);
  Serial.print(",");
  Serial.println((sweepDir > 0) ? "FWD" : "REV");

  for (int i = 0; i < rowSampleCount; i++) {
    float horizCm = horizontalProjectedDistanceCm(rowDistanceCm[i], rowElevationDeg);

    // yaw_deg,elev_deg,horiz_cm
    // Serial.print(rowIndex);
    // Serial.print(",");
    // Serial.print(i);
    // Serial.print(",");
    Serial.print(rowYawDeg[i], 2);
    // Serial.print(",");
    // Serial.print(rowServoDeg);
    Serial.print(",");
    Serial.print(rowElevationDeg, 2);
    // Serial.print(",");
    // Serial.print(rowDistanceCm[i], 2);
    Serial.print(",");
    Serial.println(horizCm, 2);
  }

  Serial.print("# ROW_END,");
  Serial.println(rowIndex);

  Serial.flush();
}

bool advanceTiltUp() {
  int nextTilt = tiltDeg + TILT_STEP_DEG;

  if (nextTilt > TILT_MAX_DEG) {
    return false;
  }

  tiltDeg = nextTilt;
  tiltServo.write(tiltDeg);
  delay(SERVO_SETTLE_MS);
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
  delay(SERVO_SETTLE_MS);

  if ((HALFSTEPS_PER_REV % AZIMUTH_SAMPLE_HALFSTEPS) != 0 || 
      (HALFSTEPS_PER_REV / AZIMUTH_SAMPLE_HALFSTEPS) != SAMPLES_PER_ROW) {
    Serial.println("# CONFIG_ERROR");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("# SCANNER_READY");
  Serial.println("# AUTO_STARTING");
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case WAITING_FOR_START:
      if (gotCommand("START")) {
        Serial.println("# START_ACK");
        state = CAPTURING_ROW;
      }
      break;

    case CAPTURING_ROW:
      captureOneFullRow();
      state = SENDING_ROW;
      break;

    case SENDING_ROW:
      sendBufferedRow();
      // Serial.print("# WAITING_READY,");
      // Serial.println(rowIndex);
      // state = WAITING_FOR_READY;
      state = ADVANCING_OR_DONE;
      break;

    case WAITING_FOR_READY:
      if (gotCommand("READY")) {
        state = ADVANCING_OR_DONE;
      }
      break;

    case ADVANCING_OR_DONE:
      // Reverse direction for the next full 360 so wires do not keep twisting
      sweepDir = -sweepDir; 

      // Move tilt upward for the next row, or stop if we reached the maximum
      if (!advanceTiltUp()) {
        releaseStepper();
        Serial.println("# SCAN_DONE");
        state = FINISHED;
      } else {
        rowIndex++;
        state = CAPTURING_ROW;
      }
      break;

    case FINISHED:
      return;
  }
}
