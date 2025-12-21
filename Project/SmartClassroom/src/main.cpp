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
  int presPercent;        // Presentation timing %
  int presencePercent;    // Attendance %
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

// ================== RESET WIDGETS ==================
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

// ================== ENGAGEMENT CALCULATION ==================
int calculateEngagement(StudentData s){
  int baseScore = (s.presencePercent * 0.4) + (s.presPercent * 0.6);
  if(s.remarks.indexOf("participat") >= 0) baseScore += 10;
  if(s.presencePercent < 50) baseScore -= 15;
  if(baseScore < 0) baseScore = 0;
  if(baseScore > 100) baseScore = 100;
  return baseScore;
}

// ================== SENSOR LOOP ==================
void readSensors() {
  if (!sessionActive) return;

  int pir = digitalRead(pirPin);
  int ldr = analogRead(ldrPin);
  bool projectorOn = (ldr > ldrThreshold);

  if (pir == HIGH && (millis() - lastPirTrigger) > 500) {
    lastPirTrigger = millis();
    if (presenceStart == 0) presenceStart = millis();
  }

  if (projectorOn && presStart == 0) presStart = millis();

  int presencePercent = 0;
  int presPercent = 0;

  if (presenceStart > 0 && pir == HIGH) {
    presencePercent = map((millis() - presenceStart)/1000, 0, 600, 0, 100);
    if(presencePercent > 100) presencePercent = 100;
  }

  if (presStart > 0 && projectorOn) {
    presPercent = map((millis() - presStart)/1000, 0, 600, 0, 100);
    if(presPercent > 100) presPercent = 100;
  }

  Blynk.virtualWrite(V_STUDENT_PIR, pir * 100);
  Blynk.virtualWrite(V_PROJECTOR, projectorOn ? 255 : 0);
  Blynk.virtualWrite(V_PRES_TIME, presPercent);
  Blynk.virtualWrite(V_ATTENDANCE, presencePercent);

  if(currentStudent >=0 && currentStudent < STUDENT_COUNT){
    students[currentStudent].presencePercent = presencePercent;
    students[currentStudent].presPercent = presPercent;
  }
}

// ================== SELECT STUDENT ==================
BLYNK_WRITE(V_SELECT_STUD){
  currentStudent = param.asInt() - 1;
  if(currentStudent<0 || currentStudent>=STUDENT_COUNT) return;
  Blynk.virtualWrite(V_ACTIVE_STUD, students[currentStudent].name);
  resetWidgets();
}

// ================== SCORE & REMARKS ==================
BLYNK_WRITE(V_SCORE){
  if(currentStudent>=0) students[currentStudent].score = param.asInt();
}

BLYNK_WRITE(V_REMARKS){
  if(currentStudent>=0) students[currentStudent].remarks = param.asString();
}

// ================== SAVE ==================
BLYNK_WRITE(V_SAVE){
  int state = param.asInt();
  if(state==1 && currentStudent>=0){
    Blynk.virtualWrite(V_SAVED_SCORE, students[currentStudent].score);
    Blynk.virtualWrite(V_SAVED_REMARKS, students[currentStudent].remarks);
    sessionActive = false;
  }
}

// ================== FEEDBACK ==================
String generateFeedback(StudentData s) {
  String feedback = "<ul class='feedback-list'>";
  
  // Attendance feedback
  if(s.presencePercent >= 15) {
    feedback += "<li> Excellent class participation!</li>";
  } else if(s.presencePercent >= 10) {
    feedback += "<li> Good engagement, try to participate more</li>";
  } else {
    feedback += "<li> Needs to increase participation</li>";
  }

  // Presentation feedback
  if(s.presPercent >= 15) {
    feedback += "<li> Outstanding presentation skills</li>";
  } else if(s.presPercent >= 10) {
    feedback += "<li> Solid presentation delivery</li>";
  } else if(s.presPercent > 0) {
    feedback += "<li> Practice presentation timing</li>";
  }


  // Custom remarks
  if(s.remarks.length() > 0) {
    feedback += "<li> Teacher note: " + s.remarks + "</li>";
  }
  
  feedback += "</ul>";
  return feedback;
}


// ================== WEB PAGE ==================
void handleRoot(){
  String html = "<!DOCTYPE html><html><head><title>Smart Classroom Dashboard</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<style>"
        "body{font-family:'Segoe UI',sans-serif;background:#f8f9fa;color:#343a40;}"
        "h1,h2,h3{text-align:center;color:#2c3e50;}"
        "table{border-collapse:collapse;width:95%;margin:25px auto;box-shadow:0 0 10px rgba(0,0,0,0.05);}"
        "th,td{border:1px solid #dee2e6;padding:12px;text-align:center;}"
        "th{background:#3498db;color:white;font-weight:600;}"
        ".high{background:#d5f5e3;color:#1d8348;}"  // Green
        ".medium{background:#fcf3cf;color:#7d6608;}" // Yellow
        ".low{background:#ffffff;color:#e74c3c;}"    // White bg with red text
        ".feedback-list {text-align:left; padding-left:20px;}"
        ".feedback-list li {margin:8px 0;}"
        "</style>";
  
  html += "<h1>Smart Classroom Dashboard</h1>";
  
  // Table: Student Overview
  html += "<h2>Student Overview</h2><table>";
  html += "<tr><th>Name</th><th>Attendance %</th><th>Presentation %</th><th>Score</th><th>Remarks</th><th>Feedback</th></tr>";
  for(int i=0;i<STUDENT_COUNT;i++){
    html += "<tr>";
    html += "<td>" + students[i].name + "</td>";
    html += "<td class='" + String(students[i].presencePercent<50?"low":(students[i].presencePercent<80?"medium":"high")) + "'>" + String(students[i].presencePercent) + "%</td>";
    html += "<td class='" + String(students[i].presPercent<50?"low":(students[i].presPercent<80?"medium":"high")) + "'>" + String(students[i].presPercent) + "%</td>";
    html += "<td>" + String(students[i].score) + "</td>";
    html += "<td>" + students[i].remarks + "</td>";
    html += "<td>" + generateFeedback(students[i]) + "</td>";
    html += "</tr>";
  }
  html += "</table>";

  // Determine best/worst student
  int bestIndex=0,worstIndex=0;
  int bestEng=0,worstEng=1000;
  for(int i=0;i<STUDENT_COUNT;i++){
    int eng = students[i].presencePercent + students[i].presPercent;
    if(eng > bestEng || (eng==bestEng && students[i].score>students[bestIndex].score)) { bestEng=eng; bestIndex=i; }
    if(eng < worstEng || (eng==worstEng && students[i].score<students[worstIndex].score)) { worstEng=eng; worstIndex=i; }
  }

  html += "<h2>Teacher Notes</h2>";
  html += "<p><b>Top Performer:</b> " + students[bestIndex].name + " (Score: " + String(students[bestIndex].score) + ", Engagement: " + String(students[bestIndex].presencePercent+students[bestIndex].presPercent) + ")</p>";
  html += "<p><b>Student to Focus:</b> " + students[worstIndex].name + " (Score: " + String(students[worstIndex].score) + ", Engagement: " + String(students[worstIndex].presencePercent+students[worstIndex].presPercent) + ")</p>";

  // Chart
  // Replace chart code in handleRoot()
html += "<h2>Student Engagement Comparison</h2>";
html += "<div style='max-width:800px;margin:0 auto;'>";
html += "<canvas id='engagementChart' height='350'></canvas>";
html += "</div>";

html += "<script>"
        "const ctx = document.getElementById('engagementChart').getContext('2d');"
        "new Chart(ctx, {"
        "  type: 'bar',"
        "  data: {"
        "    labels: [";

for(int i=0; i<STUDENT_COUNT; i++) {
  html += "'" + students[i].name + "'";
  if(i < STUDENT_COUNT-1) html += ",";
}

html += "],"
        "    datasets: [{"
        "      label: 'Engagement Score',"
        "      data: [";

for(int i=0; i<STUDENT_COUNT; i++) {
  int engagement = students[i].presencePercent + students[i].presPercent;
  html += String(engagement);
  if(i < STUDENT_COUNT-1) html += ",";
}

html += "],"
        "      backgroundColor: ['#3498db','#2ecc71','#9b59b6','#e67e22'],"
        "      borderColor: ['#2980b9','#27ae60','#8e44ad','#d35400'],"
        "      borderWidth: 2"
        "    }]"
        "  },"
        "  options: {"
        "    responsive: true,"
        "    plugins: {"
        "      legend: {display: false},"
        "      tooltip: {padding: 10, backgroundColor: 'rgba(0,0,0,0.7)'}"
        "    },"
        "    scales: {"
        "      y: {"
        "        beginAtZero: true,"
        "        max: 40,"
        "        title: {display: true, text: 'Engagement Score', padding: 15}"
        "      },"
        "      x: {"
        "        grid: {display: false},"
        "        title: {display: true, text: 'Students', padding: 15}"
        "      }"
        "    }"
        "  }"
        "});"
        "</script>";

  server.send(200,"text/html",html);
}

// ================== SETUP ==================
void setup(){
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, readSensors);

  WiFi.begin(ssid, pass);
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println(""); Serial.print("WiFi connected! IP: "); Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started.");
}

// ================== LOOP ==================
void loop(){
  Blynk.run();
  timer.run();
  server.handleClient();
}
