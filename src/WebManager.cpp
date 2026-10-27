#include "WebManager.h"

// Embedded HTML, CSS & JavaScript Dashboard (Offline-capable, no CDNs required)
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Plant Care Dashboard</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --card-border: #334155;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --accent-green: #10b981;
      --accent-blue: #38bdf8;
      --accent-red: #ef4444;
      --accent-yellow: #f59e0b;
      --btn-hover: #059669;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 20px 16px;
    }
    header {
      width: 100%;
      max-width: 600px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 24px;
    }
    .brand {
      display: flex;
      align-items: center;
      gap: 10px;
      font-size: 1.25rem;
      font-weight: 700;
    }
    .badge-live {
      display: flex;
      align-items: center;
      gap: 6px;
      background: rgba(16, 185, 129, 0.15);
      color: var(--accent-green);
      font-size: 0.75rem;
      padding: 4px 10px;
      border-radius: 9999px;
      border: 1px solid rgba(16, 185, 129, 0.3);
    }
    .live-dot {
      width: 8px;
      height: 8px;
      background: var(--accent-green);
      border-radius: 50%;
      animation: pulse 1.8s infinite;
    }
    @keyframes pulse {
      0% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(0.85); }
      100% { opacity: 1; transform: scale(1); }
    }
    .container {
      width: 100%;
      max-width: 600px;
      display: flex;
      flex-direction: column;
      gap: 16px;
    }
    .card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 16px;
      padding: 20px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.25);
    }
    .card-title {
      font-size: 0.85rem;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      color: var(--text-muted);
      margin-bottom: 12px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .moisture-display {
      display: flex;
      align-items: baseline;
      gap: 8px;
      margin-bottom: 12px;
    }
    .moisture-val {
      font-size: 3.5rem;
      font-weight: 800;
      color: var(--text);
      line-height: 1;
    }
    .moisture-unit {
      font-size: 1.5rem;
      color: var(--text-muted);
    }
    .progress-bar-bg {
      width: 100%;
      height: 12px;
      background: #334155;
      border-radius: 9999px;
      overflow: hidden;
      margin-bottom: 14px;
    }
    .progress-bar-fill {
      height: 100%;
      width: 0%;
      background: var(--accent-green);
      transition: width 0.5s ease, background-color 0.5s ease;
      border-radius: 9999px;
    }
    .meta-row {
      display: flex;
      justify-content: space-between;
      font-size: 0.85rem;
      color: var(--text-muted);
    }
    .status-pill {
      font-weight: 600;
      padding: 2px 8px;
      border-radius: 6px;
      font-size: 0.8rem;
    }
    .status-healthy { background: rgba(16, 185, 129, 0.2); color: var(--accent-green); }
    .status-dry { background: rgba(239, 68, 68, 0.2); color: var(--accent-red); }
    .status-wet { background: rgba(56, 189, 248, 0.2); color: var(--accent-blue); }

    .pump-card-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 14px;
    }
    .pump-state {
      font-size: 1.1rem;
      font-weight: 700;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .pump-active {
      color: var(--accent-blue);
      animation: pulse 1s infinite;
    }
    .pump-idle { color: var(--text-muted); }
    .btn {
      width: 100%;
      background: var(--accent-green);
      color: #fff;
      font-size: 1rem;
      font-weight: 600;
      border: none;
      border-radius: 10px;
      padding: 14px 20px;
      cursor: pointer;
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 8px;
      transition: all 0.2s;
    }
    .btn:hover:not(:disabled) {
      background: var(--btn-hover);
      transform: translateY(-1px);
    }
    .btn:disabled {
      background: #475569;
      cursor: not-allowed;
      opacity: 0.6;
    }
    .form-group {
      margin-bottom: 14px;
    }
    .form-group label {
      display: block;
      font-size: 0.85rem;
      color: var(--text-muted);
      margin-bottom: 6px;
    }
    .slider-row {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    .slider {
      flex: 1;
      height: 8px;
      border-radius: 5px;
      background: #334155;
      outline: none;
      -webkit-appearance: none;
    }
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: var(--accent-blue);
      cursor: pointer;
    }
    .toggle-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding-top: 4px;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 46px;
      height: 24px;
    }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider-switch {
      position: absolute;
      cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background-color: #475569;
      transition: .3s;
      border-radius: 24px;
    }
    .slider-switch:before {
      position: absolute;
      content: "";
      height: 18px;
      width: 18px;
      left: 3px;
      bottom: 3px;
      background-color: white;
      transition: .3s;
      border-radius: 50%;
    }
    input:checked + .slider-switch { background-color: var(--accent-green); }
    input:checked + .slider-switch:before { transform: translateX(22px); }

    .btn-secondary {
      background: #334155;
      color: var(--text);
      font-size: 0.9rem;
      padding: 10px 16px;
      margin-top: 8px;
    }
    .btn-secondary:hover:not(:disabled) { background: #475569; }

    footer {
      margin-top: 24px;
      font-size: 0.75rem;
      color: var(--text-muted);
      text-align: center;
    }
  </style>
</head>
<body>

  <header>
    <div class="brand">
      <span>🌱</span>
      <span>Smart Plant Care</span>
    </div>
    <div class="badge-live">
      <div class="live-dot"></div>
      <span id="liveStatusText">ONLINE</span>
    </div>
  </header>

  <main class="container">
    <!-- Moisture Sensor Card -->
    <div class="card">
      <div class="card-title">
        <span>Soil Moisture</span>
        <span id="statusPill" class="status-pill status-healthy">HEALTHY</span>
      </div>
      <div class="moisture-display">
        <span id="moistureVal" class="moisture-val">--</span>
        <span class="moisture-unit">%</span>
      </div>
      <div class="progress-bar-bg">
        <div id="progressBar" class="progress-bar-fill"></div>
      </div>
      <div class="meta-row">
        <span>Raw Sensor ADC: <strong id="rawAdc">--</strong></span>
        <span>Target Threshold: <strong id="currentThreshold">--%</strong></span>
      </div>
    </div>

    <!-- Pump Control Card -->
    <div class="card">
      <div class="pump-card-header">
        <span class="card-title" style="margin-bottom:0;">Water Pump</span>
        <div id="pumpState" class="pump-state pump-idle">
          <span id="pumpIcon">💧</span>
          <span id="pumpText">IDLE</span>
        </div>
      </div>
      <p id="cooldownText" style="font-size: 0.85rem; color: var(--text-muted); margin: 8px 0 14px 0;">
        Ready for watering
      </p>
      <button id="btnWater" class="btn" onclick="triggerWatering()">
        <span>💧</span>
        <span>Water Plant Now</span>
      </button>
    </div>

    <!-- Automation & Threshold Settings Card -->
    <div class="card">
      <div class="card-title">Automation Settings</div>
      
      <div class="form-group toggle-row">
        <div>
          <strong style="font-size: 0.95rem;">Auto-Watering</strong>
          <p style="font-size: 0.75rem; color: var(--text-muted);">Water automatically when soil is dry</p>
        </div>
        <label class="switch">
          <input type="checkbox" id="autoToggle" onchange="submitSettings()">
          <span class="slider-switch"></span>
        </label>
      </div>

      <hr style="border: 0; border-top: 1px solid var(--card-border); margin: 14px 0;">

      <div class="form-group">
        <label for="thresholdInput">Trigger Dry Threshold: <strong id="sliderValDisplay">30%</strong></label>
        <div class="slider-row">
          <span style="font-size: 0.8rem; color: var(--text-muted);">10%</span>
          <input type="range" id="thresholdInput" class="slider" min="10" max="80" value="30" oninput="updateSliderLabel(this.value)" onchange="submitSettings()">
          <span style="font-size: 0.8rem; color: var(--text-muted);">80%</span>
        </div>
      </div>
    </div>

    <!-- System Telemetry Card -->
    <div class="card" style="padding: 14px 20px;">
      <div class="meta-row">
        <span>Wi-Fi Signal: <strong id="wifiSignal">-- dBm</strong></span>
        <span>ESP32 Uptime: <strong id="uptime">--</strong></span>
      </div>
    </div>
  </main>

  <footer>
    ESP32 Smart Plant Care System &bull; Live Telemetry
  </footer>

  <script>
    let isWateringLocal = false;

    function formatUptime(seconds) {
      const hrs = Math.floor(seconds / 3600);
      const mins = Math.floor((seconds % 3600) / 60);
      const secs = seconds % 60;
      if (hrs > 0) return `${hrs}h ${mins}m ${secs}s`;
      if (mins > 0) return `${mins}m ${secs}s`;
      return `${secs}s`;
    }

    function updateSliderLabel(val) {
      document.getElementById('sliderValDisplay').innerText = val + '%';
    }

    async function fetchTelemetry() {
      try {
        const res = await fetch('/api/status');
        if (!res.ok) throw new Error('Network error');
        const data = await res.json();

        // Update Soil Moisture
        const m = data.moisture;
        document.getElementById('moistureVal').innerText = m;
        document.getElementById('rawAdc').innerText = data.raw;
        document.getElementById('currentThreshold').innerText = data.threshold + '%';
        
        const bar = document.getElementById('progressBar');
        bar.style.width = Math.min(100, Math.max(0, m)) + '%';
        
        // Status pill & colors
        const pill = document.getElementById('statusPill');
        if (m < data.threshold) {
          pill.className = 'status-pill status-dry';
          pill.innerText = 'DRY (NEEDS WATER)';
          bar.style.backgroundColor = 'var(--accent-red)';
        } else if (m > 80) {
          pill.className = 'status-pill status-wet';
          pill.innerText = 'VERY WET';
          bar.style.backgroundColor = 'var(--accent-blue)';
        } else {
          pill.className = 'status-pill status-healthy';
          pill.innerText = 'HEALTHY';
          bar.style.backgroundColor = 'var(--accent-green)';
        }

        // Pump State
        const btnWater = document.getElementById('btnWater');
        const pumpText = document.getElementById('pumpText');
        const pumpState = document.getElementById('pumpState');
        const cooldownText = document.getElementById('cooldownText');

        if (data.pump) {
          pumpText.innerText = 'WATERING NOW...';
          pumpState.className = 'pump-state pump-active';
          btnWater.disabled = true;
          cooldownText.innerText = 'Pump running (watering in progress)...';
        } else {
          pumpText.innerText = 'IDLE';
          pumpState.className = 'pump-state pump-idle';

          if (data.cooldownRemainingMs > 0) {
            const secsLeft = Math.ceil(data.cooldownRemainingMs / 1000);
            btnWater.disabled = true;
            cooldownText.innerText = `Cooldown active: ${secsLeft}s remaining`;
          } else {
            btnWater.disabled = false;
            cooldownText.innerText = 'Ready for watering';
          }
        }

        // Settings (only update if user is not actively adjusting)
        if (!document.getElementById('thresholdInput').matches(':active')) {
          document.getElementById('thresholdInput').value = data.threshold;
          document.getElementById('sliderValDisplay').innerText = data.threshold + '%';
        }
        document.getElementById('autoToggle').checked = data.autoWatering;

        // System telemetry
        document.getElementById('wifiSignal').innerText = data.wifiRssi + ' dBm';
        document.getElementById('uptime').innerText = formatUptime(data.uptimeSec);
        document.getElementById('liveStatusText').innerText = 'ONLINE';

      } catch (err) {
        document.getElementById('liveStatusText').innerText = 'RECONNECTING...';
      }
    }

    async function triggerWatering() {
      const btn = document.getElementById('btnWater');
      btn.disabled = true;
      try {
        const res = await fetch('/api/water', { method: 'POST' });
        const json = await res.json();
        if (!json.success) {
          alert('Watering request rejected: ' + (json.message || 'Cooldown in progress'));
        }
        fetchTelemetry();
      } catch (e) {
        alert('Failed to send water command: ' + e.message);
      }
    }

    async function submitSettings() {
      const threshold = document.getElementById('thresholdInput').value;
      const autoWatering = document.getElementById('autoToggle').checked ? '1' : '0';

      try {
        await fetch(`/api/settings?threshold=${threshold}&auto=${autoWatering}`, {
          method: 'POST'
        });
        fetchTelemetry();
      } catch (e) {
        console.error('Failed to save settings:', e);
      }
    }

    // Initial load and periodic polling every 2 seconds
    fetchTelemetry();
    setInterval(fetchTelemetry, 2000);
  </script>
</body>
</html>
)rawliteral";

WebManager::WebManager(int port, 
                       SoilSensor& soil, 
                       PumpController& pump, 
                       int& thresholdRef, 
                       bool& autoWateringRef,
                       const char* ssid, 
                       const char* password, 
                       const char* hostname)
  : _server(port),
    _soilSensor(soil),
    _pumpController(pump),
    _threshold(thresholdRef),
    _autoWatering(autoWateringRef),
    _ssid(ssid),
    _password(password),
    _hostname(hostname),
    _wifiConnected(false) {}

bool WebManager::begin() {
  Serial.print(F("Connecting to Wi-Fi: "));
  Serial.println(_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);

  // Attempt connection with 15-second timeout (non-freezing)
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(400);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    _wifiConnected = true;
    Serial.print(F("Wi-Fi connected! IP address: "));
    Serial.println(WiFi.localIP());

    // Initialize mDNS responder (http://plantcare.local)
    if (MDNS.begin(_hostname)) {
      Serial.print(F("mDNS responder started: http://"));
      Serial.print(_hostname);
      Serial.println(F(".local"));
      MDNS.addService("http", "tcp", 80);
    }
  } else {
    _wifiConnected = false;
    Serial.println(F("Wi-Fi connection timed out. System running in offline mode."));
  }

  // Register Server Handlers
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });
  _server.on("/api/water", HTTP_POST, [this]() { handlePostWater(); });
  _server.on("/api/settings", HTTP_POST, [this]() { handlePostSettings(); });
  _server.onNotFound([this]() { handleNotFound(); });

  _server.begin();
  Serial.println(F("HTTP Web Server started."));
  return _wifiConnected;
}

void WebManager::handleClient() {
  _server.handleClient();
}

bool WebManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WebManager::getLocalIP() const {
  if (isConnected()) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
}

int WebManager::getRSSI() const {
  if (isConnected()) {
    return WiFi.RSSI();
  }
  return 0;
}

void WebManager::handleRoot() {
  _server.send_P(200, "text/html", INDEX_HTML);
}

void WebManager::handleGetStatus() {
  int moisture = _soilSensor.getMoisturePercent();
  int rawAdc = _soilSensor.getRawValue();
  bool isWatering = _pumpController.isWatering();
  unsigned long cooldown = _pumpController.getRemainingCooldownMs();
  int rssi = getRSSI();
  unsigned long uptime = millis() / 1000;

  String json = "{";
  json += "\"moisture\":" + String(moisture) + ",";
  json += "\"raw\":" + String(rawAdc) + ",";
  json += "\"pump\":" + String(isWatering ? "true" : "false") + ",";
  json += "\"threshold\":" + String(_threshold) + ",";
  json += "\"autoWatering\":" + String(_autoWatering ? "true" : "false") + ",";
  json += "\"cooldownRemainingMs\":" + String(cooldown) + ",";
  json += "\"wifiRssi\":" + String(rssi) + ",";
  json += "\"uptimeSec\":" + String(uptime);
  json += "}";

  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.send(200, "application/json", json);
}

void WebManager::handlePostWater() {
  bool ok = _pumpController.requestWatering();

  String json = "{";
  if (ok) {
    json += "\"success\":true,\"message\":\"Watering started\"";
  } else {
    json += "\"success\":false,\"message\":\"Cannot water (cooling down or already running)\"";
  }
  json += "}";

  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.send(ok ? 200 : 400, "application/json", json);
}

void WebManager::handlePostSettings() {
  if (_server.hasArg("threshold")) {
    int val = _server.arg("threshold").toInt();
    if (val >= 5 && val <= 95) {
      _threshold = val;
    }
  }

  if (_server.hasArg("auto")) {
    int autoVal = _server.arg("auto").toInt();
    _autoWatering = (autoVal == 1);
  }

  String json = "{\"success\":true,\"threshold\":" + String(_threshold) + 
                ",\"autoWatering\":" + String(_autoWatering ? "true" : "false") + "}";

  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.send(200, "application/json", json);
}

void WebManager::handleNotFound() {
  _server.send(404, "text/plain", "404: Not Found");
}
