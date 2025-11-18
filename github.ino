void updateAllProviders() {
  Serial.println("\n[UPDATE] Starting update...");
  
  activeProviders = 0;
  for (int i = 0; i < MAX_PROVIDERS; i++) {
    if (providers[i].enabled && providers[i].apiToken.length() > 0) {
      activeProviders++;
    }
  }
  
  if (currentScreen == SCREEN_NOTIFICATIONS) {
    for (int i = 0; i < MAX_PROVIDERS; i++) {
      if (providers[i].enabled && providers[i].apiToken.length() > 0) {
        updateProvider(i);
        delay(1000);
      }
    }
    
    totalNotifications = 0;
    for (int i = 0; i < MAX_PROVIDERS; i++) {
      if (providers[i].enabled) {
        totalNotifications += providers[i].notificationCount;
      }
    }
    
    Serial.print("[UPDATE] Complete - Total: ");
    Serial.println(totalNotifications);
  } else if (currentScreen == SCREEN_PROFILE) {
    if (providers[GITHUB].enabled && providers[GITHUB].apiToken.length() > 0) {
      updateGitHubProfile(GITHUB);
    }
    Serial.println("[UPDATE] Profile data updated");
  } else if (currentScreen == SCREEN_ACTIVITY) {
    if (providers[GITHUB].enabled && providers[GITHUB].apiToken.length() > 0) {
      updateGitHubActivity(GITHUB);
    }
    Serial.println("[UPDATE] Activity data updated");
  }
  
  time(&lastUpdateTimestamp);
  
  #ifdef ENABLE_DISPLAY
  updateDisplay(false);
  #endif
}

struct HttpResponse {
  int code;
  bool success;
};

HttpResponse makeHttpsRequest(WiFiClientSecure* client, const String& url, const String& authToken, DynamicJsonDocument* doc, const String& userAgent = "ESP32-NotificationHub") {
  HttpResponse response = {0, false};
  
  HTTPClient* https = new HTTPClient();
  if (!https) {
    Serial.println("[HTTP] Failed to create HTTPClient");
    return response;
  }
  
  if (!https->begin(*client, url)) {
    Serial.println("[HTTP] Connection failed");
    delete https;
    return response;
  }
  
  https->addHeader("Authorization", "Bearer " + authToken);
  https->addHeader("User-Agent", userAgent);
  https->addHeader("Accept", "application/vnd.github+json");
  https->setTimeout(HTTP_TIMEOUT_MS);
  
  response.code = https->GET();
  
  if (response.code == 200 && doc != nullptr) {
    String payload = https->getString();
    DeserializationError error = deserializeJson(*doc, payload);
    payload = "";
    
    if (!error) {
      response.success = true;
    } else {
      Serial.print("[HTTP] JSON parse error: ");
      Serial.println(error.c_str());
    }
  }
  
  https->end();
  delete https;
  
  return response;
}

void updateProvider(int idx) {
  Serial.print("[PROVIDER] Updating ");
  Serial.print(providers[idx].displayName);
  Serial.println("...");
  unsigned long startTime = millis();
  
  switch(idx) {
    case GITHUB:
      updateGitHub(idx);
      break;
    default:
      Serial.println("[PROVIDER] Not implemented");
      providers[idx].lastError = "Not implemented";
      break;
  }
  
  unsigned long duration = millis() - startTime;
  Serial.print("[PROVIDER] Update took ");
  Serial.print(duration);
  Serial.println("ms");
}

void updateGitHub(int idx) {
  Serial.print("[GITHUB] Heap: ");
  Serial.println(ESP.getFreeHeap());
  
  WiFiClientSecure *client = new WiFiClientSecure;
  if (!client) {
    Serial.println("[GITHUB] ✗ Out of memory");
    providers[idx].lastError = "Out of memory";
    return;
  }
  client->setInsecure();
  
  if (providers[idx].username.length() == 0) {
    DynamicJsonDocument userDoc(1024);
    HttpResponse userResp = makeHttpsRequest(client, "https://api.github.com/user", providers[idx].apiToken, &userDoc);
    if (userResp.success && userDoc.containsKey("login")) {
      providers[idx].username = userDoc["login"].as<String>();
      Serial.print("[GITHUB] Username: ");
      Serial.println(providers[idx].username);
    }
    userDoc.clear();
  }
  
  int totalUnread = 0;
  int totalChecked = 0;
  int reviews = 0;
  int mentionCount = 0;
  int assignCount = 0;
  int otherCount = 0;
  int page = 1;
  char urlBuffer[128];
  
  while (page <= GITHUB_MAX_PAGES) {
    snprintf(urlBuffer, sizeof(urlBuffer), "https://api.github.com/notifications?per_page=%d&page=%d", GITHUB_PER_PAGE, page);
    
    Serial.print("[GITHUB] Page ");
    Serial.print(page);
    Serial.print(" - Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.print(", Max alloc: ");
    Serial.println(ESP.getMaxAllocHeap());
    
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    HttpResponse pageResp = makeHttpsRequest(client, String(urlBuffer), providers[idx].apiToken, &doc);
    
    if (!pageResp.success) {
      if (page == 1) {
        Serial.print("[GITHUB] ✗ HTTP ");
        Serial.println(pageResp.code);
        if (pageResp.code == 401) providers[idx].lastError = "Invalid token";
        else if (pageResp.code == 403) providers[idx].lastError = "Rate limited";
        else if (pageResp.code <= 0) providers[idx].lastError = "Request failed";
        else providers[idx].lastError = "HTTP error";
      }
      doc.clear();
      break;
    }
    
    if (!doc.is<JsonArray>()) {
      if (page == 1) {
        Serial.println("[GITHUB] ✗ Not an array");
        providers[idx].lastError = "Invalid response";
      }
      doc.clear();
      break;
    }
    
    JsonArray notifications = doc.as<JsonArray>();
    int pageCount = notifications.size();
    
    if (pageCount == 0) {
      doc.clear();
      break;
    }
    
    for (size_t i = 0; i < pageCount; i++) {
      totalChecked++;
      
      JsonVariant unreadVar = notifications[i]["unread"];
      if (!unreadVar.isNull() && unreadVar.as<bool>()) {
        totalUnread++;
        
        const char* reason = notifications[i]["reason"];
        if (strcmp(reason, "review_requested") == 0) {
          reviews++;
        } else if (strcmp(reason, "mention") == 0) {
          mentionCount++;
        } else if (strcmp(reason, "assign") == 0) {
          assignCount++;
        } else {
          otherCount++;
        }
      }
    }
    
    Serial.print("[GITHUB] Page ");
    Serial.print(page);
    Serial.print(": +");
    Serial.println(pageCount);
    
    if (pageCount < GITHUB_PER_PAGE) {
      break;
    }
    
    doc.clear();
    page++;
    delay(200);
    yield();
  }
  
  delete client;
  
  Serial.print("[GITHUB] Final heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" bytes, Max alloc: ");
  Serial.println(ESP.getMaxAllocHeap());
  
  providers[idx].notificationCount = totalUnread;
  providers[idx].reviewRequests = reviews;
  providers[idx].mentions = mentionCount;
  providers[idx].assignments = assignCount;
  providers[idx].otherReasons = otherCount;
  providers[idx].lastUpdate = millis();
  providers[idx].lastError = "";
  
  Serial.print("[GITHUB] ✓ ");
  Serial.print(totalUnread);
  Serial.print("/");
  Serial.print(totalChecked);
  Serial.print(" (");
  Serial.print(page - 1);
  Serial.println(" pages)");
  
  Serial.print("[GITHUB] Reviews: ");
  Serial.print(reviews);
  Serial.print(", Mentions: ");
  Serial.print(mentionCount);
  Serial.print(", Assigned: ");
  Serial.print(assignCount);
  Serial.print(", Other: ");
  Serial.println(otherCount);
}

void updateGitHubActivity(int idx) {
  Serial.println("[GITHUB ACTIVITY] Fetching contribution data...");
  
  WiFiClientSecure *client = new WiFiClientSecure;
  if (!client) {
    Serial.println("[GITHUB ACTIVITY] ✗ Out of memory");
    githubActivity.lastError = "Out of memory";
    return;
  }
  client->setInsecure();
  
  if (providers[idx].username.length() == 0) {
    Serial.println("[GITHUB ACTIVITY] ✗ No username available");
    githubActivity.lastError = "No username";
    delete client;
    return;
  }
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[GITHUB ACTIVITY] ✗ Failed to get time");
    githubActivity.lastError = "No time";
    delete client;
    return;
  }
  
  char todayStr[11];
  char fromDateStr[11];
  strftime(todayStr, sizeof(todayStr), "%Y-%m-%d", &timeinfo);
  
  time_t now = time(nullptr);
  time_t weekAgo = now - (7 * 24 * 60 * 60);
  localtime_r(&weekAgo, &timeinfo);
  strftime(fromDateStr, sizeof(fromDateStr), "%Y-%m-%d", &timeinfo);
  
  String query = "{\"query\":\"query{user(login:\\\"" + providers[idx].username + "\\\"){contributionsCollection(from:\\\"" + String(fromDateStr) + "T00:00:00Z\\\",to:\\\"" + String(todayStr) + "T23:59:59Z\\\"){contributionCalendar{totalContributions weeks{contributionDays{contributionCount date}}}}}}}\"}";
  
  HTTPClient *https = new HTTPClient();
  if (!https) {
    Serial.println("[GITHUB ACTIVITY] ✗ Failed to create HTTPClient");
    delete client;
    return;
  }
  
  if (!https->begin(*client, "https://api.github.com/graphql")) {
    Serial.println("[GITHUB ACTIVITY] ✗ Connection failed");
    delete https;
    delete client;
    return;
  }
  
  https->addHeader("Authorization", "Bearer " + providers[idx].apiToken);
  https->addHeader("Content-Type", "application/json");
  https->addHeader("User-Agent", "ESP32-NotificationHub");
  https->setTimeout(HTTP_TIMEOUT_MS);
  
  int httpCode = https->POST(query);
  
  if (httpCode == 200) {
    String payload = https->getString();
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      JsonObject data = doc["data"]["user"]["contributionsCollection"];
      int weekTotal = data["contributionCalendar"]["totalContributions"] | 0;
      
      int todayCommits = 0;
      int currentStreak = 0;
      int totalContribs = 0;
      
      JsonArray weeks = data["contributionCalendar"]["weeks"].as<JsonArray>();
      if (!weeks.isNull()) {
        bool streakActive = true;
        int daysChecked = 0;
        
        for (int w = weeks.size() - 1; w >= 0 && daysChecked < 30; w--) {
          JsonArray days = weeks[w]["contributionDays"].as<JsonArray>();
          for (int d = days.size() - 1; d >= 0 && daysChecked < 30; d--) {
            String date = days[d]["date"].as<String>();
            int count = days[d]["contributionCount"] | 0;
            
            if (date == String(todayStr)) {
              todayCommits = count;
            }
            
            totalContribs += count;
            
            if (daysChecked == 0 || streakActive) {
              if (count > 0) {
                currentStreak++;
              } else if (daysChecked > 0) {
                streakActive = false;
              }
            }
            
            daysChecked++;
          }
        }
      }
      
      githubActivity.todayCommits = todayCommits;
      githubActivity.weekContributions = weekTotal;
      githubActivity.currentStreak = currentStreak;
      githubActivity.totalContributions = totalContribs;
      githubActivity.lastUpdate = millis();
      githubActivity.lastError = "";
      
      Serial.print("[GITHUB ACTIVITY] ✓ Today: ");
      Serial.print(todayCommits);
      Serial.print(", Week: ");
      Serial.print(weekTotal);
      Serial.print(", Streak: ");
      Serial.print(currentStreak);
      Serial.print("d, Total (30d): ");
      Serial.println(totalContribs);
      
    } else {
      Serial.print("[GITHUB ACTIVITY] ✗ JSON parse error: ");
      Serial.println(error.c_str());
      githubActivity.lastError = "Parse error";
    }
  } else {
    Serial.print("[GITHUB ACTIVITY] ✗ HTTP ");
    Serial.println(httpCode);
    githubActivity.lastError = "HTTP error";
  }
  
  https->end();
  delete https;
  delete client;
}

void updateGitHubProfile(int idx) {
  Serial.println("[GITHUB PROFILE] Fetching profile data...");
  
  WiFiClientSecure *client = new WiFiClientSecure;
  if (!client) {
    Serial.println("[GITHUB PROFILE] ✗ Out of memory");
    return;
  }
  client->setInsecure();
  
  if (providers[idx].username.length() == 0) {
    Serial.println("[GITHUB PROFILE] ✗ No username available");
    delete client;
    return;
  }
  
  String query = "{\"query\":\"query{user(login:\\\"" + providers[idx].username + "\\\"){repositories(first:100,ownerAffiliations:OWNER){totalCount nodes{stargazerCount}}pullRequests(first:1,states:OPEN){totalCount}followers{totalCount}}}\"}";
  
  HTTPClient *https = new HTTPClient();
  if (!https) {
    Serial.println("[GITHUB PROFILE] ✗ Failed to create HTTPClient");
    delete client;
    return;
  }
  
  if (!https->begin(*client, "https://api.github.com/graphql")) {
    Serial.println("[GITHUB PROFILE] ✗ Connection failed");
    delete https;
    delete client;
    return;
  }
  
  https->addHeader("Authorization", "Bearer " + providers[idx].apiToken);
  https->addHeader("Content-Type", "application/json");
  https->addHeader("User-Agent", "ESP32-NotificationHub");
  https->setTimeout(HTTP_TIMEOUT_MS);
  
  int httpCode = https->POST(query);
  
  if (httpCode == 200) {
    String payload = https->getString();
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      JsonObject user = doc["data"]["user"];
      
      int repos = user["repositories"]["totalCount"] | 0;
      int openPRs = user["pullRequests"]["totalCount"] | 0;
      int followers = user["followers"]["totalCount"] | 0;
      
      int totalStars = 0;
      JsonArray repoNodes = user["repositories"]["nodes"].as<JsonArray>();
      if (!repoNodes.isNull()) {
        for (JsonObject repo : repoNodes) {
          totalStars += repo["stargazerCount"] | 0;
        }
      }
      
      profileData.publicRepos = repos;
      profileData.totalStars = totalStars;
      profileData.openPRs = openPRs;
      profileData.followers = followers;
      
      Serial.print("[GITHUB PROFILE] ✓ Repos: ");
      Serial.print(repos);
      Serial.print(", Stars: ");
      Serial.print(totalStars);
      Serial.print(", Open PRs: ");
      Serial.print(openPRs);
      Serial.print(", Followers: ");
      Serial.println(followers);
      
    } else {
      Serial.print("[GITHUB PROFILE] ✗ JSON parse error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("[GITHUB PROFILE] ✗ HTTP ");
    Serial.println(httpCode);
  }
  
  https->end();
  delete https;
  delete client;
}

