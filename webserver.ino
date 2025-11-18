void setupWebServer() {
  Serial.println("\n[WEB] Setting up web server...");
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/config", HTTP_POST, handleSaveConfig);
  server.on("/api/providers", HTTP_GET, handleGetProviders);
  server.on("/api/providers", HTTP_POST, handleSaveProviders);
  server.on("/api/refresh", HTTP_POST, handleRefresh);
  server.on("/api/reset", HTTP_POST, handleReset);
  
  server.begin();
  Serial.println("[WEB] Web server started on port 80");
  Serial.println("[WEB] Registered endpoints:");
  Serial.println("[WEB]   GET  /");
  Serial.println("[WEB]   GET  /api/status");
  Serial.println("[WEB]   POST /api/config");
  Serial.println("[WEB]   GET  /api/providers");
  Serial.println("[WEB]   POST /api/providers");
  Serial.println("[WEB]   POST /api/refresh");
  Serial.println("[WEB]   POST /api/reset");
}

const char HTML_PAGE[] PROGMEM = R"(<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'><title>Notification Hub</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;padding:20px}.container{max-width:800px;margin:0 auto}.card{background:white;border-radius:16px;padding:24px;margin-bottom:20px;box-shadow:0 8px 16px rgba(0,0,0,0.1)}h1{color:#2d3748;margin-bottom:8px}h2{color:#4a5568;margin-bottom:16px;font-size:20px}.subtitle{color:#718096;margin-bottom:24px}.form-group{margin-bottom:20px}label{display:block;color:#4a5568;font-weight:600;margin-bottom:8px;font-size:14px}input[type='text'],input[type='password'],input[type='number']{width:100%;padding:12px;border:2px solid #e2e8f0;border-radius:8px;font-size:14px}input:focus{outline:none;border-color:#667eea}button{padding:12px 24px;background:#667eea;color:white;border:none;border-radius:8px;font-size:14px;font-weight:600;cursor:pointer;width:100%;margin-top:8px}button:hover{background:#5568d3}.status{display:flex;justify-content:space-between;padding:12px;background:#edf2f7;border-radius:8px;margin-bottom:16px}.indicator{width:12px;height:12px;border-radius:50%;background:#48bb78}.provider{padding:16px;border:2px solid #e2e8f0;border-radius:8px;margin-bottom:12px}.badge{background:#f3e8ff;color:#6b21a8;padding:4px 12px;border-radius:12px;font-size:12px;font-weight:600}.message{padding:12px;border-radius:8px;margin-bottom:16px}.success{background:#c6f6d5;color:#22543d}.error{background:#fed7d7;color:#742a2a}.tabs{display:flex;gap:8px;margin-bottom:20px;border-bottom:2px solid #e2e8f0}.tab{padding:12px 20px;background:none;border:none;color:#718096;font-weight:600;cursor:pointer;border-bottom:2px solid transparent;margin-bottom:-2px;width:auto}.tab.active{color:#667eea;border-bottom-color:#667eea}.tab-content{display:none}.tab-content.active{display:block}</style></head><body><div class='container'><div class='card'><h1>🔔 Notification Hub</h1><p class='subtitle'>Configure your notification dashboard</p><div class='status'><div><div class='indicator' id='wifi'></div><span id='wifiStatus'>Checking...</span></div><div><span id='total'>0 notifications</span></div></div><div class='tabs'><button class='tab active' onclick='showTab(0)'>Dashboard</button><button class='tab' onclick='showTab(1)'>WiFi</button><button class='tab' onclick='showTab(2)'>Providers</button><button class='tab' onclick='showTab(3)'>Settings</button></div><div id='msg'></div><div id='tab0' class='tab-content active'><h2>Status</h2><div id='dashboard'>Loading...</div><button onclick='refresh()'>Refresh Now</button></div><div id='tab1' class='tab-content'><h2>WiFi Configuration</h2><form onsubmit='saveWiFi(event)'><div class='form-group'><label>WiFi SSID</label><input type='text' id='ssid' required></div><div class='form-group'><label>WiFi Password</label><input type='password' id='pass'></div><div class='form-group'><label>Admin Password</label><input type='password' id='admin' required></div><button type='submit'>Save WiFi</button></form></div><div id='tab2' class='tab-content'><h2>Providers</h2><div id='providerList'></div><button onclick='saveProviders()'>Save Providers</button></div><div id='tab3' class='tab-content'><h2>Settings</h2><form onsubmit='saveSettings(event)'><div class='form-group'><label>Update Interval (minutes)</label><input type='number' id='interval' min='1' max='1440' value='10'></div><div class='form-group'><label>Admin Password</label><input type='password' id='adminConfirm' required></div><button type='submit'>Save Settings</button></form></div></div></div><script>let providers=[];function showTab(n){document.querySelectorAll('.tab').forEach((t,i)=>{t.classList.toggle('active',i===n)});document.querySelectorAll('.tab-content').forEach((c,i)=>{c.classList.toggle('active',i===n)})}async function loadStatus(){try{const r=await fetch('/api/status');const d=await r.json();document.getElementById('wifiStatus').textContent=d.wifiConnected?'Connected':'Not connected';document.getElementById('total').textContent=d.totalNotifications+' notifications';let html='';if(d.providers&&d.providers.length>0){d.providers.forEach(p=>{html+=`<div class='provider'><span class='badge'>${p.displayName}</span>: ${p.count} notifications</div>`})}else{html='<p>No providers configured</p>'}document.getElementById('dashboard').innerHTML=html}catch(e){console.error(e)}}async function loadProviders(){try{const r=await fetch('/api/providers');const d=await r.json();providers=d.providers;let html='';providers.forEach(p=>{html+=`<div class='provider'><label><input type='checkbox' ${p.enabled?'checked':''} onchange='providers[${p.id}].enabled=this.checked'> ${p.displayName}</label><input type='password' id='token${p.id}' placeholder='API Token' style='margin-top:8px'></div>`});document.getElementById('providerList').innerHTML=html}catch(e){console.error(e)}}async function saveWiFi(e){e.preventDefault();const params=new URLSearchParams();params.append('wifi_ssid',document.getElementById('ssid').value);params.append('wifi_password',document.getElementById('pass').value);params.append('admin_password',document.getElementById('admin').value);try{const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:params});const d=await r.json();showMsg(d.message||'Saved! Rebooting...','success')}catch(e){showMsg('Error: '+e.message,'error')}}async function saveProviders(){const pass=prompt('Enter admin password:');if(!pass)return;const data=providers.map(p=>{const token=document.getElementById('token'+p.id).value;return{id:p.id,enabled:p.enabled,apiToken:token||undefined}});try{const r=await fetch('/api/providers',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({admin_password:pass,providers:data})});const d=await r.json();showMsg(d.message||'Saved!','success');loadProviders();loadStatus()}catch(e){showMsg('Error: '+e.message,'error')}}async function saveSettings(e){e.preventDefault();const params=new URLSearchParams();params.append('update_interval',document.getElementById('interval').value);params.append('admin_password',document.getElementById('adminConfirm').value);try{const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:params});const d=await r.json();showMsg(d.message||'Saved!','success')}catch(e){showMsg('Error: '+e.message,'error')}}async function refresh(){try{await fetch('/api/refresh',{method:'POST'});showMsg('Refreshing...','success');setTimeout(loadStatus,2000)}catch(e){showMsg('Error: '+e.message,'error')}}function showMsg(text,type){const msg=document.getElementById('msg');msg.className='message '+type;msg.textContent=text;setTimeout(()=>msg.textContent='',5000)}loadStatus();loadProviders();setInterval(loadStatus,30000)</script></body></html>)";

void handleRoot() {
  Serial.println("[WEB] Request: GET / from " + server.client().remoteIP().toString());
  server.send_P(200, "text/html", HTML_PAGE);
  Serial.println("[WEB] Response: 200 OK (HTML page)");
}

void handleStatus() {
  Serial.println("[WEB] Request: GET /api/status");
  
  DynamicJsonDocument doc(2048);
  doc["wifiConnected"] = wifiConnected;
  doc["ssid"] = config.wifi_ssid;
  doc["ip"] = WiFi.localIP().toString();
  doc["updateInterval"] = config.update_interval;
  
  JsonArray providersArray = doc.createNestedArray("providers");
  int total = 0;
  
  for (int i = 0; i < MAX_PROVIDERS; i++) {
    if (providers[i].enabled) {
      JsonObject p = providersArray.createNestedObject();
      p["name"] = providers[i].name;
      p["displayName"] = providers[i].displayName;
      p["count"] = providers[i].notificationCount;
      p["reviews"] = providers[i].reviewRequests;
      p["mentions"] = providers[i].mentions;
      p["assignments"] = providers[i].assignments;
      p["other"] = providers[i].otherReasons;
      total += providers[i].notificationCount;
    }
  }
  
  doc["totalNotifications"] = total;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  Serial.println("[WEB] Response: 200 OK");
}

void handleSaveConfig() {
  Serial.println("[WEB] Request: POST /api/config from " + server.client().remoteIP().toString());
  
  String password = server.arg("admin_password");
  
  if (password != config.admin_password && config.configured) {
    Serial.println("[WEB] ✗ Invalid admin password");
    server.send(401, "application/json", "{\"error\":\"Invalid password\"}");
    return;
  }
  
  Serial.println("[WEB] Updating configuration...");
  if (server.hasArg("wifi_ssid")) {
    config.wifi_ssid = server.arg("wifi_ssid");
    Serial.println("[WEB]   New SSID: " + config.wifi_ssid);
  }
  if (server.hasArg("wifi_password")) {
    config.wifi_password = server.arg("wifi_password");
    Serial.println("[WEB]   New WiFi password set");
  }
  if (server.hasArg("update_interval")) {
    config.update_interval = server.arg("update_interval").toInt();
    Serial.println("[WEB]   New update interval: " + String(config.update_interval) + " minutes");
  }
  
  config.configured = true;
  saveConfig();
  
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Saved. Rebooting...\"}");
  Serial.println("[WEB] Configuration saved. Rebooting in 1 second...");
  delay(1000);
  ESP.restart();
}

void handleGetProviders() {
  Serial.println("[WEB] Request: GET /api/providers");
  
  DynamicJsonDocument doc(2048);
  JsonArray providersArray = doc.createNestedArray("providers");
  
  for (int i = 0; i < MAX_PROVIDERS; i++) {
    JsonObject p = providersArray.createNestedObject();
    p["id"] = i;
    p["name"] = providers[i].name;
    p["displayName"] = providers[i].displayName;
    p["enabled"] = providers[i].enabled;
    p["hasToken"] = providers[i].apiToken.length() > 0;
    p["count"] = providers[i].notificationCount;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  Serial.println("[WEB] Response: 200 OK");
}

void handleSaveProviders() {
  Serial.println("[WEB] Request: POST /api/providers");
  
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    Serial.println("[WEB] ✗ Invalid JSON");
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String password = doc["admin_password"].as<String>();
  if (password != config.admin_password) {
    Serial.println("[WEB] ✗ Invalid password");
    server.send(401, "application/json", "{\"error\":\"Invalid password\"}");
    return;
  }
  
  Serial.println("[WEB] Updating providers...");
  JsonArray providersArray = doc["providers"];
  int updatedCount = 0;
  
  for (JsonObject p : providersArray) {
    int id = p["id"].as<int>();
    
    if (id >= 0 && id < MAX_PROVIDERS) {
      providers[id].enabled = p["enabled"].as<bool>();
      Serial.print("[WEB] ");
      Serial.print(providers[id].displayName);
      Serial.println(providers[id].enabled ? ": ENABLED" : ": DISABLED");
      
      if (p.containsKey("apiToken")) {
        String token = p["apiToken"].as<String>();
        if (token.length() > 0) {
          providers[id].apiToken = token;
          Serial.println("[WEB] Token updated");
        }
      }
      updatedCount++;
    }
  }
  
  saveProviderSettings();
  Serial.println("[WEB] ✓ Saved");
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Saved\"}");
}

void handleRefresh() {
  Serial.println("[WEB] Request: POST /api/refresh from " + server.client().remoteIP().toString());
  Serial.println("[WEB] Manual refresh triggered via web interface");
  updateAllProviders();
  server.send(200, "application/json", "{\"success\":true}");
  Serial.println("[WEB] Response: 200 OK (refresh complete)");
}

void handleReset() {
  Serial.println("[WEB] Request: POST /api/reset from " + server.client().remoteIP().toString());
  
  String password = server.arg("admin_password");
  
  if (password != config.admin_password) {
    Serial.println("[WEB] ✗ Invalid admin password");
    server.send(401, "application/json", "{\"error\":\"Invalid password\"}");
    return;
  }
  
  Serial.println("[WEB] ⚠ FACTORY RESET initiated!");
  Serial.println("[WEB] Clearing all configuration...");
  
  preferences.begin("config", false);
  preferences.clear();
  preferences.end();
  Serial.println("[WEB]   Config cleared");
  
  preferences.begin("providers", false);
  preferences.clear();
  preferences.end();
  Serial.println("[WEB]   Provider settings cleared");
  
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Reset. Rebooting...\"}");
  Serial.println("[WEB] Factory reset complete. Rebooting in 1 second...");
  delay(1000);
  ESP.restart();
}

