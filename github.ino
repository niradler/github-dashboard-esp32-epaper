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
  } else if (currentScreen == SCREEN_PR_OVERVIEW) {
    if (providers[GITHUB].enabled && providers[GITHUB].apiToken.length() > 0) {
      updateGitHubPRs(GITHUB);
    }
    Serial.println("[UPDATE] PR data updated");
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

void updateGitHubPRs(int idx) {
  Serial.println("[GITHUB PRs] Fetching PR data...");
  
  WiFiClientSecure *client = new WiFiClientSecure;
  if (!client) {
    Serial.println("[GITHUB PRs] ✗ Out of memory");
    prData.lastError = "Out of memory";
    return;
  }
  client->setInsecure();
  
  if (providers[idx].username.length() == 0) {
    Serial.println("[GITHUB PRs] ✗ No username available");
    prData.lastError = "No username";
    delete client;
    return;
  }
  
  String query = "{\"query\":\"query{search(query:\\\"author:" + providers[idx].username + " is:pr is:open\\\",type:ISSUE,first:100){nodes{...on PullRequest{mergeable reviewDecision}}}}\"}";
  
  HTTPClient *https = new HTTPClient();
  if (!https) {
    Serial.println("[GITHUB PRs] ✗ Failed to create HTTPClient");
    delete client;
    return;
  }
  
  if (!https->begin(*client, "https://api.github.com/graphql")) {
    Serial.println("[GITHUB PRs] ✗ Connection failed");
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
      JsonArray prs = doc["data"]["search"]["nodes"].as<JsonArray>();
      
      int openCount = 0;
      int readyCount = 0;
      int awaitingCount = 0;
      int changesCount = 0;
      
      if (!prs.isNull()) {
        for (JsonObject pr : prs) {
          openCount++;
          
          String mergeable = pr["mergeable"].as<String>();
          String reviewDecision = pr["reviewDecision"].as<String>();
          
          if (mergeable == "MERGEABLE") {
            readyCount++;
          } else if (reviewDecision == "CHANGES_REQUESTED") {
            changesCount++;
          } else if (reviewDecision.isEmpty() || reviewDecision == "null") {
            awaitingCount++;
          }
        }
      }
      
      prData.openPRs = openCount;
      prData.readyToMerge = readyCount;
      prData.awaitingReview = awaitingCount;
      prData.changesRequested = changesCount;
      prData.lastUpdate = millis();
      prData.lastError = "";
      
      Serial.print("[GITHUB PRs] ✓ Open: ");
      Serial.print(openCount);
      Serial.print(", Ready: ");
      Serial.print(readyCount);
      Serial.print(", Awaiting: ");
      Serial.print(awaitingCount);
      Serial.print(", Changes: ");
      Serial.println(changesCount);
      
    } else {
      Serial.print("[GITHUB PRs] ✗ JSON parse error: ");
      Serial.println(error.c_str());
      prData.lastError = "Parse error";
    }
  } else {
    Serial.print("[GITHUB PRs] ✗ HTTP ");
    Serial.println(httpCode);
    prData.lastError = "HTTP error";
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

