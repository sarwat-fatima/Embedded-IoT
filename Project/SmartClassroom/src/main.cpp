#define BLYNK_TEMPLATE_ID "TMPL6bnrG4pnK"
#define BLYNK_TEMPLATE_NAME "Smart ClassRoom"
#define BLYNK_AUTH_TOKEN "S6ratnohzoGe9T94WaLasaZ2I6pIge4s"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#include <PubSubClient.h>

// ================== WIFI ==================
char ssid[] = "Haru";
char pass[] = "Haruki12";

// ================== MQTT ==================
WiFiClient espClient;
PubSubClient mqttClient(espClient);
const char* mqtt_server = "10.226.199.65";   // <-- CHANGE THIS

// ================== PINS ==================
const int pirPin = 34;
const int ldrPin = 35;

// ================== VIRTUAL PINS ==================
#define V_SELECT_STUD   V2
#define V_ACTIVE_STUD   V3
#define V_STUDENT_PIR   V5
#define V_PROJECTOR     V6
#define V_PRES_TIME     V10
#define V_ATTENDANCE    V12
#define V_SCORE         V15
#define V_REMARKS       V16
#define V_SAVE          V17
#define V_SAVED_SCORE   V18
#define V_SAVED_REMARKS V19

WebServer server(80);
BlynkTimer timer;

// ================== STUDENT DATA ==================
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

unsigned long presStart = 0;
unsigned long presenceStart = 0;
unsigned long lastPirTrigger = 0;

int ldrThreshold = 3000;

// ================== MQTT CONNECT ==================
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32_SmartClass")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// ================== SEND DATA TO NODE-RED ==================
void sendToNodeRED(StudentData s) {
  String payload = "{";
  payload += "\"student\":\"" + s.name + "\",";
  payload += "\"attendance\":" + String(s.presencePercent) + ",";
  payload += "\"presentation\":" + String(s.presPercent) + ",";
  payload += "\"score\":" + String(s.score) + ",";
  payload += "\"remarks\":\"" + s.remarks + "\"";
  payload += "}";

  mqttClient.publish("smartclassroom/student", payload.c_str());
  Serial.println("Sent to Node-RED:");
  Serial.println(payload);
}

// ================== RESET ==================
void resetWidgets() {
  presStart = 0;
  presenceStart = 0;
  lastPirTrigger = 0;
  sessionActive = true;

  Blynk.virtualWrite(V_STUDENT_PIR, 0);
  Blynk.virtualWrite(V_PROJECTOR, 0);
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
  bool projectorOn = (ldr > ldrThreshold);

  if (pir == HIGH && millis() - lastPirTrigger > 500) {
    lastPirTrigger = millis();
    if (presenceStart == 0) presenceStart = millis();
  }

  if (projectorOn && presStart == 0) presStart = millis();

  int presencePercent = 0;
  int presPercent = 0;

  if (presenceStart > 0 && pir == HIGH) {
    presencePercent = map((millis() - presenceStart) / 1000, 0, 600, 0, 100);
    if (presencePercent > 100) presencePercent = 100;
  }

  if (presStart > 0 && projectorOn) {
    presPercent = map((millis() - presStart) / 1000, 0, 600, 0, 100);
    if (presPercent > 100) presPercent = 100;
  }

  Blynk.virtualWrite(V_STUDENT_PIR, pir * 100);
  Blynk.virtualWrite(V_PROJECTOR, projectorOn ? 255 : 0);
  Blynk.virtualWrite(V_PRES_TIME, presPercent);
  Blynk.virtualWrite(V_ATTENDANCE, presencePercent);

  if (currentStudent >= 0) {
    students[currentStudent].presencePercent = presencePercent;
    students[currentStudent].presPercent = presPercent;
  }
}

// ================== BLYNK ==================
BLYNK_WRITE(V_SELECT_STUD) {
  currentStudent = param.asInt() - 1;
  if (currentStudent < 0 || currentStudent >= STUDENT_COUNT) return;
  Blynk.virtualWrite(V_ACTIVE_STUD, students[currentStudent].name);
  resetWidgets();
}

BLYNK_WRITE(V_SCORE) {
  if (currentStudent >= 0) students[currentStudent].score = param.asInt();
}

BLYNK_WRITE(V_REMARKS) {
  if (currentStudent >= 0) students[currentStudent].remarks = param.asString();
}

BLYNK_WRITE(V_SAVE) {
  if (param.asInt() == 1 && currentStudent >= 0) {
    Blynk.virtualWrite(V_SAVED_SCORE, students[currentStudent].score);
    Blynk.virtualWrite(V_SAVED_REMARKS, students[currentStudent].remarks);

    sendToNodeRED(students[currentStudent]);   // 🔥 NODE-RED STORAGE

    sessionActive = false;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  mqttClient.setServer(mqtt_server, 1883);
  connectMQTT();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, readSensors);

  server.begin();
}

// ================== LOOP ==================
void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  Blynk.run();
  timer.run();
}
