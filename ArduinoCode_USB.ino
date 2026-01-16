#include <Servo.h>

/* ---------- Servos ---------- */
Servo gripperServo;
Servo wristPitch;
Servo wristRoll;
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;

/* ---------- Smoothing Settings ---------- */
const int UPDATE_INTERVAL = 15; // Update every 15ms (~66Hz)
const float SMOOTH_STEP = 3.0;  // Max degrees to move per interval (Adjust for speed)
unsigned long lastUpdate = 0;

/* ---------- Wrist Pitch ---------- */
bool pitchEnabled = false;
float currentPitch = 90.0;
int targetPitch = 90;

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
float currentShoulder = 135.0;   // start mid‑range
int targetShoulder = 135;
const int SHOULDER_MIN = 0;
const int SHOULDER_MAX = 270;
/* ---------- Elbow ---------- */
bool elbowEnabled = false;
float currentElbow = 90.0;
int targetElbow = 90;

/* ---------- Gripper ---------- */
float currentGripper = 90.0;
int targetGripper = 90;

/* ---------- Joint Limits ---------- */
const int JOINT_MIN = 0;
const int JOINT_MAX = 180;
const int JOINT_STEP = 10; // For relative moves

void setup() {
  Serial.begin(38400);
  Serial.setTimeout(10);

  gripperServo.attach(13);
  gripperServo.write(targetGripper);

  Serial.println("USB Robotic Arm - SMOOTH CONTROL READY");
}

void loop() {
  // 1. Read Serial Commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.length() > 0) {
      char type = cmd.charAt(0);
      String valStr = cmd.substring(1);
      int val = valStr.toInt();

      /* ---------- GRIPPER ---------- */
      if (type == 'G') {
        targetGripper = constrain(val, 0, 180);
      }
      else if (cmd == "M1") targetGripper = 150;
      else if (cmd == "M2") targetGripper = 50;
      else if (cmd == "M3") targetGripper = 90;
      else if (cmd == "M4") targetGripper = 0;

      /* ---------- WRIST PITCH ---------- */
      else if (cmd == "STP") { wristPitch.attach(10); pitchEnabled = true; }
      else if (cmd == "SPP") { wristPitch.detach(); pitchEnabled = false; }
      else if (cmd.startsWith("WP") && pitchEnabled) {
        targetPitch = constrain(cmd.substring(2).toInt(), 0, 180);
      }
      // Legacy Relative
      else if (pitchEnabled && cmd == "L") targetPitch = max(0, targetPitch - 10);
      else if (pitchEnabled && cmd == "R") targetPitch = min(180, targetPitch + 10);
      else if (pitchEnabled && cmd == "S") targetPitch = 90;

      /* ---------- WRIST ROLL ---------- */
      else if (cmd == "STR") { wristRoll.attach(9); rollEnabled = true; }
      else if (cmd == "SPR") { wristRoll.detach(); rollEnabled = false; }
      else if (cmd.startsWith("WR") && rollEnabled) {
        targetRoll = constrain(cmd.substring(2).toInt(), ROLL_MIN, ROLL_MAX);
      }
      else if (rollEnabled && cmd == "RL") targetRoll = max(ROLL_MIN, targetRoll - 20);
      else if (rollEnabled && cmd == "RR") targetRoll = min(ROLL_MAX, targetRoll + 20);
      else if (rollEnabled && cmd == "RS") targetRoll = 40;

      /* ---------- BASE ---------- */
      else if (cmd == "STB") { baseServo.attach(6); baseEnabled = true; }
      else if (cmd == "SPB") { baseServo.detach(); baseEnabled = false; }
      else if (type == 'B' && baseEnabled && cmd.length() > 1 && isDigit(cmd.charAt(1))) {
        targetBase = constrain(val, JOINT_MIN, JOINT_MAX);
      }
      else if (baseEnabled && cmd == "BL") targetBase = max(JOINT_MIN, targetBase - JOINT_STEP);
      else if (baseEnabled && cmd == "BR") targetBase = min(JOINT_MAX, targetBase + JOINT_STEP);

     /* ---------- SHOULDER (MG665R 270°) ---------- */
      else if (cmd == "STS") { shoulderServo.attach(7, 500, 2500); shoulderEnabled = true; }
      else if (cmd == "SPS") { shoulderServo.detach(); shoulderEnabled = false; }
      else if (type == 'S' && shoulderEnabled && cmd.length() > 1 && isDigit(cmd.charAt(1)) 
               && !cmd.startsWith("ST") && !cmd.startsWith("SP")) {
        targetShoulder = constrain(val, SHOULDER_MIN, SHOULDER_MAX);
      }
      else if (shoulderEnabled && cmd == "SU") targetShoulder = min(SHOULDER_MAX, targetShoulder + JOINT_STEP);
      else if (shoulderEnabled && cmd == "SD") targetShoulder = max(SHOULDER_MIN, targetShoulder - JOINT_STEP);



      /* ---------- ELBOW ---------- */
      else if (cmd == "STE") { elbowServo.attach(8); elbowEnabled = true; }
      else if (cmd == "SPE") { elbowServo.detach(); elbowEnabled = false; }
      else if (type == 'E' && elbowEnabled && cmd.length() > 1 && isDigit(cmd.charAt(1))) {
        targetElbow = constrain(val, JOINT_MIN, JOINT_MAX);
      }
      else if (elbowEnabled && cmd == "EU") targetElbow = min(JOINT_MAX, targetElbow + JOINT_STEP);
      else if (elbowEnabled && cmd == "ED") targetElbow = max(JOINT_MIN, targetElbow - JOINT_STEP);
    }
  }

  // 2. Smooth Movement Update
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    
    updateServo(gripperServo, currentGripper, targetGripper, true); // Gripper always enabled in this code
    updateServo(wristPitch, currentPitch, targetPitch, pitchEnabled);
    updateServo(wristRoll, currentRoll, targetRoll, rollEnabled);
    updateServo(baseServo, currentBase, targetBase, baseEnabled);
    updateServo(shoulderServo, currentShoulder, targetShoulder, shoulderEnabled);
    updateServo(elbowServo, currentElbow, targetElbow, elbowEnabled);
  }
}

void updateServo(Servo &servo, float &current, int target, bool enabled) {
  if (!enabled) return;
  
  if (current != target) {
    if (abs(target - current) <= SMOOTH_STEP) {
      current = target;
    } else {
      if (target > current) current += SMOOTH_STEP;
      else current -= SMOOTH_STEP;
    }
    servo.write((int)current);
  }
}
