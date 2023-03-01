#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define PROXIMITY_PIN 4
#define PIR_PIN 14
#define PIEZO_PIN 5

const char* ssid = "";
const char* password = "";

String motionDetected[10];
uint8_t m_index = 0;
String uptime;
String indexHTML1 = "<!DOCTYPE html><html><head><title>sahil project1</title></head><body><h1>sahil Project1</h1>";
String indexHTML2 = "</body><script>setTimeout(function () {window.location.reload(1);}, 1500);</script></html>";
bool detectionFlag = false;

ESP8266WebServer server(80);

void setup() {
  pinMode(PROXIMITY_PIN, INPUT);
  pinMode(PIEZO_PIN, OUTPUT);
  Serial.begin(115200);

  digitalWrite(PIEZO_PIN, LOW);

  WiFi.begin(ssid, password);
  while (!WiFi.isConnected()) {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED) {
    if (MDNS.begin("sahilweb")) {
      Serial.println("MDNS started at http://sahilweb.local/");
    } else {
      Serial.println("MDNS error!");
    }
  }
  server.on("/", HTTP_GET, handleRoot); 
  server.begin();
}

void loop() {
  const int prox_reading = digitalRead(PROXIMITY_PIN);
  const int total_time_seconds = millis() / 1000;
  const int current_time_hours = total_time_seconds / 3600;
  const int current_time_minutes = (total_time_seconds % 3600) / 60;
  const int current_time_seconds = total_time_seconds % 60;

  String str_time_s = String(current_time_seconds);
  String str_time_m, str_time_h;
  if (current_time_minutes) {
    str_time_m = String(current_time_minutes);
  } else {
    str_time_m = "00";
  }
  if (current_time_hours) {
    str_time_h = String(current_time_hours);
  } else {
    str_time_h = "00";
  }
  uptime = str_time_h + ":" + str_time_m + ":" + str_time_s;

  if (m_index >= 9) {
    m_index = 0;
  }

  if (!prox_reading) {
    digitalWrite(PIEZO_PIN, HIGH);
    motionDetected[m_index] = uptime;
    m_index++;
    detectionFlag = true;
  } else {
    digitalWrite(PIEZO_PIN, LOW);
    detectionFlag = false;
  }

  for (size_t i = 0; i < 10; i++) {
    if (motionDetected[i] != NULL)
      Serial.println(motionDetected[i]);
  }

  server.handleClient();

  delay(1000);
}

void handleRoot() {
  String motion = detectionFlag ? "MOTION DETECTED" : "MOTION NOT DETECTED";
  String indexdotHTML = indexHTML1 + "<h2>Server uptime: " + uptime + "</h2>" + "<h2>" + motion
   + "<h2>Security logs:</h2><hr/>"
   "<ul>" + getMotionLogs() + "</ul>" + "</h2>" + indexHTML2;
  server.send(200, "text/html", indexdotHTML);
}

String getMotionLogs() {
  String motionLogs = "";
  for (int i = 0; i < 10; i++) {
    if (motionDetected[i] != NULL) {
      motionLogs += "<li>Motion detected in close proximity at " + motionDetected[i] + "</li>";
    }
  }
  return motionLogs;
}
