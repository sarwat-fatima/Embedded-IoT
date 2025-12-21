#define BLYNK_TEMPLATE_ID "TMPL6bnrG4pnK"
#define BLYNK_TEMPLATE_NAME "Smart ClassRoom"
#define BLYNK_AUTH_TOKEN "S6ratnohzoGe9T94WaLasaZ2I6pIge4s"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>

char ssid[] = "Haru";
char pass[] = "Haruki12";

// ================== PINS ==================
const int pirPin = 34;   // Student PIR
const int ldrPin = 35;   // Projector LDR

// ================== VIRTUAL PINS ==================
#define V_STUDENT_PIR   V5
#define V_PROJECTOR     V6
#define V_LIGHT_INT     V13
#define V_PRES_TIME     V10
#define V_ATTENDANCE    V12
#define V_SELECT_STUD   V2
#define V_ACTIVE_STUD   V3
#define V_SCORE         V15
#define V_REMARKS       V16
#define V_SAVE          V17
#define V_SAVED_SCORE   V18
#define V_SAVED_REMARKS V19
#define V_PRES_CHART    V21
#define V_MARKS_CHART   V22
#define V_LDR_THRESHOLD V23 // Slider to adjust LDR threshold

BlynkTimer timer;
WebServer server(80);  // ESP32 Web server

// ================== DATA STRUCTURE ==================
struct StudentData {
  String name;
  int presPercent;
  int presencePercent;
  int score;
  String remarks;
};

StudentData students[] = {
  {"Haruki", 0, 0, 0, ""},
  {"Mano", 0, 0, 0, ""},
  {"Ibana", 0, 0, 0, ""},
  {"Amna", 0, 0, 0, ""}
};

const int STUDENT_COUNT = 4;
int currentStudent = -1;
bool sessionActive = false;

// timers
unsigned long presStart = 0;
unsigned long presenceStart = 0;
unsigned long lastPirTrigger = 0; // for debounce

int presPercent = 0;
int presencePercent = 0;
int score = 0;
String remarks = "";

int ldrThreshold = 2000; // default LDR threshold
const int minPresenceSec = 10; // Minimum seconds to consider present

// ================== RESET WIDGETS ==================
void resetWidgets() {
  presStart = 0;
  presenceStart = 0;
  lastPirTrigger = 0;
  presPercent = 0;
  presencePercent = 0;
  score = 0;
  remarks = "";
  sessionActive = true;

  Blynk.virtualWrite(V_STUDENT_PIR, 0);
  Blynk.virtualWrite(V_PROJECTOR, 0);
  Blynk.virtualWrite(V_LIGHT_INT, 10);
  Blynk.virtualWrite(V_PRES_TIME, 0);
  Blynk.virtualWrite(V_ATTENDANCE, 0);
  Blynk.virtualWrite(V_SCORE, 0);
  Blynk.virtualWrite(V_REMARKS, "");
  Blynk.virtualWrite(V_SAVED_SCORE, "");
  Blynk.virtualWrite(V_SAVED_REMARKS, "");
  Blynk.virtualWrite(V_SAVE, 0);
}

// ================== SENSOR LOOP ==================
void readSensors() {
  if (!sessionActive) return;

  int pir = digitalRead(pirPin);
  int ldr = analogRead(ldrPin);
  bool projectorOn = (ldr > 3000); // Change projector logic based on LDR > 3000

  // PIR debounce
  if (pir == HIGH && (millis() - lastPirTrigger) > 500) {
    lastPirTrigger = millis();
    if (presenceStart == 0) presenceStart = millis();
  }

  if (projectorOn && presStart == 0) presStart = millis();

  // Calculate percentages
  if (presenceStart > 0 && pir == HIGH) {
    presencePercent = map((millis() - presenceStart) / 1000, 0, 600, 0, 100);
    if (presencePercent > 100) presencePercent = 100;
  }

  if (presStart > 0 && projectorOn) {
    presPercent = map((millis() - presStart) / 1000, 0, 600, 0, 100);
    if (presPercent > 100) presPercent = 100;
  }

  // Update widgets
  Blynk.virtualWrite(V_STUDENT_PIR, pir * 100);
  Blynk.virtualWrite(V_PROJECTOR, projectorOn ? 255 : 0);
  int lightLevel = (projectorOn && pir) ? 30 : (pir ? 100 : 10);
  Blynk.virtualWrite(V_LIGHT_INT, lightLevel);
  Blynk.virtualWrite(V_PRES_TIME, presPercent);
  Blynk.virtualWrite(V_ATTENDANCE, presencePercent);

  // Update current student data
  if (currentStudent >= 0 && currentStudent < STUDENT_COUNT) {
    students[currentStudent].presPercent = presPercent;
    students[currentStudent].presencePercent = presencePercent;
  }

  // Serial debug
  Serial.print("PIR: "); Serial.print(pir);
  Serial.print(" | LDR: "); Serial.print(ldr);
  Serial.print(" | Projector: "); Serial.print(projectorOn);
  Serial.print(" | Light%: "); Serial.print(lightLevel);
  Serial.print(" | Presence%: "); Serial.print(presencePercent);
  Serial.print(" | PresTime%: "); Serial.println(presPercent);
}

// ================== SELECT STUDENT ==================
BLYNK_WRITE(V_SELECT_STUD) {
  currentStudent = param.asInt() - 1;
  if (currentStudent < 0 || currentStudent >= STUDENT_COUNT) return;

  Blynk.virtualWrite(V_ACTIVE_STUD, students[currentStudent].name);
  resetWidgets();

  // Update charts for new selection
  for(int i = 0; i < STUDENT_COUNT; i++){
    int presVal = (i == currentStudent) ? students[i].presPercent : 0;
    int scoreVal = (i == currentStudent) ? students[i].score : 0;
    Blynk.virtualWrite(V_PRES_CHART, i+1, presVal);
    Blynk.virtualWrite(V_MARKS_CHART, i+1, scoreVal);
  }
}

// ================== SCORE & REMARKS ==================
BLYNK_WRITE(V_SCORE) {
  score = param.asInt();
  if (currentStudent >= 0) students[currentStudent].score = score;
}

BLYNK_WRITE(V_REMARKS) {
  remarks = param.asString();
  if (currentStudent >= 0) students[currentStudent].remarks = remarks;
}

// ================== SAVE ==================
BLYNK_WRITE(V_SAVE) {
  int state = param.asInt();
  if (state == 1 && currentStudent >= 0) {
    Blynk.virtualWrite(V_SAVED_SCORE, students[currentStudent].score);
    Blynk.virtualWrite(V_SAVED_REMARKS, students[currentStudent].remarks);
    Blynk.virtualWrite(V_PRES_CHART, currentStudent + 1, students[currentStudent].presPercent);
    Blynk.virtualWrite(V_MARKS_CHART, currentStudent + 1, students[currentStudent].score);
    sessionActive = false;
    Blynk.virtualWrite(V_ACTIVE_STUD, "Saved");
  }
}

// ================== LDR THRESHOLD ==================
BLYNK_WRITE(V_LDR_THRESHOLD) {
  ldrThreshold = param.asInt();
  Serial.print("LDR Threshold: "); Serial.println(ldrThreshold);
}

// ================== WEB PAGE ==================
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Classroom Data</title></head><body>";
  html += "<h2>Student Data</h2><table border='1'><tr><th>Name</th><th>Pres %</th><th>Attendance %</th><th>Score</th><th>Remarks</th></tr>";
  for (int i=0;i<STUDENT_COUNT;i++){
    html += "<tr><td>" + students[i].name + "</td><td>" + String(students[i].presPercent) + "</td><td>" + String(students[i].presencePercent) + "</td><td>" + String(students[i].score) + "</td><td>" + students[i].remarks + "</td></tr>";
  }
  html += "</table></body></html>";
  server.send(200, "text/html", html);
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, readSensors);

  // Start WiFi web server
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started.");
}

// ================== LOOP ==================
void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();
}
