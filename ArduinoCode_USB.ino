#include <Servo.h>

/* ---------- Servos ---------- */
Servo gripperServo;
Servo wristPitch;   // MG90S
Servo wristRoll;
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;

/* ---------- Smoothing Settings ---------- */
const int UPDATE_INTERVAL = 20;
const float SMOOTH_STEP = 1.0;
unsigned long lastUpdate = 0;

/* ---------- Wrist Pitch (MG90S) ---------- */
bool pitchEnabled = false;
float currentPitch = 90.0;
int targetPitch = 90;
const int PITCH_MIN = 5;
const int PITCH_MAX = 175;

/* ---------- Wrist Roll ---------- */
bool rollEnabled = false;
float currentRoll = 90.0;
int targetRoll = 90;
const int ROLL_MIN = 0;
const int ROLL_MAX = 180;

/* ---------- Base ---------- */
bool baseEnabled = false;
float currentBase = 90.0;
int targetBase = 90;

/* ---------- Shoulder ---------- */
bool shoulderEnabled = false;
float currentShoulder = 90.0;
int targetShoulder = 90;
const int SHOULDER_MIN = 0;
const int SHOULDER_MAX = 180;

/* ---------- Elbow ---------- */
bool elbowEnabled = false;
float currentElbow = 90.0;
int targetElbow = 90;
const int ELBOW_MIN = 0;
const int ELBOW_MAX = 180;

/* ---------- Gripper ---------- */
float currentGripper = 90.0;
int targetGripper = 90;
const float GRIPPER_STEP = 12.0;

/* ---------- General ---------- */
const int JOINT_STEP = 10;

void setup() {
  Serial.begin(38400);
  Serial.setTimeout(10);

  gripperServo.attach(13);
  gripperServo.write((int)currentGripper);

  Serial.println("USB Robotic Arm - FINAL STABLE VERSION");
}

void loop() {

  /* ---------- SERIAL COMMANDS ---------- */
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() > 0) {

      /* ---------- GRIPPER ---------- */
      if (cmd.startsWith("G"))
        targetGripper = constrain(cmd.substring(1).toInt(), 0, 180);
      else if (cmd == "M1") targetGripper = 150;
      else if (cmd == "M2") targetGripper = 40;
      else if (cmd == "M3") targetGripper = 90;
      else if (cmd == "M4") targetGripper = 0;

      /* ---------- WRIST PITCH ---------- */
      else if (cmd == "STP") {
        wristPitch.attach(10, 500, 2500);
        wristPitch.write((int)currentPitch);
        pitchEnabled = true;
      }
      else if (cmd == "SPP") {
        wristPitch.detach();
        pitchEnabled = false;
      }
      else if (cmd.startsWith("WP") && pitchEnabled)
        targetPitch = constrain(cmd.substring(2).toInt(), PITCH_MIN, PITCH_MAX);
      else if (pitchEnabled && cmd == "PL")
        targetPitch = max(PITCH_MIN, targetPitch - JOINT_STEP);
      else if (pitchEnabled && cmd == "PR")
        targetPitch = min(PITCH_MAX, targetPitch + JOINT_STEP);
      else if (pitchEnabled && cmd == "PS")
        targetPitch = 90;

      /* ---------- WRIST ROLL ---------- */
      else if (cmd == "STR") {
        wristRoll.attach(9);
        wristRoll.write((int)currentRoll);
        rollEnabled = true;
      }
      else if (cmd == "SPR") {
        wristRoll.detach();
        rollEnabled = false;
      }
      else if (cmd.startsWith("WR") && rollEnabled)
        targetRoll = constrain(cmd.substring(2).toInt(), ROLL_MIN, ROLL_MAX);
      else if (rollEnabled && cmd == "RL")
        targetRoll = max(ROLL_MIN, targetRoll - 20);
      else if (rollEnabled && cmd == "RR")
        targetRoll = min(ROLL_MAX, targetRoll + 20);
      else if (rollEnabled && cmd == "RS")
        targetRoll = 90;

      /* ---------- BASE ---------- */
      else if (cmd == "STB") {
        baseServo.attach(6);
        baseServo.write((int)currentBase);
        baseEnabled = true;
      }
      else if (cmd == "SPB") {
        baseServo.detach();
        baseEnabled = false;
      }
      else if (cmd.startsWith("B") && cmd.length() > 1 && isDigit(cmd.charAt(1)) && baseEnabled)
        targetBase = constrain(cmd.substring(1).toInt(), 0, 180);
      else if (baseEnabled && cmd == "BL")
        targetBase = max(0, targetBase - JOINT_STEP);
      else if (baseEnabled && cmd == "BR")
        targetBase = min(180, targetBase + JOINT_STEP);

      /* ---------- SHOULDER (FIXED) ---------- */
      else if (cmd == "STS") {
        shoulderServo.attach(7, 500, 2500);
        shoulderServo.write((int)currentShoulder);
        shoulderEnabled = true;
      }
      else if (cmd == "SPS") {
        shoulderServo.detach();
        shoulderEnabled = false;
      }
      else if (cmd.startsWith("S") && cmd.length() > 1 && isDigit(cmd.charAt(1)) && shoulderEnabled)
        targetShoulder = constrain(cmd.substring(1).toInt(), SHOULDER_MIN, SHOULDER_MAX);
      else if (shoulderEnabled && cmd == "SU")
        targetShoulder = min(SHOULDER_MAX, targetShoulder + JOINT_STEP);
      else if (shoulderEnabled && cmd == "SD")
        targetShoulder = max(SHOULDER_MIN, targetShoulder - JOINT_STEP);

      /* ---------- ELBOW ---------- */
      else if (cmd == "STE") {
        elbowServo.attach(8);
        elbowServo.write((int)currentElbow);
        elbowEnabled = true;
      }
      else if (cmd == "SPE") {
        elbowServo.detach();
        elbowEnabled = false;
      }
      else if (cmd.startsWith("E") && cmd.length() > 1 && isDigit(cmd.charAt(1)) && elbowEnabled)
        targetElbow = constrain(cmd.substring(1).toInt(), ELBOW_MIN, ELBOW_MAX);
      else if (elbowEnabled && cmd == "EU")
        targetElbow = min(ELBOW_MAX, targetElbow + JOINT_STEP);
      else if (elbowEnabled && cmd == "ED")
        targetElbow = max(ELBOW_MIN, targetElbow - JOINT_STEP);
    }
  }

  /* ---------- SMOOTH UPDATE ---------- */
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();

    // Gripper (fast)
    if (currentGripper != targetGripper) {
      if (abs(targetGripper - currentGripper) <= GRIPPER_STEP)
        currentGripper = targetGripper;
      else
        currentGripper += (targetGripper > currentGripper) ? GRIPPER_STEP : -GRIPPER_STEP;

      gripperServo.write((int)currentGripper);
    }

    updateServo(wristPitch, currentPitch, targetPitch, pitchEnabled);
    updateServo(wristRoll, currentRoll, targetRoll, rollEnabled);
    updateServo(baseServo, currentBase, targetBase, baseEnabled);
    updateServo(shoulderServo, currentShoulder, targetShoulder, shoulderEnabled);
    updateServo(elbowServo, currentElbow, targetElbow, elbowEnabled);
  }
}

/* ---------- Smooth Servo Helper ---------- */
void updateServo(Servo &servo, float &current, int target, bool enabled) {
  if (!enabled) return;

  if (current != target) {
    if (abs(target - current) <= SMOOTH_STEP)
      current = target;
    else
      current += (target > current) ? SMOOTH_STEP : -SMOOTH_STEP;

    servo.write((int)current);
  }
}
