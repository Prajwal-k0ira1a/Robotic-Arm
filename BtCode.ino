#include <SoftwareSerial.h>
#include <Servo.h>

/* ---------- Bluetooth ---------- */
SoftwareSerial BT(4, 3);   // RX, TX

/* ---------- Servos ---------- */
Servo gripperServo;
Servo wristPitch;
Servo wristRoll;
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;

/* ---------- System Limits ---------- */
const int JOINT_MIN = 0;
const int JOINT_MAX = 180;
const int JOINT_STEP = 1;

/* ---------- Gripper ---------- */
const int GRIP_OPEN = 150;
const int GRIP_CLOSE = 50;
const int GRIP_CENTER = 90;

/* ---------- Wrist Pitch ---------- */
bool pitchEnabled = false;
const int PITCH_CENTER = 90;

/* ---------- Wrist Roll ---------- */
bool rollEnabled = false;
int rollPos = 40;
const int ROLL_MIN = 0;
const int ROLL_MAX = 180;
const int ROLL_STEP = 20;

/* ---------- Base ---------- */
bool baseEnabled = false;
int basePos = 90;

/* ---------- Shoulder (180° Servo) ---------- */
bool shoulderEnabled = false;
int shoulderPos = 90;
const int SHOULDER_MIN = 0;
const int SHOULDER_MAX = 180;
const int SHOULDER_STEP = 5;

/* ---------- Elbow (180° Servo) ---------- */
bool elbowEnabled = false;
int elbowPos = 90;
const int ELBOW_MIN = 0;
const int ELBOW_MAX = 180;
const int ELBOW_STEP = 5;

/* ---------- Utility ---------- */
void errorMsg(const char* msg) {
  Serial.print("[ERROR] ");
  Serial.println(msg);
}

void infoMsg(const char* msg) {
  Serial.print("[INFO] ");
  Serial.println(msg);
}

void setup() {
  Serial.begin(38400);
  BT.begin(38400);

  gripperServo.attach(13);
  gripperServo.write(GRIP_CENTER);

  infoMsg("Bluetooth Robotic Arm Initialized");
  infoMsg("Waiting for commands...");
}

void loop() {

  if (!BT.available()) return;

  String cmd = BT.readStringUntil(\n);
  cmd.trim();

  if (cmd.length() == 0) {
    errorMsg("Empty command received");
    return;
  }

  /* ---------- GRIPPER ---------- */
  if (cmd == "M1") {
    gripperServo.write(GRIP_OPEN);
  }
  else if (cmd == "M2") {
    gripperServo.write(GRIP_CLOSE);
  }
  else if (cmd == "M3") {
    gripperServo.write(GRIP_CENTER);
  }

  /* ---------- WRIST PITCH ---------- */
  else if (cmd == "STP") {
    wristPitch.attach(10);
    wristPitch.write(PITCH_CENTER);
    pitchEnabled = true;
    infoMsg("Wrist Pitch Enabled");
  }
  else if (cmd == "SPP") {
    wristPitch.detach();
    pitchEnabled = false;
    infoMsg("Wrist Pitch Disabled");
  }
  else if (cmd == "L" || cmd == "R" || cmd == "S") {
    if (!pitchEnabled) {
      errorMsg("Wrist Pitch not enabled (send STP first)");
    } else {
      if (cmd == "L") wristPitch.write(60);
      if (cmd == "R") wristPitch.write(160);
      if (cmd == "S") wristPitch.write(PITCH_CENTER);
    }
  }

  /* ---------- WRIST ROLL ---------- */
  else if (cmd == "STR") {
    wristRoll.attach(9);
    wristRoll.write(rollPos);
    rollEnabled = true;
    infoMsg("Wrist Roll Enabled");
  }
  else if (cmd == "SPR") {
    wristRoll.detach();
    rollEnabled = false;
    infoMsg("Wrist Roll Disabled");
  }
  else if (cmd == "RL" || cmd == "RR") {
    if (!rollEnabled) {
      errorMsg("Wrist Roll not enabled (send STR first)");
    } else {
      if (cmd == "RL") rollPos = max(ROLL_MIN, rollPos - ROLL_STEP);
      if (cmd == "RR") rollPos = min(ROLL_MAX, rollPos + ROLL_STEP);
      wristRoll.write(rollPos);
    }
  }

  /* ---------- BASE ---------- */
  else if (cmd == "STB") {
    baseServo.attach(6);
    baseServo.write(basePos);
    baseEnabled = true;
    infoMsg("Base Enabled");
  }
  else if (cmd == "SPB") {
    baseServo.detach();
    baseEnabled = false;
    infoMsg("Base Disabled");
  }
  else if (cmd == "BL" || cmd == "BR") {
    if (!baseEnabled) {
      errorMsg("Base not enabled (send STB first)");
    } else {
      if (cmd == "BL") basePos = max(JOINT_MIN, basePos - JOINT_STEP);
      if (cmd == "BR") basePos = min(JOINT_MAX, basePos + JOINT_STEP);
      baseServo.write(basePos);
    }
  }

  /* ---------- SHOULDER (180°) ---------- */
  else if (cmd == "STS") {
    shoulderServo.attach(7, 500, 2500);
    shoulderServo.write(shoulderPos);
    shoulderEnabled = true;
    infoMsg("Shoulder Enabled 180°)");
  }
  else if (cmd == "SPS") {
    shoulderServo.detach();
    shoulderEnabled = false;
    infoMsg("Shoulder Disabled");
  }
  else if (cmd == "SU" || cmd == "SD") {
    if (!shoulderEnabled) {
      errorMsg("Shoulder not enabled (send STS first)");
    } else {
      if (cmd == "SU") shoulderPos = min(SHOULDER_MAX, shoulderPos + SHOULDER_STEP);
      if (cmd == "SD") shoulderPos = max(SHOULDER_MIN, shoulderPos - SHOULDER_STEP);
      shoulderServo.write(shoulderPos);
      Serial.print("[INFO] Shoulder Position: ");
      Serial.println(shoulderPos);
    }
  }

  /* ---------- ELBOW (180°) ---------- */
  else if (cmd == "STE") {
    elbowServo.attach(8, 500, 2500);
    elbowServo.write(elbowPos);
    elbowEnabled = true;
    infoMsg("Elbow Enabled (180°)");
  }
  else if (cmd == "SPE") {
    elbowServo.detach();
    elbowEnabled = false;
    infoMsg("Elbow Disabled");
  }
  else if (cmd == "EU" || cmd == "ED") {
    if (!elbowEnabled) {
      errorMsg("Elbow not enabled (send STE first)");
    } else {
      if (cmd == "EU") elbowPos = min(ELBOW_MAX, elbowPos + ELBOW_STEP);
      if (cmd == "ED") elbowPos = max(ELBOW_MIN, elbowPos - ELBOW_STEP);
      elbowServo.write(elbowPos);
      Serial.print("[INFO] Elbow Position: ");
      Serial.println(elbowPos);
    }
  }

  /* ---------- UNKNOWN COMMAND ---------- */
  else {
    Serial.print("[ERROR] Unknown Command: ");
    Serial.println(cmd);
  }
}
