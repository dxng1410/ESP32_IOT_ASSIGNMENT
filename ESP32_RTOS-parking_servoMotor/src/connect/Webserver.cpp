/* Enhanced ESP32 Web Server with RGB LED, Single LED, and Sensor Display
  Based on Random Nerd Tutorials
*/
#include "WebServer.h"
#define NUM_PIXEL_WEB 4  // Same as in TaskLedRGB.cpp

// Username and password for web page access
const char* http_username = "admin";
const char* http_password = "admin";

// Assign output variables to GPIO pins
const int output10 = 8;
const int output17 = 9;
const int ledD13 = 48;  // LED D13 (GPIO48)

// --- CẤU HÌNH QUẠT/MOTOR ---
const int fanPin = 10;  
bool isFanRunning = false; 
int fanDutyCycle = 0;      
int fanStep = 5;           
unsigned long lastFanTime = 0;

String output10State = "off";
String output17State = "off";
String ledD13State = "off";
String rgbLedState = "off";
int currentRGBColor = 0; // 0=Red, 1=Green, 2=Blue, 3=White, 4=Off

// Biến lưu trữ giá trị cảm biến cho Web dùng
float web_temp = 0.0;
float web_hum = 0.0;
unsigned long lastDHTRead = 0; 

// Create a web server object
WebServer server(80);

// External sensor variables
extern DHT20 dht20;
extern float distance;
extern int lightValue;
extern float lightPercentage;
extern int soilMoistureValue;
extern float soilMoisturePercent;
// --- THÊM DÒNG NÀY ---
extern float ml_inference_result; 

Adafruit_NeoPixel neoPixel_web(NUM_PIXEL_WEB, output10 , NEO_GRB + NEO_KHZ800);

void handleWifiConfig() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-E'> <meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>ESP32 WiFi Config</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; padding: 50px; }";
    html += "div { background-color: white; max-width: 500px; margin: auto; padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }";
    html += "h1 { color: #333; }";
    html += "input[type='text'], input[type='password'], select { width: 90%; padding: 12px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
    html += "input[type='submit'] { background-color: #4CAF50; color: white; padding: 14px 20px; border: none; border-radius: 5px; cursor: pointer; width: 90%; font-size: 16px; }";
    html += "input[type='submit']:hover { background-color: #45a049; }";
    html += "#scan-btn { background-color: #008CBA; }";
    html += "a { color: #008CBA; text-decoration: none; }";
    html += "</style>";
    html += "<script>";
    html += "function scan() {";
    html += "  document.getElementById('status').innerHTML = 'Scanning...';";
    html += "  fetch('/scan').then(response => response.json()).then(data => {";
    html += "    let sel = document.getElementById('ssid-select');";
    html += "    sel.innerHTML = '<option value=\"\">-- Select WiFi --</option>';";
    html += "    data.forEach(net => {";
    html += "      sel.innerHTML += '<option value=\"' + net.ssid + '\">' + net.ssid + ' (' + net.rssi + ')</option>';";
    html += "    });";
    html += "    document.getElementById('status').innerHTML = 'Scan complete.';";
    html += "  });";
    html += "}";
    html += "function updateSSID() {";
    html += "  let sel = document.getElementById('ssid-select');";
    html += "  if (sel.value) document.getElementById('ssid-input').value = sel.value;";
    html += "}";
    html += "</script>";
    html += "</head><body onload='scan()'>";
    html += "<div><h1>ESP32 WiFi Configuration</h1>";
    html += "<p>Please connect to your WiFi network.</p>";
    html += "<p id='status'></p>";
    html += "<select id='ssid-select' onchange='updateSSID()'><option value=''>Scanning...</option></select><br/>";
    html += "<form action='/connect' method='POST'>";
    html += "<input id='ssid-input' type='text' name='ssid' placeholder='Or type SSID manually' required><br/>";
    html += "<input type='password' name='pass' placeholder='Password'><br/>";
    html += "<input type='submit' value='Connect'>";
    html += "</form>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void handleScan() {
    Serial.println("Scan WiFi started...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan done.");
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\"";
        json += ",\"rssi\":" + String(WiFi.RSSI(i));
        json += "}";
    }
    json += "]";
    server.send(200, "application/json", json);
}

void handleWifiConnect() {
    if (!server.hasArg("ssid") || !server.hasArg("pass")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    server.send(200, "text/html", "<h1>Connecting...</h1><p>Connecting to " + ssid + ".<br/>If successful, the ESP32 will restart and connect to this network.</p>");

    // Lưu thông tin vào Preferences
    preferences.begin("wifi-creds", false); // false = read-write
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.end();

    Serial.println("Credentials saved. Restarting...");
    delay(1000);
    ESP.restart(); // Khởi động lại để áp dụng chế độ STA
}

bool isAuthenticated() {
  if (!server.authenticate(http_username, http_password)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

// Function to get sensor data as HTML string (FIXED FOR DARK MODE)
String getSensorDataHTML() {
  String html = "<div class='sensor-box'>"; 
  html += "<div class='sensor-item'>";
  html += "<span class='sensor-label'>Temperature</span>";
  html += "<span class='sensor-value'>" + String(web_temp, 1) + " <small>°C</small></span>";
  html += "</div>";

  html += "<div class='sensor-item'>";
  html += "<span class='sensor-label'>Humidity</span>";
  html += "<span class='sensor-value'>" + String(web_hum, 1) + " <small>%</small></span>";
  html += "</div>";
  html += "</div>";
  return html;
}

void setRGBColor(int colorIndex) {
  uint32_t colors[] = {
    neoPixel_web.Color(255, 0, 0),    // Red
    neoPixel_web.Color(0, 255, 0),    // Green
    neoPixel_web.Color(0, 0, 255),    // Blue
    neoPixel_web.Color(255, 255, 255), // White
    neoPixel_web.Color(0, 0, 0)       // Off
  };
  
  if (colorIndex >= 0 && colorIndex < 5) {
    neoPixel_web.clear();
    for (int i = 0; i < 4; i++) { // NUM_PIXEL = 4
      neoPixel_web.setPixelColor(i, colors[colorIndex]);
    }
    neoPixel_web.show();
    currentRGBColor = colorIndex;
    rgbLedState = (colorIndex == 4) ? "off" : "on";
  }
}

// --- NEW HANDLEROOT WITH FIXED TOGGLE SWITCH ---
void handleRoot() {
  if (!isAuthenticated()) return;

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>PROJECT TKLL - HCMUT Dashboard</title>";
  html += "<link rel=\"icon\" href=\"data:,\">";
  html += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>";
  html += "<script src=\"https://cdn.jsdelivr.net/npm/@jaames/iro@5\"></script>";
  
  html += "<style>";
  // --- MÀU SẮC (THEME VARIABLES) ---
  html += ":root {";
  html += "  --bg-body: #f0f2f5;";
  html += "  --bg-card: #ffffff;";
  html += "  --bg-box: #f7f8fa;";
  html += "  --text-main: #1c1e21;";
  html += "  --text-sub: #65676b;";
  html += "  --primary: #1877f2;"; // Xanh Facebook/Bách Khoa
  html += "  --shadow: 0 2px 8px rgba(0,0,0,0.1);";
  html += "}";

  // Cấu hình DARK MODE
  html += "[data-theme='dark'] {";
  html += "  --bg-body: #18191a;";
  html += "  --bg-card: #242526;";
  html += "  --bg-box: #3a3b3c;";
  html += "  --text-main: #e4e6eb;";
  html += "  --text-sub: #b0b3b8;";
  html += "  --shadow: 0 4px 12px rgba(0,0,0,0.5);";
  html += "}";

  // --- GLOBAL STYLES ---
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background-color: var(--bg-body); color: var(--text-main); margin: 0; padding: 20px; transition: 0.3s; }";
  
  html += ".main-wrapper { max-width: 1000px; margin: 0 auto; }";
  
  // Header
  html += ".header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 25px; padding-bottom: 15px; border-bottom: 2px solid var(--bg-box); }";
  html += ".brand { display: flex; align-items: center; gap: 15px; }";
  html += ".brand img { height: 65px; width: auto; object-fit: contain; }"; 
  
  // --- CSS TOGGLE SWITCH (ĐÃ FIX LỖI CHE ICON) ---
  html += ".switch { position: relative; display: inline-block; width: 64px; height: 32px; }";
  html += ".switch input { opacity: 0; width: 0; height: 0; }";
  
  // Thanh trượt (Slider)
  html += ".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #87CEEB; transition: .4s; border-radius: 34px; box-shadow: inset 0 0 5px rgba(0,0,0,0.2); }";
  
  // Nút tròn (Knob) - Z-index cao hơn icon để đè lên
  html += ".slider:before { position: absolute; content: ''; height: 24px; width: 24px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; z-index: 2; box-shadow: 0 2px 5px rgba(0,0,0,0.3); }";
  
  // Khi bật (Dark Mode)
  html += "input:checked + .slider { background-color: #2c3e50; }";
  html += "input:checked + .slider:before { transform: translateX(32px); }";
  
  // Icons - Nằm dưới nút tròn (z-index 1)
  html += ".icon-sun, .icon-moon { position: absolute; top: 50%; transform: translateY(-50%); font-size: 18px; z-index: 1; transition: opacity 0.3s; }";
  
  // Vị trí cố định
  html += ".icon-sun { right: 8px; opacity: 1; }"; // Mặc định hiện Sun
  html += ".icon-moon { left: 8px; opacity: 0; }"; // Mặc định ẩn Moon
  
  // Logic hiển thị Icon (FIXED SELECTOR)
  html += "input:checked + .slider .icon-sun { opacity: 0; }"; // Bật -> Ẩn Sun
  html += "input:checked + .slider .icon-moon { opacity: 1; }"; // Bật -> Hiện Moon

  // Grid System
  html += ".grid-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 25px; margin-bottom: 25px; }";

  // Cards
  html += ".card { background-color: var(--bg-card); padding: 25px; border-radius: 15px; box-shadow: var(--shadow); transition: 0.3s; }";
  html += "h2 { color: var(--text-main); margin-top: 0; font-size: 1.4rem; border-left: 5px solid var(--primary); padding-left: 10px; }";
  html += "h3 { font-size: 1.1rem; color: var(--text-sub); margin-top: 0; }";

  // --- SENSOR BOX STYLES ---
  html += ".sensor-box { background-color: var(--bg-box); padding: 20px; border-radius: 12px; display: flex; justify-content: space-around; align-items: center; margin: 15px 0; }";
  html += ".sensor-item { text-align: center; }";
  html += ".sensor-label { display: block; font-size: 0.9rem; color: var(--text-sub); margin-bottom: 8px; text-transform: uppercase; font-weight: 600; letter-spacing: 0.5px; }";
  html += ".sensor-value { font-size: 2rem; font-weight: bold; color: var(--text-main); }";
  html += ".sensor-value small { font-size: 1rem; color: var(--text-sub); font-weight: normal; margin-left: 3px;}";

  // Slider & Button
  html += ".slider-range { -webkit-appearance: none; width: 100%; height: 10px; border-radius: 5px; background: var(--bg-box); outline: none; margin: 25px 0; }";
  html += ".slider-range::-webkit-slider-thumb { -webkit-appearance: none; width: 28px; height: 28px; border-radius: 50%; background: var(--primary); cursor: pointer; border: 3px solid var(--bg-card); box-shadow: 0 0 5px rgba(0,0,0,0.2); }";
  
  html += ".btn { width: 100%; padding: 14px; border: none; border-radius: 10px; font-weight: bold; cursor: pointer; color: white; transition: 0.2s; font-size: 1rem; }";
  // Nút Đỏ (OFF)
  html += ".btn-red { background-color: #ff4757; }";
  html += ".btn-red:hover { background-color: #ff6b81; transform: translateY(-2px); }";
  // Nút Xanh (ON) - MỚI
  html += ".btn-green { background-color: #2ecc71; }";
  html += ".btn-green:hover { background-color: #27ae60; transform: translateY(-2px); }";

  // TinyML Label
  html += ".ml-score { font-size: 1.6rem; font-weight: bold; color: #f1c40f; }";

  html += "</style></head><body>";

  html += "<div class='main-wrapper'>";
  
  // HEADER
  html += "<div class='header'>";
  html += "  <div class='brand'>";
  html += "    <img src='https://upload.wikimedia.org/wikipedia/commons/d/de/HCMUT_official_logo.png' alt='HCMUT Logo'>";
  html += "    <div><h1 style='margin:0; font-size:1.6rem; color: var(--primary);'>PROJECT TKLL</h1><span style='color:var(--text-sub)'>Smart IoT Dashboard</span></div>";
  html += "  </div>";
  
  // --- NÚT GẠT (TOGGLE SWITCH) ---
  html += "  <label class='switch'>";
  html += "    <input type='checkbox' id='theme-toggle' onchange='toggleTheme()'>";
  html += "    <span class='slider'>";
  html += "       <span class='icon-sun'>☀️</span>";
  html += "       <span class='icon-moon'>🌙</span>";
  html += "    </span>";
  html += "  </label>";
  html += "</div>";

  // GRID HÀNG 1
  html += "<div class='grid-container'>";
    
    // FAN CARD
    html += "<div class='card'>";
    html += "  <h2>Fan Speed Control</h2>";
    html += "  <div style='text-align:center; margin: 25px 0;'>";
    html += "    <span style='font-size:3.5rem; font-weight:bold; color:var(--primary)' id='fan_val_display'>" + String(fanDutyCycle) + "</span>";
    html += "    <br><span style='color:var(--text-sub); font-size: 0.9rem;'>PWM Output (0-255)</span>";
    html += "  </div>";
    html += "  <input type='range' min='0' max='255' value='" + String(fanDutyCycle) + "' class='slider-range' id='fanRange' oninput='updateFan(this.value)'>";
    html += "  <button class='btn btn-red' onclick='updateFan(0); document.getElementById(\"fanRange\").value=0;'>STOP FAN SYSTEM</button>";
    
    // TinyML Section
    html += "  <div style='margin-top:25px; padding-top:20px; border-top:2px dashed var(--bg-box); text-align:center;'>";
    html += "    <span style='color:var(--text-sub); display:block; margin-bottom:10px; font-weight:600;'>TinyML Anomaly Score</span>";
    html += "    <span id='ml_val' class='ml-score'>" + String(ml_inference_result, 2) + "</span>";
    html += "    <div style='font-size:0.8rem; color:var(--text-sub); margin-top:5px;'>Threshold: > 0.5 is Anomaly</div>";
    html += "  </div>";
    html += "</div>";

    // RGB CARD (ĐÃ CẬP NHẬT GIAO DIỆN VÀ NÚT)
    html += "<div class='card'>";
    html += "  <h2>RGB LED Control</h2>";
    // Thêm gap và padding để thoáng hơn
    html += "  <div style='display:flex; flex-direction:column; align-items:center; gap: 20px; padding: 10px 0;'>";
    html += "    <div id='picker'></div>";
    html += "    <div id='values' style='font-family:monospace; color:var(--text-main); font-size: 1.1rem;'></div>";
    // Nhóm 2 nút
    html += "    <div style='display:flex; gap: 15px; width: 100%;'>";
    html += "       <button class='btn btn-green' onclick='turnOn()'>Turn ON</button>";
    html += "       <button class='btn btn-red' onclick='turnOff()'>Turn OFF</button>";
    html += "    </div>";
    html += "  </div>";
    html += "</div>";

  html += "</div>"; // End Grid 1

  // GRID HÀNG 2 (SENSOR & CHART)
  html += "<div class='card' style='margin-bottom:30px;'>";
  html += "  <h2>Environment Monitor</h2>";
  html += getSensorDataHTML();
  html += "  <div style='position:relative; height:350px; width:100%; margin-top: 20px;'>";
  html += "    <canvas id='sensorChart'></canvas>";
  html += "  </div>";
  html += "</div>";

  html += "</div>"; // End wrapper

  // --- JAVASCRIPT ---
  html += "<script>";
  
  // Theme Toggle
  html += "function toggleTheme() {";
  html += "  const body = document.body;";
  html += "  const checkbox = document.getElementById('theme-toggle');";
  html += "  if (checkbox.checked) {";
  html += "    body.setAttribute('data-theme', 'dark');";
  html += "    updateChartTheme('dark');";
  html += "  } else {";
  html += "    body.setAttribute('data-theme', 'light');";
  html += "    updateChartTheme('light');";
  html += "  }";
  html += "}";

  // Fan Ajax
  html += "function updateFan(val) {";
  html += "  document.getElementById('fan_val_display').innerHTML = val;";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/fan/speed?value=' + val, true);";
  html += "  xhr.send();";
  html += "}";

  // RGB Picker
  html += "const colorPicker = new iro.ColorPicker('#picker', { width: 240, color: '#ffffff', layout: [{ component: iro.ui.Wheel, options: {} }, { component: iro.ui.Slider, options: { sliderType: 'value' } }] });";
  html += "let timeoutId = null;";
  html += "colorPicker.on('color:change', function(color) {";
  html += "  document.getElementById('values').innerHTML = `RGB: ${Math.round(color.rgb.r)}, ${Math.round(color.rgb.g)}, ${Math.round(color.rgb.b)}`;";
  html += "  if (timeoutId) clearTimeout(timeoutId);";
  html += "  timeoutId = setTimeout(() => {";
  html += "    const xhr = new XMLHttpRequest();";
  html += "    xhr.open('GET', `/rgb/custom?r=${Math.round(color.rgb.r)}&g=${Math.round(color.rgb.g)}&b=${Math.round(color.rgb.b)}`, true);";
  html += "    xhr.send();";
  html += "  }, 50);"; 
  html += "});";
  
  // HÀM TURN ON MỚI
  html += "function turnOn() {";
  html += "  const xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/rgb/white', true);"; // Bật màu trắng mặc định
  html += "  xhr.send();";
  html += "  colorPicker.color.rgb = { r: 255, g: 255, b: 255 };"; // Cập nhật picker
  html += "}";

  // HÀM TURN OFF
  html += "function turnOff() {";
  html += "  const xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/rgb/off', true);";
  html += "  xhr.send();";
  html += "  colorPicker.color.rgb = { r: 0, g: 0, b: 0 };";
  html += "}";

  // Chart
  html += "var ctx = document.getElementById('sensorChart').getContext('2d');";
  html += "var chart = new Chart(ctx, {";
  html += "  type: 'line',";
  html += "  data: { labels: [], datasets: [";
  html += "    { label: 'Temp (°C)', borderColor: '#ff4757', backgroundColor: 'rgba(255, 71, 87, 0.2)', tension: 0.4, fill: true, pointRadius: 3, data: [] },";
  html += "    { label: 'Hum (%)', borderColor: '#1e90ff', backgroundColor: 'rgba(30, 144, 255, 0.2)', tension: 0.4, fill: true, pointRadius: 3, data: [] }";
  html += "  ]},";
  html += "  options: { responsive: true, maintainAspectRatio: false, scales: { x: { grid: { color: '#e5e5e5', display: false } }, y: { grid: { color: '#e5e5e5', borderDash: [5, 5] } } }, plugins: { legend: { position: 'top', align: 'end' } } }";
  html += "});";

  html += "function updateChartTheme(mode) {";
  html += "  const color = (mode === 'dark') ? '#3a3b3c' : '#e5e5e5';";
  html += "  const textColor = (mode === 'dark') ? '#b0b3b8' : '#666';";
  html += "  chart.options.scales.x.grid.color = color;";
  html += "  chart.options.scales.y.grid.color = color;";
  html += "  Chart.defaults.color = textColor;"; 
  html += "  chart.update();";
  html += "}";

  // Data Update Loop
  html += "function updateData() {";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      var data = JSON.parse(this.responseText);"; 
  html += "      if(document.querySelectorAll('.sensor-value').length > 0) {";
  html += "         document.querySelectorAll('.sensor-value')[0].innerHTML = data.temp.toFixed(1) + ' <small>°C</small>';";
  html += "         document.querySelectorAll('.sensor-value')[1].innerHTML = data.hum.toFixed(1) + ' <small>%</small>';";
  html += "      }";
  html += "      if(document.getElementById('ml_val')) document.getElementById('ml_val').innerHTML = data.ml.toFixed(2);";
  
  html += "      var time = new Date().toLocaleTimeString();";
  html += "      if(chart.data.labels.length > 20) { chart.data.labels.shift(); chart.data.datasets[0].data.shift(); chart.data.datasets[1].data.shift(); }";
  html += "      chart.data.labels.push(time);";
  html += "      chart.data.datasets[0].data.push(data.temp);";
  html += "      chart.data.datasets[1].data.push(data.hum);";
  html += "      chart.update();";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('GET', '/readSensors', true);"; 
  html += "  xhttp.send();";
  html += "}";
  
  html += "setInterval(updateData, 2000);"; 
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleFanOn() {
  if (!isAuthenticated()) return;
  isFanRunning = true; 
  handleRoot(); 
}

void handleFanOff() {
  if (!isAuthenticated()) return;
  isFanRunning = false; 
  analogWrite(fanPin, 0); 
  handleRoot(); 
}

void handleFanSpeed() {
  if (!isAuthenticated()) return;

  if (server.hasArg("value")) {
    int speed = server.arg("value").toInt(); 
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;
    fanDutyCycle = speed; 
    analogWrite(fanPin, speed); 
    isFanRunning = (speed > 0); 
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing value");
  }
}

void handleRGBRed() {
  if (!isAuthenticated()) return;
  setRGBColor(0);
  handleRoot();
}

void handleRGBGreen() {
  if (!isAuthenticated()) return;
  setRGBColor(1);
  handleRoot();
}

void handleRGBBlue() {
  if (!isAuthenticated()) return;
  setRGBColor(2);
  handleRoot();
}

void handleRGBWhite() {
  if (!isAuthenticated()) return;
  setRGBColor(3);
  handleRoot();
}

void handleRGBOff() {
  if (!isAuthenticated()) return;
  setRGBColor(4);
  handleRoot();
}

void handleRGBCustom() {
  if (!isAuthenticated()) return;
  
  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("b").toInt();
  
  neoPixel_web.clear();
  uint32_t color = neoPixel_web.Color(r, g, b);
  for (int i = 0; i < NUM_PIXEL_WEB; i++) {
    neoPixel_web.setPixelColor(i, color);
  }
  neoPixel_web.show();
  
  server.send(200, "text/plain", "OK");
}

void InitWebsever() {
    if (isAPMode) {
        // --- CHẾ ĐỘ ACCESS POINT ---
        server.on("/", HTTP_GET, handleWifiConfig);
        server.on("/scan", HTTP_GET, handleScan);
        server.on("/connect", HTTP_POST, handleWifiConnect);

        server.begin();
        Serial.println("HTTP server started in AP Mode.");

    } else {
        // --- CHẾ ĐỘ STATION (BÌNH THƯỜNG) ---
        pinMode(output10, OUTPUT);
        pinMode(output17, OUTPUT);
        pinMode(ledD13, OUTPUT);
        
        digitalWrite(output10, LOW);
        digitalWrite(output17, LOW);
        digitalWrite(ledD13, LOW);

        setRGBColor(4); // Start with LED off

        publishData("webseverIP", WiFi.localIP().toString());
        
        server.on("/", handleRoot); 

        server.on("/servo/on", handleFanOn);   
        server.on("/servo/off", handleFanOff);

        server.on("/fan/speed", HTTP_GET, handleFanSpeed);
        
        server.on("/rgb/red", handleRGBRed);
        server.on("/rgb/green", handleRGBGreen);
        server.on("/rgb/blue", handleRGBBlue);
        server.on("/rgb/white", handleRGBWhite);
        server.on("/rgb/off", handleRGBOff);
        server.on("/rgb/custom", handleRGBCustom);

        server.on("/readSensors", HTTP_GET, []() {
        // Thêm trường "ml" vào chuỗi JSON
        String json = "{\"temp\":" + String(web_temp) + 
                      ",\"hum\":" + String(web_hum) + 
                      ",\"ml\":" + String(ml_inference_result) + "}";
        server.send(200, "application/json", json);
        });
        
        server.begin();
        Serial.println("HTTP server started in STA Mode with LED and sensor controls");
    }
}

void WebSeverloop() {
    server.handleClient(); // Xử lý Web
  
  // Code đọc cảm biến (Non-blocking)
  if (millis() - lastDHTRead > 1000) { // Cứ 1000ms (1 giây) đọc 1 lần
    lastDHTRead = millis();
    
    int status = dht20.read(); // Đọc cảm biến
    
    if (status == DHT20_OK) {
      web_temp = dht20.getTemperature();
      web_hum = dht20.getHumidity();
      
      Serial.print("Temp: "); Serial.print(web_temp);
      Serial.print(" - Hum: "); Serial.println(web_hum);
    } else {
      Serial.println("DHT20 Error in Loop");
    }
  }
}