#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <time.h>
#include <vector>

ESP8266WebServer server(80);

#define LED_PIN 2
#define EEPROM_SIZE 512
#define WIFI_SSID_ADDR 0
#define WIFI_PASS_ADDR 64
#define AP_SSID_ADDR 128
#define AP_PASS_ADDR 192
#define FLAG_ADDR 250
#define LAST_RESET_ADDR 256

String wifiSSID="",wifiPASS="";
String apSSID="ESP8266_PowerLog",apPASS="12345678";
String logFile="/power_log.txt";
String lastOnFile="/last_on.txt";
time_t bootTime=0;

struct LogEntry{
  String type;
  time_t timestamp;
  time_t duration;
};

struct Stats{
  time_t todayOff;
  time_t todayOn;
  time_t last7Off;
  time_t last7On;
  time_t last15Off;
  time_t last15On;
  time_t monthOff;
  time_t monthOn;
  Stats():todayOff(0),todayOn(0),last7Off(0),last7On(0),last15Off(0),last15On(0),monthOff(0),monthOn(0){}
};

void saveString(int addr,String s,int maxLen){
  EEPROM.begin(EEPROM_SIZE);
  for(int i=0;i<maxLen;i++)EEPROM.write(addr+i,(i<s.length())?s[i]:0);
  EEPROM.commit();
}

String readString(int addr,int maxLen){
  EEPROM.begin(EEPROM_SIZE);
  char buf[maxLen+1];
  for(int i=0;i<maxLen;i++)buf[i]=EEPROM.read(addr+i);
  buf[maxLen]=0;
  return String(buf);
}

void saveLastReset(time_t t){
  EEPROM.begin(EEPROM_SIZE);
  for(int i=0;i<4;i++)EEPROM.write(LAST_RESET_ADDR+i,(t>>(8*i))&0xFF);
  EEPROM.commit();
}

time_t readLastReset(){
  EEPROM.begin(EEPROM_SIZE);
  time_t t=0;
  for(int i=0;i<4;i++)t|=((time_t)EEPROM.read(LAST_RESET_ADDR+i))<<(8*i);
  return t;
}

void saveConfig(){
  saveString(WIFI_SSID_ADDR,wifiSSID,64);
  saveString(WIFI_PASS_ADDR,wifiPASS,64);
  saveString(AP_SSID_ADDR,apSSID,64);
  saveString(AP_PASS_ADDR,apPASS,64);
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(FLAG_ADDR,1);
  EEPROM.commit();
}

void loadConfig(){
  EEPROM.begin(EEPROM_SIZE);
  if(EEPROM.read(FLAG_ADDR)!=1)return;
  wifiSSID=readString(WIFI_SSID_ADDR,64);
  wifiPASS=readString(WIFI_PASS_ADDR,64);
  String tmpAP=readString(AP_SSID_ADDR,64);
  String tmpPass=readString(AP_PASS_ADDR,64);
  
  // Trim whitespace and null characters
  wifiSSID.trim();
  wifiPASS.trim();
  tmpAP.trim();
  tmpPass.trim();
  
  if(tmpAP.length()>0)apSSID=tmpAP;
  if(tmpPass.length()>0)apPASS=tmpPass;
}

String getTimeString(time_t t){
  if(t<=0)return "N/A";
  struct tm*tmstruct=localtime(&t);
  char buf[32];
  strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",tmstruct);
  return String(buf);
}

String formatDuration(time_t s){
  if(s<=0)return "0s";
  int days=s/86400;s%=86400;
  int h=s/3600;s%=3600;
  int m=s/60;int sec=s%60;
  String result="";
  if(days>0)result+=String(days)+"d ";
  if(h>0||days>0)result+=String(h)+"h ";
  if(m>0||h>0||days>0)result+=String(m)+"m ";
  result+=String(sec)+"s";
  return result;
}

void logEvent(String type,time_t t,time_t dur=0){
  File f=SPIFFS.open(logFile,"a");
  if(f){
    f.println(type+" "+String(t)+" "+String(dur));
    f.close();
  }
}

void checkMonthlyReset(){
  time_t now=time(nullptr);
  time_t lastReset=readLastReset();
  if(lastReset<=0){
    saveLastReset(now);
    return;
  }
  struct tm*tmNow=localtime(&now);
  struct tm*tmLast=localtime(&lastReset);
  if(tmNow->tm_year!=tmLast->tm_year||tmNow->tm_mon!=tmLast->tm_mon){
    SPIFFS.remove(logFile);
    SPIFFS.remove(lastOnFile);
    saveLastReset(now);
    Serial.println("Monthly reset performed!");
  }
}

std::vector<LogEntry>parseLog(){
  std::vector<LogEntry>entries;
  File f=SPIFFS.open(logFile,"r");
  if(!f)return entries;
  while(f.available()){
    String line=f.readStringUntil('\n');
    line.trim();
    if(line.length()==0)continue;
    LogEntry e;
    int sp1=line.indexOf(' ');
    int sp2=line.indexOf(' ',sp1+1);
    e.type=line.substring(0,sp1);
    e.timestamp=line.substring(sp1+1,sp2).toInt();
    e.duration=(sp2>0)?line.substring(sp2+1).toInt():0;
    entries.push_back(e);
  }
  f.close();
  return entries;
}

Stats calculateStats(){
  Stats s;
  time_t now=time(nullptr);
  struct tm*tmNow=localtime(&now);
  time_t todayStart=now-(tmNow->tm_hour*3600+tmNow->tm_min*60+tmNow->tm_sec);
  time_t day7=now-7*86400;
  time_t day15=now-15*86400;
  time_t monthStart=now-(tmNow->tm_mday-1)*86400-(tmNow->tm_hour*3600+tmNow->tm_min*60+tmNow->tm_sec);
  
  std::vector<LogEntry>entries=parseLog();
  for(size_t i=0;i<entries.size();i++){
    LogEntry e=entries[i];
    if(e.type=="[OFF]"){
      if(e.timestamp>=todayStart)s.todayOff+=e.duration;
      if(e.timestamp>=day7)s.last7Off+=e.duration;
      if(e.timestamp>=day15)s.last15Off+=e.duration;
      if(e.timestamp>=monthStart)s.monthOff+=e.duration;
    }else if(e.type=="[ON]"){
      if(e.timestamp>=todayStart)s.todayOn+=e.duration;
      if(e.timestamp>=day7)s.last7On+=e.duration;
      if(e.timestamp>=day15)s.last15On+=e.duration;
      if(e.timestamp>=monthStart)s.monthOn+=e.duration;
    }
  }
  return s;
}

String navbar(String active){
  String nav="<nav class='navbar navbar-expand-lg navbar-dark bg-dark'><div class='container-fluid'>"
  "<a class='navbar-brand' href='/'>⚡ ESP Power</a>"
  "<button class='navbar-toggler' type='button' data-bs-toggle='collapse' data-bs-target='#navbarNav' "
  "aria-controls='navbarNav' aria-expanded='false' aria-label='Toggle navigation'>"
  "<span class='navbar-toggler-icon'></span></button>"
  "<div class='collapse navbar-collapse' id='navbarNav'><ul class='navbar-nav ms-auto'>";
  nav+="<li class='nav-item'><a class='nav-link";
  if(active=="home")nav+=" active";
  nav+="' href='/'>History</a></li>";
  nav+="<li class='nav-item'><a class='nav-link";
  if(active=="stats")nav+=" active";
  nav+="' href='/stats'>Stats</a></li>";
  nav+="<li class='nav-item'><a class='nav-link";
  if(active=="config")nav+=" active";
  nav+="' href='/config'>Config</a></li>";
  nav+="</ul></div></div></nav>";
  return nav;
}

String pageHeader(String t,String active){
  String html="<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>"
  "<meta name='viewport' content='width=device-width,initial-scale=1.0,maximum-scale=5.0,user-scalable=yes'>"
  "<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css' rel='stylesheet' integrity='sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN' crossorigin='anonymous'>"
  "<title>"+t+"</title><style>body{padding-bottom:70px;background:#f8f9fa;}"
  ".card{border-radius:15px;box-shadow:0 3px 8px rgba(0,0,0,0.1);margin-bottom:1rem;}"
  "footer{position:fixed;bottom:0;width:100%;height:60px;line-height:60px;background:#f1f1f1;text-align:center;}"
  ".badge-on{background-color:#28a745!important;}.badge-off{background-color:#dc3545!important;}"
  ".navbar-brand{font-weight:bold;font-size:1.3rem;}"
  ".table-responsive{overflow-x:auto;-webkit-overflow-scrolling:touch;}"
  "@media(max-width:768px){body{padding-bottom:80px;}.card{margin:0.5rem;}.container{padding:0.5rem;}}"
  "</style></head><body>"+navbar(active)+"<div class='container mt-4'>";
  return html;
}

String pageFooter(){
  return "</div><footer class='text-center'>Made with ❤️ by "
  "<a href='https://github.com/anbuinfosec' target='_blank'>@anbuinfosec</a></footer>"
  "<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js' integrity='sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL' crossorigin='anonymous'></script></body></html>";
}

String getHTML(){
  std::vector<LogEntry>entries=parseLog();
  Stats s=calculateStats();
  time_t now=time(nullptr);
  
  String html=pageHeader("Power History","home");
  html+="<div class='card p-4 mb-3'><h3>⚡ Power History</h3>";
  html+="<div class='mb-3'><a href='/clear' class='btn btn-danger btn-sm'>Clear Logs</a></div>";
  
  html+="<div class='table-responsive'><table class='table table-bordered table-striped'><thead class='table-dark'>"
  "<tr><th>#</th><th>Event</th><th>Time</th><th>Duration</th></tr></thead><tbody>";
  
  for(size_t i=0;i<entries.size();i++){
    LogEntry e=entries[i];
    String badge=(e.type=="[ON]")?"<span class='badge badge-on'>ON</span>":"<span class='badge badge-off'>OFF</span>";
    String dur=(e.duration>0)?formatDuration(e.duration):"-";
    html+="<tr><td>"+String(i+1)+"</td><td>"+badge+"</td><td>"+getTimeString(e.timestamp)+"</td><td>"+dur+"</td></tr>";
  }
  html+="</tbody></table></div></div>";
  
  html+="<div class='card p-4 mb-3'><h4>📊 Power Statistics</h4><div class='table-responsive'><table class='table table-sm'>";
  html+="<tr><th>Period</th><th>Power OFF Time</th><th>Power ON Time</th></tr>";
  html+="<tr><td><strong>Today</strong></td><td class='text-danger'>"+formatDuration(s.todayOff)+"</td><td class='text-success'>"+formatDuration(now-bootTime)+"</td></tr>";
  html+="<tr><td><strong>Last 7 Days</strong></td><td class='text-danger'>"+formatDuration(s.last7Off)+"</td><td class='text-success'>"+formatDuration(s.last7On)+"</td></tr>";
  html+="<tr><td><strong>Last 15 Days</strong></td><td class='text-danger'>"+formatDuration(s.last15Off)+"</td><td class='text-success'>"+formatDuration(s.last15On)+"</td></tr>";
  html+="<tr><td><strong>This Month</strong></td><td class='text-danger'>"+formatDuration(s.monthOff)+"</td><td class='text-success'>"+formatDuration(s.monthOn)+"</td></tr>";
  html+="</table></div><small class='text-muted'>Current uptime: "+formatDuration(now-bootTime)+"</small></div>";
  
  html+="<p class='text-muted text-center'><small>Updated: "+getTimeString(now)+"</small></p>";
  html+=pageFooter();
  return html;
}

String statsPage(){
  Stats s=calculateStats();
  String html=pageHeader("ESP Stats","stats");
  html+="<div class='card p-4 mb-3'><h3>📡 Device Statistics</h3>";
  html+="<div class='table-responsive'><table class='table table-bordered table-striped mt-3'>";
  html+="<tr><th>Chip ID</th><td>"+String(ESP.getChipId(),HEX)+"</td></tr>";
  html+="<tr><th>Flash Chip ID</th><td>"+String(ESP.getFlashChipId(),HEX)+"</td></tr>";
  html+="<tr><th>Flash Size</th><td>"+String(ESP.getFlashChipSize()/1024)+" KB</td></tr>";
  html+="<tr><th>Real Flash Size</th><td>"+String(ESP.getFlashChipRealSize()/1024)+" KB</td></tr>";
  html+="<tr><th>Free Heap</th><td>"+String(ESP.getFreeHeap())+" bytes</td></tr>";
  html+="<tr><th>CPU Frequency</th><td>"+String(ESP.getCpuFreqMHz())+" MHz</td></tr>";
  html+="<tr><th>SDK Version</th><td>"+String(ESP.getSdkVersion())+"</td></tr>";
  html+="<tr><th>Boot Version</th><td>"+String(ESP.getBootVersion())+"</td></tr>";
  html+="<tr><th>Boot Mode</th><td>"+String(ESP.getBootMode())+"</td></tr>";
  html+="<tr><th>Sketch Size</th><td>"+String(ESP.getSketchSize())+" bytes</td></tr>";
  html+="<tr><th>Free Sketch Space</th><td>"+String(ESP.getFreeSketchSpace())+" bytes</td></tr>";
  html+="<tr><th>Reset Reason</th><td>"+ESP.getResetReason()+"</td></tr>";
  html+="<tr><th>Reset Info</th><td>"+ESP.getResetInfo()+"</td></tr>";
  
  FSInfo fs;
  SPIFFS.info(fs);
  html+="<tr><th>SPIFFS Total</th><td>"+String(fs.totalBytes)+" bytes</td></tr>";
  html+="<tr><th>SPIFFS Used</th><td>"+String(fs.usedBytes)+" bytes</td></tr>";
  html+="<tr><th>SPIFFS Free</th><td>"+String(fs.totalBytes-fs.usedBytes)+" bytes</td></tr>";
  
  html+="<tr><th>WiFi Mode</th><td>";
  WiFiMode_t mode=WiFi.getMode();
  if(mode==WIFI_AP_STA)html+="Repeater (AP + STA)";
  else if(mode==WIFI_AP)html+="Access Point";
  else if(mode==WIFI_STA)html+="Station";
  else html+="Off";
  html+="</td></tr>";
  
  html+="<tr><th>AP SSID</th><td>"+String(apSSID)+"</td></tr>";
  html+="<tr><th>AP IP</th><td>"+WiFi.softAPIP().toString()+"</td></tr>";
  html+="<tr><th>AP MAC</th><td>"+WiFi.softAPmacAddress()+"</td></tr>";
  html+="<tr><th>AP Clients</th><td>"+String(WiFi.softAPgetStationNum())+"</td></tr>";
  
  html+="<tr><th>WiFi Status</th><td>";
  html+=(WiFi.status()==WL_CONNECTED)?"<span class='badge bg-success'>Connected</span>":"<span class='badge bg-secondary'>Disconnected</span>";
  html+="</td></tr>";
  if(WiFi.status()==WL_CONNECTED){
    html+="<tr><th>Connected To</th><td>"+WiFi.SSID()+"</td></tr>";
    html+="<tr><th>Station IP</th><td>"+WiFi.localIP().toString()+"</td></tr>";
    html+="<tr><th>Gateway</th><td>"+WiFi.gatewayIP().toString()+"</td></tr>";
    html+="<tr><th>Subnet Mask</th><td>"+WiFi.subnetMask().toString()+"</td></tr>";
    html+="<tr><th>DNS</th><td>"+WiFi.dnsIP().toString()+"</td></tr>";
    html+="<tr><th>Station MAC</th><td>"+WiFi.macAddress()+"</td></tr>";
    html+="<tr><th>RSSI</th><td>"+String(WiFi.RSSI())+" dBm</td></tr>";
  }
  
  html+="<tr><th>Current Time</th><td>"+getTimeString(time(nullptr))+"</td></tr>";
  html+="<tr><th>Uptime</th><td>"+formatDuration(millis()/1000)+"</td></tr>";
  html+="</table></div>";
  
  html+="<h5 class='mt-4'>Power Statistics</h5><ul class='list-group'>";
  html+="<li class='list-group-item'>Today: <span class='float-end'><strong class='text-danger'>OFF: "+formatDuration(s.todayOff)+"</strong> | <strong class='text-success'>ON: "+formatDuration(time(nullptr)-bootTime)+"</strong></span></li>";
  html+="<li class='list-group-item'>Last 7 days: <span class='float-end'><strong class='text-danger'>OFF: "+formatDuration(s.last7Off)+"</strong> | <strong class='text-success'>ON: "+formatDuration(s.last7On)+"</strong></span></li>";
  html+="<li class='list-group-item'>Last 15 days: <span class='float-end'><strong class='text-danger'>OFF: "+formatDuration(s.last15Off)+"</strong> | <strong class='text-success'>ON: "+formatDuration(s.last15On)+"</strong></span></li>";
  html+="<li class='list-group-item'>Last Month: <span class='float-end'><strong class='text-danger'>OFF: "+formatDuration(s.monthOff)+"</strong> | <strong class='text-success'>ON: "+formatDuration(s.monthOn)+"</strong></span></li>";
  html+="</ul><p class='mt-3 text-muted'><small>Data resets automatically after one month.</small></p>";
  html+="</div>"+pageFooter();
  return html;
}

String configPage(){
  String html=pageHeader("Configuration","config");
  html+="<div class='card p-4'><h3>⚙️ Wi-Fi & AP Configuration</h3>";
  html+="<form action='/save' method='POST' class='mt-3'>";
  html+="<div class='mb-3'><label class='form-label'>Wi-Fi SSID:</label>"
  "<input class='form-control' name='ssid' value='"+wifiSSID+"' placeholder='Your WiFi Network'></div>";
  html+="<div class='mb-3'><label class='form-label'>Wi-Fi Password:</label>"
  "<input class='form-control' type='password' name='pass' value='"+wifiPASS+"' placeholder='WiFi Password'></div>";
  html+="<div class='mb-3'><label class='form-label'>Fallback AP SSID:</label>"
  "<input class='form-control' name='apssid' value='"+apSSID+"' placeholder='ESP8266_PowerLog'></div>";
  html+="<div class='mb-3'><label class='form-label'>Fallback AP Password:</label>"
  "<input class='form-control' type='password' name='appass' value='"+apPASS+"' placeholder='Min 8 characters' minlength='8'></div>";
  html+="<button class='btn btn-success w-100 btn-lg'>💾 Save & Restart</button></form></div>"+pageFooter();
  return html;
}

void handleSave(){
  wifiSSID=server.arg("ssid");
  wifiPASS=server.arg("pass");
  apSSID=server.arg("apssid");
  apPASS=server.arg("appass");
  
  // Trim whitespace that can break WiFi connections
  wifiSSID.trim();
  wifiPASS.trim();
  apSSID.trim();
  apPASS.trim();
  
  if(apSSID.length()==0)apSSID="ESP8266_PowerLog";
  
  Serial.println("Saving WiFi config:");
  Serial.println("  SSID: "+wifiSSID+" (length: "+String(wifiSSID.length())+")");
  String passInfo=(wifiPASS.length()>0)?"[SET]":"[EMPTY]";
  Serial.println("  Password: "+passInfo+" (length: "+String(wifiPASS.length())+")");
  
  saveConfig();
  String html=pageHeader("Configuration Saved","");
  html+="<div class='card p-4 text-center'>"
  "<div class='alert alert-success'><h4>✓ Configuration Saved Successfully!</h4></div>"
  "<p class='lead'>Device is restarting...</p>"
  "<div class='spinner-border text-primary mt-3' role='status'><span class='visually-hidden'>Loading...</span></div>"
  "</div>"+pageFooter();
  server.send(200,"text/html",html);
  delay(2000);
  ESP.restart();
}

void handleRoot(){
  server.send(200,"text/html",getHTML());
}

void handleStats(){
  server.send(200,"text/html",statsPage());
}

void handleConfig(){
  server.send(200,"text/html",configPage());
}

void handleClear(){
  SPIFFS.remove(logFile);
  SPIFFS.remove(lastOnFile);
  String html=pageHeader("Logs Cleared","");
  html+="<div class='card p-4 text-center'>"
  "<div class='alert alert-warning'><h4>🗑️ All Logs Cleared!</h4></div>"
  "<p>Redirecting to home page...</p>"
  "<div class='spinner-border text-warning mt-3' role='status'><span class='visually-hidden'>Loading...</span></div>"
  "</div><meta http-equiv='refresh' content='2;url=/'>"+pageFooter();
  server.send(200,"text/html",html);
}

void setup(){
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,LOW);
  
  SPIFFS.begin();
  loadConfig();
  
  Serial.println("\n=== WiFi Configuration ===");
  Serial.println("SSID from EEPROM: "+wifiSSID+" (length: "+String(wifiSSID.length())+")");
  Serial.println("Password length: "+String(wifiPASS.length()));
  
  bool wifiConnected=false;
  if(wifiSSID.length()>0){
    Serial.println("\nStarting Repeater Mode (AP + STA)");
    WiFi.mode(WIFI_AP_STA); // Set mode FIRST
    
    // Start AP for repeater functionality
    Serial.println("Starting AP: "+apSSID);
    WiFi.softAP(apSSID.c_str(),apPASS.c_str());
    delay(100); // Give AP time to start
    Serial.println("AP IP: "+WiFi.softAPIP().toString());
    
    // Connect to WiFi as station
    Serial.println("\nConnecting to WiFi: '"+wifiSSID+"'");
    WiFi.begin(wifiSSID.c_str(),wifiPASS.c_str());
    unsigned long start=millis();
    int dots=0;
    while(WiFi.status()!=WL_CONNECTED&&millis()-start<20000){
      digitalWrite(LED_PIN,!digitalRead(LED_PIN));
      delay(250);
      if(dots++%4==0)Serial.print(".");
    }
    Serial.println();
    if(WiFi.status()==WL_CONNECTED){
      Serial.println("✓ Connected to WiFi!");
      Serial.println("  Station IP: "+WiFi.localIP().toString());
      Serial.println("  AP IP: "+WiFi.softAPIP().toString());
      Serial.println("  Signal: "+String(WiFi.RSSI())+" dBm");
      Serial.println("✓ Repeater Mode Active (AP + STA)");
      wifiConnected=true;
      digitalWrite(LED_PIN,HIGH);
    }else{
      Serial.println("✗ Failed to connect to WiFi");
      Serial.println("  Status: "+String(WiFi.status()));
      Serial.println("  AP remains active at: "+WiFi.softAPIP().toString());
      Serial.println("⚠ AP-only mode (repeater disabled until WiFi connects)");
      // Keep WIFI_AP_STA mode - don't switch to WIFI_AP
      // This allows auto-reconnection attempts
      digitalWrite(LED_PIN,HIGH);
    }
  }else{
    Serial.println("No WiFi credentials configured");
    Serial.println("AP Mode Only - Connect to "+apSSID);
    WiFi.mode(WIFI_AP);
    digitalWrite(LED_PIN,HIGH);
  }
  
  // Sync time AFTER WiFi connects (needs internet)
  Serial.println("\n=== Time Synchronization ===");
  configTime(6*3600,0,"pool.ntp.org","time.nist.gov");
  if(wifiConnected){
    Serial.print("Syncing time from NTP");
    int attempts=0;
    while(time(nullptr)<100000&&attempts++<60){
      Serial.print(".");
      delay(500);
    }
    if(time(nullptr)>=100000){
      Serial.println(" ✓ Success!");
      Serial.println("Current time: "+getTimeString(time(nullptr)));
    }else{
      Serial.println(" ✗ Failed (using system time)");
    }
  }else{
    Serial.println("⚠ Skipping NTP sync (no internet connection)");
    Serial.println("Time will show as 1970 until WiFi connects");
  }
  
  bootTime=time(nullptr);
  if(bootTime<100000){
    Serial.println("⚠ Warning: Invalid time detected ("+String(bootTime)+")");
    bootTime=0; // Mark as invalid
  }
  
  checkMonthlyReset();
  
  time_t lastOn=0;
  time_t offDuration=0;
  File f=SPIFFS.open(lastOnFile,"r");
  if(f){
    lastOn=f.readString().toInt();
    f.close();
  }
  
  if(lastOn>0&&bootTime>lastOn&&bootTime>=100000){
    offDuration=bootTime-lastOn;
    logEvent("[OFF]",lastOn,offDuration);
    Serial.println("Power OFF duration: "+formatDuration(offDuration));
  }
  
  if(bootTime>=100000){
    File lf=SPIFFS.open(lastOnFile,"w");
    if(lf){
      lf.print(bootTime);
      lf.close();
    }
    logEvent("[ON]",bootTime,0);
    Serial.println("Power ON logged at: "+getTimeString(bootTime));
  }else{
    Serial.println("⚠ Skipping power logging (invalid time)");
  }
  
  server.on("/",handleRoot);
  server.on("/stats",handleStats);
  server.on("/config",handleConfig);
  server.on("/save",HTTP_POST,handleSave);
  server.on("/clear",handleClear);
  server.begin();
  Serial.println("Web server started!");
}

void loop(){
  server.handleClient();
  
  // Auto-reconnect WiFi if configured and disconnected (for repeater mode)
  static unsigned long lastWiFiCheck=0;
  static bool wasConnected=false;
  if(wifiSSID.length()>0&&millis()-lastWiFiCheck>30000){
    bool isConnected=(WiFi.status()==WL_CONNECTED);
    if(!isConnected&&WiFi.getMode()!=WIFI_AP){
      Serial.println("WiFi disconnected. Attempting reconnect...");
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(wifiSSID.c_str(),wifiPASS.c_str());
    }else if(isConnected&&!wasConnected){
      Serial.println("✓ WiFi reconnected! IP: "+WiFi.localIP().toString());
      Serial.println("✓ Repeater Mode Active");
    }
    wasConnected=isConnected;
    lastWiFiCheck=millis();
  }
  
  // Update timestamp every hour
  static unsigned long lastCheck=0;
  if(millis()-lastCheck>3600000){
    time_t now=time(nullptr);
    if(now>=100000){
      File lf=SPIFFS.open(lastOnFile,"w");
      if(lf){
        lf.print(now);
        lf.close();
      }
    }
    lastCheck=millis();
  }
  
  // Retry NTP sync if time is invalid and WiFi is connected
  static unsigned long lastNTPCheck=0;
  static bool ntpSynced=false;
  if(!ntpSynced&&WiFi.status()==WL_CONNECTED&&millis()-lastNTPCheck>60000){
    time_t now=time(nullptr);
    if(now<100000){
      Serial.println("Retrying NTP sync...");
      configTime(6*3600,0,"pool.ntp.org","time.nist.gov");
      delay(2000);
      now=time(nullptr);
      if(now>=100000){
        Serial.println("✓ NTP sync successful: "+getTimeString(now));
        ntpSynced=true;
        bootTime=now;
      }
    }else{
      ntpSynced=true;
    }
    lastNTPCheck=millis();
  }
}
