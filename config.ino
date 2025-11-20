void initProviders() {
  Serial.println("\n[INIT] Initializing providers...");
  
  providers[GITHUB].name = "github";
  providers[GITHUB].displayName = "GitHub";
  providers[GITHUB].enabled = false;
  providers[GITHUB].notificationCount = 0;
  providers[GITHUB].reviewRequests = 0;
  providers[GITHUB].mentions = 0;
  providers[GITHUB].assignments = 0;
  providers[GITHUB].otherReasons = 0;
  
  Serial.println("[INIT] Base provider structure created");
  loadProviderSettings();
  Serial.println("[INIT] Providers initialized successfully");
}

void loadConfig() {
  Serial.println("\n[CONFIG] Loading configuration from flash...");
  preferences.begin("config", false);
  config.wifi_ssid = preferences.getString("wifi_ssid", "");
  config.wifi_password = preferences.getString("wifi_pass", "");
  config.admin_password = preferences.getString("admin_pass", "admin");
  config.update_interval = preferences.getInt("update_int", 10);
  config.configured = preferences.getBool("configured", false);
  config.wifi_ap_password = preferences.getString("ap_pass", "configure");
  currentScreen = preferences.getInt("current_screen", SCREEN_NOTIFICATIONS);
  preferences.end();
  
  Serial.println("[CONFIG] Configuration loaded:");
  Serial.println("  WiFi SSID: " + config.wifi_ssid);
  Serial.println("  WiFi Password: " + String(config.wifi_password.length() > 0 ? "***SET***" : "***NOT SET***"));
  Serial.println("  Admin Password: " + String(config.admin_password.length() > 0 ? "***SET***" : "***NOT SET***"));
  Serial.println("  AP Password: " + String(config.wifi_ap_password.length() > 0 ? "***SET***" : "***NOT SET***"));
  Serial.println("  Update Interval: " + String(config.update_interval) + " minutes");
  Serial.println("  Configured: " + String(config.configured ? "YES" : "NO"));
  Serial.print("  Current Screen: ");
  if (currentScreen == SCREEN_NOTIFICATIONS) Serial.println("Notifications");
  else if (currentScreen == SCREEN_PROFILE) Serial.println("Profile");
  else Serial.println("PR Overview");
}

void saveConfig() {
  Serial.println("\n[CONFIG] Saving configuration to flash...");
  preferences.begin("config", false);
  preferences.putString("wifi_ssid", config.wifi_ssid);
  preferences.putString("wifi_pass", config.wifi_password);
  preferences.putString("admin_pass", config.admin_password);
  preferences.putInt("update_int", config.update_interval);
  preferences.putBool("configured", config.configured);
  preferences.putString("ap_pass", config.wifi_ap_password);
  preferences.end();
  Serial.println("[CONFIG] Configuration saved successfully!");
  Serial.println("  WiFi SSID: " + config.wifi_ssid);
  Serial.println("  Update Interval: " + String(config.update_interval) + " minutes");
  Serial.println("  AP Password: " + String(config.wifi_ap_password.length() > 0 ? "***SET***" : "***NOT SET***"));
}

void loadProviderSettings() {
  Serial.println("\n[PROVIDERS] Loading provider settings from flash...");
  preferences.begin("providers", false);
  int enabledCount = 0;
  for (int i = 0; i < MAX_PROVIDERS; i++) {
    String prefix = providers[i].name + "_";
    providers[i].enabled = preferences.getBool((prefix + "enabled").c_str(), false);
    providers[i].apiToken = preferences.getString((prefix + "token").c_str(), "");
    
    Serial.print("[PROVIDERS]   " + providers[i].displayName + ": ");
    Serial.print(providers[i].enabled ? "ENABLED" : "DISABLED");
    Serial.println(providers[i].apiToken.length() > 0 ? " (Token set)" : " (No token)");
    
    if (providers[i].enabled) enabledCount++;
  }
  preferences.end();
  Serial.println("[PROVIDERS] Loaded " + String(enabledCount) + " enabled provider(s)");
}

void saveProviderSettings() {
  Serial.println("\n[PROVIDERS] Saving provider settings to flash...");
  preferences.begin("providers", false);
  int enabledCount = 0;
  for (int i = 0; i < MAX_PROVIDERS; i++) {
    String prefix = providers[i].name + "_";
    preferences.putBool((prefix + "enabled").c_str(), providers[i].enabled);
    preferences.putString((prefix + "token").c_str(), providers[i].apiToken);
    
    Serial.print("[PROVIDERS]   " + providers[i].displayName + ": ");
    Serial.println(providers[i].enabled ? "ENABLED" : "DISABLED");
    
    if (providers[i].enabled) enabledCount++;
  }
  preferences.end();
  Serial.println("[PROVIDERS] Saved " + String(enabledCount) + " enabled provider(s)");
}

void saveCurrentScreen() {
  preferences.begin("config", false);
  preferences.putInt("current_screen", currentScreen);
  preferences.end();
  Serial.print("[CONFIG] Current screen saved: ");
  if (currentScreen == SCREEN_NOTIFICATIONS) Serial.println("Notifications");
  else if (currentScreen == SCREEN_PROFILE) Serial.println("Profile");
  else Serial.println("PR Overview");
}

