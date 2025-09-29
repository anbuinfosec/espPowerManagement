#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#undef CLOSED
#include <ESPAsyncTCP.h>
#else
#include <WiFi.h>
#include <AsyncTCP.h>
#endif
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
#define LOG_FILE "/powerlog.json"
#define HISTORY_DAYS 14
#define AUTOSAVE_INTERVAL 300000UL
#define LED_PIN LED_BUILTIN
AsyncWebServer server(80);
unsigned long bootMillis;
unsigned long lastSaveMillis;
bool ledState = false;
struct DayStats {
  uint32_t uptime_sec;
  uint32_t downtime_sec;
  uint16_t outages;
  uint32_t longest_outage_sec;
};
struct PowerEvent {
  time_t on_time;
  time_t off_time;
  uint32_t duration_sec;
};
struct PowerLog {
  time_t last_on;
  time_t last_off;
  uint32_t last_downtime_sec;
  std::vector<DayStats> days;
  std::vector<PowerEvent> events;
};
PowerLog logData;
time_t getNow() {
  return logData.last_on + (millis() / 1000);
}
String timeStr(uint32_t sec) {
  char buf[16];
  uint16_t h = sec / 3600;
  uint8_t m = (sec % 3600) / 60;
  snprintf(buf, sizeof(buf), "%02uh %02um", h, m);
  return String(buf);
}
void saveLog() {
  File f = LittleFS.open(LOG_FILE, "w");
  if (!f) return;
  DynamicJsonDocument doc(8192);
  doc["last_on"] = logData.last_on;
  doc["last_off"] = logData.last_off;
  doc["last_downtime_sec"] = logData.last_downtime_sec;
  JsonArray days = doc.createNestedArray("days");
  for (auto& d : logData.days) {
    JsonObject o = days.createNestedObject();
    o["uptime"] = d.uptime_sec;
    o["downtime"] = d.downtime_sec;
    o["outages"] = d.outages;
    o["longest"] = d.longest_outage_sec;
  }
  JsonArray events = doc.createNestedArray("events");
  for (auto& e : logData.events) {
    JsonObject o = events.createNestedObject();
    o["on"] = e.on_time;
    o["off"] = e.off_time;
    o["duration"] = e.duration_sec;
  }
  serializeJson(doc, f);
  f.close();
}
void loadLog() {
  if (!LittleFS.exists(LOG_FILE)) {
    logData.days.resize(HISTORY_DAYS);
    return;
  }
  File f = LittleFS.open(LOG_FILE, "r");
  if (!f) return;
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, f);
  if (err) {
    logData.days.resize(HISTORY_DAYS);
    return;
  }
  logData.last_on = doc["last_on"] | 0;
  logData.last_off = doc["last_off"] | 0;
  logData.last_downtime_sec = doc["last_downtime_sec"] | 0;
  logData.days.clear();
  for (JsonObject d : doc["days"].as<JsonArray>()) {
    DayStats s;
    s.uptime_sec = d["uptime"] | 0;
    s.downtime_sec = d["downtime"] | 0;
    s.outages = d["outages"] | 0;
    s.longest_outage_sec = d["longest"] | 0;
    logData.days.push_back(s);
  }
  logData.events.clear();
  for (JsonObject e : doc["events"].as<JsonArray>()) {
    PowerEvent ev;
    ev.on_time = e["on"] | 0;
    ev.off_time = e["off"] | 0;
    ev.duration_sec = e["duration"] | 0;
    logData.events.push_back(ev);
  }
  f.close();
}
void recordPowerOn() {
  time_t now = time(nullptr);
  logData.last_on = now;
  if (logData.last_off > 0) {
    uint32_t dt = now - logData.last_off;
    logData.last_downtime_sec = dt;
    if (!logData.days.size()) logData.days.resize(HISTORY_DAYS);
    logData.days[0].downtime_sec += dt;
    logData.days[0].outages++;
    if (dt > logData.days[0].longest_outage_sec)
      logData.days[0].longest_outage_sec = dt;
    PowerEvent ev;
    ev.on_time = now;
    ev.off_time = logData.last_off;
    ev.duration_sec = dt;
    logData.events.insert(logData.events.begin(), ev);
    if (logData.events.size() > 20) logData.events.pop_back();
  }
  saveLog();
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
}
void recordPowerOff() {
  time_t now = getNow();
  logData.last_off = now;
  saveLog();
}
void updateUptime() {
  time_t now = getNow();
  if (!logData.days.size()) logData.days.resize(HISTORY_DAYS);
  logData.days[0].uptime_sec = now - logData.last_on;
}
void rotateDay() {
  logData.days.insert(logData.days.begin() + 1, DayStats());
  if (logData.days.size() > HISTORY_DAYS)
    logData.days.resize(HISTORY_DAYS);
  saveLog();
}
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width,initial-scale=1'>
  <title>ESP Power Uptime Monitor</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;700&display=swap" rel="stylesheet">
  <style>
    body { font-family: 'Inter',sans-serif; background: #f4f6fb; margin:0; padding:0; }
    .container { max-width: 700px; margin: 2em auto; background: #fff; border-radius: 18px; box-shadow: 0 4px 24px #0001; padding: 2em; }
    h1 { color: #1a237e; font-size: 2.2em; margin-bottom: 0.2em; }
    .live { display: flex; align-items: center; gap: 1em; margin-bottom: 1em; }
    .live .dot { width: 16px; height: 16px; border-radius: 50%; background: #43a047; box-shadow: 0 0 8px #43a04788; }
    .stat { font-size: 1.1em; margin: 0.3em 0; }
    .summary { margin: 1.5em 0 2em 0; }
    .charts { display: flex; flex-wrap: wrap; gap: 2em; justify-content: center; }
    .chart-container { width: 320px; background: #f8fafc; border-radius: 12px; padding: 1em; box-shadow: 0 2px 8px #0001; }
    .controls { margin-top: 2em; text-align: right; }
    button { background: #1a237e; color: #fff; border: none; border-radius: 6px; padding: 0.7em 1.5em; font-size: 1em; margin-left: 0.5em; cursor: pointer; transition: background 0.2s; }
    button:hover { background: #3949ab; }
    @media (max-width: 800px) { .container { padding: 1em; } .charts { flex-direction: column; align-items: center; } }
  </style>
</head>
<body>
  <div class="container">
    <h1>⚡ ESP Power Uptime Monitor</h1>
    <div class="live"><div class="dot"></div><span id="since"></span></div>
    <div class="stat"><b>Current Session:</b> <span id="session"></span></div>
    <div class="stat"><b>Last Downtime:</b> <span id="last_downtime"></span></div>
    <div class="stat"><b>Outages Today:</b> <span id="outages"></span></div>
    <div class="summary">
      <h3>Today’s Summary</h3>
      <div class="stat"><b>Uptime:</b> <span id="today_uptime"></span></div>
      <div class="stat"><b>Downtime:</b> <span id="today_downtime"></span></div>
      <div class="stat"><b>Longest Outage:</b> <span id="longest_outage"></span></div>
    </div>
    <div class="charts">
      <div class="chart-container"><canvas id="barChart"></canvas></div>
      <div class="chart-container"><canvas id="lineChart"></canvas></div>
    </div>
    <div class="controls">
        <button onclick="downloadLogs()">Download Logs</button>
        <button onclick="clearData()">Clear Data</button>
      </div>
      <div style="margin-top:2em;text-align:center;color:#888;font-size:0.98em;">
        Made with ❤️ by <a href="https://github.com/anbuinfosec" target="_blank" style="color:#3949ab;text-decoration:none;font-weight:600;">@anbuinfosec</a>
      </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
    async function fetchStatus() {
      let r = await fetch('/status');
      let d = await r.json();
      document.getElementById('since').textContent = `Power ON (since ${d.since})`;
      document.getElementById('session').textContent = d.session;
      document.getElementById('last_downtime').textContent = d.last_downtime;
      document.getElementById('outages').textContent = d.outages;
      document.getElementById('today_uptime').textContent = d.today_uptime;
      document.getElementById('today_downtime').textContent = d.today_downtime;
      document.getElementById('longest_outage').textContent = d.longest_outage;
      updateCharts(d);
    }
    function updateCharts(d) {
      let bar = window.barChart;
      if (!bar) {
        let ctx = document.getElementById('barChart').getContext('2d');
        window.barChart = new Chart(ctx, {
          type: 'bar',
          data: {
            labels: Array.from({length:24},(_,i)=>i+":00"),
            datasets: [{label:"Uptime (min)",data:d.hourly_uptime,backgroundColor:'#43a047'}]
          },
          options: {scales:{y:{beginAtZero:true,max:60}},plugins:{legend:{display:false}}}
        });
      } else {
        bar.data.datasets[0].data = d.hourly_uptime;
        bar.update();
      }
      let line = window.lineChart;
      if (!line) {
        let ctx = document.getElementById('lineChart').getContext('2d');
        window.lineChart = new Chart(ctx, {
          type: 'line',
          data: {
            labels: d.weekdays,
            datasets: [{label:"Uptime (h)",data:d.weekly_uptime,fill:true,borderColor:'#1a237e',backgroundColor:'#1a237e22',pointRadius:4,pointBackgroundColor:'#3949ab'}]
          },
          options: {scales:{y:{beginAtZero:true,max:24}},plugins:{legend:{display:false}}}
        });
      } else {
        line.data.labels = d.weekdays;
        line.data.datasets[0].data = d.weekly_uptime;
        line.update();
      }
    }
    function downloadLogs() { window.location='/logs'; }
    async function clearData() {
      if (confirm("Clear all data?")) {
        await fetch('/clear', {method:'POST'});
        location.reload();
      }
    }
    fetchStatus();
    setInterval(fetchStatus, 10000);
  </script>
</body>
</html>
)rawliteral";
void handleStatus(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(2048);
  time_t now = getNow();
  doc["since"] = String(ctime(&logData.last_on)).substring(0,16);
  doc["session"] = timeStr(now - logData.last_on);
  doc["last_downtime"] = timeStr(logData.last_downtime_sec);
  doc["outages"] = logData.days[0].outages;
  doc["today_uptime"] = timeStr(logData.days[0].uptime_sec);
  doc["today_downtime"] = timeStr(logData.days[0].downtime_sec);
  doc["longest_outage"] = timeStr(logData.days[0].longest_outage_sec);
  JsonArray hourly = doc.createNestedArray("hourly_uptime");
  uint32_t up = logData.days[0].uptime_sec / 60;
  for (int i=0; i<24; ++i) hourly.add(up/24);
  JsonArray weekdays = doc.createNestedArray("weekdays");
  JsonArray weekly = doc.createNestedArray("weekly_uptime");
  for (int i=0; i<7 && i<logData.days.size(); ++i) {
    weekdays.add("Day-"+String(i+1));
    weekly.add(logData.days[i].uptime_sec/3600.0);
  }
  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}
void handleLogs(AsyncWebServerRequest *request) {
  if (!LittleFS.exists(LOG_FILE)) {
    request->send(404, "text/plain", "No log");
    return;
  }
  request->send(LittleFS, LOG_FILE, "application/json", true);
}
void handleClear(AsyncWebServerRequest *request) {
  LittleFS.remove(LOG_FILE);
  logData = PowerLog();
  logData.days.resize(HISTORY_DAYS);
  request->send(200, "text/plain", "Cleared");
}
void setupWeb() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(200, "text/html", htmlPage);
  });
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/logs", HTTP_GET, handleLogs);
  server.on("/clear", HTTP_POST, handleClear);
  server.begin();
}
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  Serial.begin(115200);
  delay(100);
  LittleFS.begin();
  loadLog();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  digitalWrite(LED_PIN, HIGH);
  configTime(0,0,"pool.ntp.org","time.nist.gov");
  time_t now = time(nullptr);
  if (now < 100000) now = millis()/1000 + (logData.last_on ? logData.last_on : 0);
  recordPowerOn();
  setupWeb();
  bootMillis = millis();
  lastSaveMillis = millis();
}
void loop() {
  updateUptime();
  if (millis() - lastSaveMillis > AUTOSAVE_INTERVAL) {
    saveLog();
    lastSaveMillis = millis();
  }
  static int lastDay = -1;
  time_t now = getNow();
  struct tm *tmnow = localtime(&now);
  if (tmnow->tm_mday != lastDay) {
    rotateDay();
    lastDay = tmnow->tm_mday;
  }
  delay(10000);
}