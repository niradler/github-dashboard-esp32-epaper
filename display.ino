#ifdef ENABLE_DISPLAY

const unsigned char github_logo_32x32[] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 
	0xff, 0xc0, 0x03, 0xff, 0xff, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x7f, 
	0xfc, 0xc0, 0x03, 0x3f, 0xf8, 0xf7, 0xef, 0x9f, 0xf0, 0xff, 0xff, 0x8f, 
	0xe0, 0xff, 0xff, 0x87, 0xe0, 0xff, 0xff, 0x87, 0xc1, 0xff, 0xff, 0x83, 
	0xc1, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 
	0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc1, 0xff, 0xff, 0xc3, 
	0xc1, 0xff, 0xff, 0x83, 0xc0, 0xff, 0xff, 0x83, 0xc0, 0x7f, 0xfe, 0x03, 
	0xe0, 0x1f, 0xf8, 0x47, 0xe0, 0x07, 0xe0, 0xc7, 0xe0, 0x0f, 0xf1, 0x87, 
	0xf0, 0x0f, 0xff, 0x0f, 0xf8, 0x0f, 0xff, 0x1f, 0xfc, 0x0f, 0xf0, 0x3f, 
	0xfe, 0x0f, 0xf0, 0x7f, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0xcf, 0xf3, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char icon_review_16x16[] PROGMEM = {
  0x03, 0xc0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3e, 0x7c, 0x7c, 0x3e, 0x78, 0x1e, 
  0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0x78, 0x1e, 0x7c, 0x3e, 
  0x3e, 0x7c, 0x1f, 0xf8, 0x0f, 0xf0, 0x03, 0xc0
};

const unsigned char icon_mention_16x16[] PROGMEM = {
  0x03, 0xc0, 0x0c, 0x30, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x47, 0xe2, 
  0x8c, 0x31, 0x88, 0x11, 0x88, 0x11, 0x8c, 0x31, 0x47, 0xe2, 0x40, 0x02, 
  0x20, 0x04, 0x10, 0x08, 0x0c, 0x30, 0x03, 0xc0
};

const unsigned char icon_assign_16x16[] PROGMEM = {
  0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 
  0x07, 0xe0, 0x03, 0xc0, 0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe, 0x7f, 0xfe, 
  0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0
};

const unsigned char icon_other_16x16[] PROGMEM = {
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80
};

const unsigned char icon_commits_16x16[] PROGMEM = {
  0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x0f, 0xf0, 0x1f, 0xf8,
  0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0,
  0x03, 0xc0, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0
};

const unsigned char icon_week_16x16[] PROGMEM = {
  0xff, 0xff, 0x80, 0x01, 0x80, 0x01, 0x9c, 0x71, 0x9c, 0x71, 0x9c, 0x71,
  0x9c, 0x71, 0x9c, 0x71, 0x9c, 0x71, 0x9c, 0x71, 0x9c, 0x71, 0x9c, 0x71,
  0x9c, 0x71, 0x80, 0x01, 0x80, 0x01, 0xff, 0xff
};

const unsigned char icon_streak_16x16[] PROGMEM = {
  0x01, 0x80, 0x03, 0x80, 0x07, 0x00, 0x0e, 0x00, 0x1c, 0x00, 0x38, 0x00,
  0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x00, 0x38, 0x00, 0x1c,
  0x00, 0x0e, 0x00, 0x07, 0x01, 0xc0, 0x01, 0x80
};

const unsigned char icon_total_16x16[] PROGMEM = {
  0x0f, 0xf0, 0x1f, 0xf8, 0x38, 0x1c, 0x70, 0x0e, 0x60, 0x06, 0xe0, 0x07,
  0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xe0, 0x07, 0x60, 0x06,
  0x70, 0x0e, 0x38, 0x1c, 0x1f, 0xf8, 0x0f, 0xf0
};

const unsigned char icon_repos_16x16[] PROGMEM = {
  0x7f, 0xfe, 0x40, 0x02, 0x40, 0x02, 0x5f, 0xfa, 0x50, 0x0a, 0x50, 0x0a,
  0x50, 0x0a, 0x5f, 0xfa, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02,
  0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x7f, 0xfe
};

const unsigned char icon_stars_16x16[] PROGMEM = {
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
  0x31, 0x8c, 0x19, 0x98, 0x0f, 0xf0, 0x07, 0xe0, 0x07, 0xe0, 0x0f, 0xf0,
  0x1e, 0x78, 0x3c, 0x3c, 0x78, 0x1e, 0x70, 0x0e
};

const unsigned char icon_prs_16x16[] PROGMEM = {
  0x7f, 0xfe, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06,
  0x60, 0x06, 0x7f, 0xfe, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x01, 0x80,
  0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x0f, 0xf0
};

const unsigned char icon_followers_16x16[] PROGMEM = {
  0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x03, 0xc0, 0x1f, 0xf8,
  0x3f, 0xfc, 0x3f, 0xfc, 0x1f, 0xf8, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
  0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe, 0x7f, 0xfe
};

const unsigned char icon_pr_open_16x16[] PROGMEM = {
  0x7f, 0xfe, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06,
  0x60, 0x06, 0x7f, 0xfe, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0,
  0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8
};

const unsigned char icon_pr_ready_16x16[] PROGMEM = {
  0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x00, 0x0e, 0x00, 0x1c, 0x00, 0x38,
  0x40, 0x70, 0x60, 0xe0, 0x71, 0xc0, 0x7b, 0x80, 0x3f, 0x00, 0x1e, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char icon_pr_review_16x16[] PROGMEM = {
  0x07, 0xe0, 0x18, 0x18, 0x20, 0x04, 0x40, 0x02, 0x40, 0x02, 0x80, 0x01,
  0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02, 0x40, 0x02,
  0x20, 0x04, 0x18, 0x18, 0x07, 0xe0, 0x00, 0x00
};

const unsigned char icon_pr_changes_16x16[] PROGMEM = {
  0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f, 0xfc,
  0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8,
  0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80
};

void initDisplay() {
  Serial.println("[DISPLAY] Init e-paper...");
  display.init(115200);
  display.setRotation(1);
  display.setFullWindow();
  
  u8g2Fonts.begin(display);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setFontDirection(0);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(10, 30);
    display.print("GitHub");
    
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(10, 60);
    display.print("Notification");
    display.setCursor(10, 85);
    display.print("Dashboard");
  } while (display.nextPage());
  
  Serial.println("[DISPLAY] Ready");
}

void drawGitHubLogo(int x, int y, int size) {
  display.fillCircle(x + size/2, y + size/2, size/2, GxEPD_BLACK);
  int innerSize = size * 0.7;
  display.fillCircle(x + size/2, y + size/2, innerSize/2, GxEPD_WHITE);
  int dotSize = size * 0.15;
  display.fillCircle(x + size/2, y + size/2, dotSize, GxEPD_BLACK);
}

String getFormattedTime(time_t timestamp = 0) {
  struct tm timeinfo;
  if (timestamp == 0) {
    if (!getLocalTime(&timeinfo)) {
      return "No time";
    }
  } else {
    localtime_r(&timestamp, &timeinfo);
  }
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
  return String(buffer);
}

String getFormattedDate(time_t timestamp = 0) {
  struct tm timeinfo;
  if (timestamp == 0) {
    if (!getLocalTime(&timeinfo)) {
      return "No date";
    }
  } else {
    localtime_r(&timestamp, &timeinfo);
  }
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%d/%m %H:%M", &timeinfo);
  return String(buffer);
}

String getTimeAgo(time_t timestamp) {
  if (timestamp == 0) return "Never";
  
  time_t now;
  time(&now);
  int diff = (int)difftime(now, timestamp);
  
  if (diff < 60) return String(diff) + "s ago";
  if (diff < 3600) return String(diff / 60) + "m ago";
  if (diff < 86400) return String(diff / 3600) + "h ago";
  return String(diff / 86400) + "d ago";
}

float getBatteryVoltage() {
  int raw = analogRead(BATTERY_ADC);
  return (raw / 4095.0) * 2.0 * 3.3;
}

int getBatteryPercentage() {
  float voltage = getBatteryVoltage();
  if (voltage >= 4.2) return 100;
  if (voltage <= 3.3) return 0;
  return (int)((voltage - 3.3) / (4.2 - 3.3) * 100);
}

void drawWiFiBars(int x, int y, int rssi) {
  int bars = 0;
  if (rssi >= -60) bars = 4;
  else if (rssi >= -70) bars = 3;
  else if (rssi >= -80) bars = 2;
  else if (rssi >= -90) bars = 1;
  
  for (int i = 0; i < 4; i++) {
    int barHeight = 2 + (i * 2);
    int barX = x + (i * 3);
    int barY = y - barHeight;
    
    if (i < bars) {
      display.fillRect(barX, barY, 2, barHeight, GxEPD_BLACK);
    } else {
      display.drawRect(barX, barY, 2, barHeight, GxEPD_BLACK);
    }
  }
}

void drawBatteryBars(int x, int y, int percentage) {
  int bars = 0;
  if (percentage >= 75) bars = 4;
  else if (percentage >= 50) bars = 3;
  else if (percentage >= 25) bars = 2;
  else if (percentage >= 10) bars = 1;
  
  display.drawRect(x - 1, y - 8, 14, 9, GxEPD_BLACK);
  display.fillRect(x + 13, y - 6, 2, 5, GxEPD_BLACK);
  
  for (int i = 0; i < 4; i++) {
    if (i < bars) {
      display.fillRect(x + (i * 3), y - 7, 2, 7, GxEPD_BLACK);
    }
  }
}

void drawFooter(DisplayPrinter& printer) {
  display.drawLine(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20, GxEPD_BLACK);
  
  printer.setFont(u8g2_font_6x10_tf);
  printer.setCursorY(SCREEN_HEIGHT - 6);
  printer.setCursorX(3);
  
  if (wifiConnected) {
    String ssid = WiFi.SSID();
    if (ssid.length() > 10) {
      ssid = ssid.substring(0, 10);
    }
    printer.print(ssid);
    
    int rssi = WiFi.RSSI();
    drawWiFiBars(printer.getCursorX() + 4, SCREEN_HEIGHT - 4, rssi);
  } else {
    printer.print("NO WiFi");
  }
  
  printer.setCursorX(90);
  if (providers[GITHUB].username.length() > 0) {
    String username = providers[GITHUB].username;
    if (username.length() > 12) {
      username = username.substring(0, 12);
    }
    printer.print("@" + username);
  }
  
  printer.setCursorX(175);
  printer.print(getFormattedTime());
  
  int batteryPercent = getBatteryPercentage();
  drawBatteryBars(SCREEN_WIDTH - 18, SCREEN_HEIGHT - 4, batteryPercent);
}

void drawCategoryCell(DisplayPrinter& printer, const unsigned char* icon, const char* label, int count, int xOffset, int y) {
  const int cellWidth = 115;
  const int iconX = xOffset + 5;
  const int labelX = iconX + 20;
  
  if (icon != nullptr) {
    display.drawBitmap(iconX, y, icon, 16, 16, GxEPD_BLACK);
  }
  
  printer.setFont(u8g2_font_helvB10_tf);
  printer.setCursorX(labelX);
  printer.setCursorY(y + 12);
  printer.print(label);
  
  String countStr = String(count);
  int digitWidth = 9;
  int numberWidth = countStr.length() * digitWidth;
  int rightEdge = xOffset + cellWidth;
  int countX = rightEdge - numberWidth - 3;
  
  printer.setFont(u8g2_font_helvB14_tn);
  printer.setCursorX(countX);
  printer.setCursorY(y + 12);
  printer.print(count);
}

bool shouldUpdateDisplay() {
  if (!wifiConnected) return true;
  
  if (currentScreen == SCREEN_NOTIFICATIONS) {
    int total = providers[GITHUB].notificationCount;
    int reviews = providers[GITHUB].reviewRequests;
    int mentions = providers[GITHUB].mentions;
    int assigned = providers[GITHUB].assignments;
    int other = providers[GITHUB].otherReasons;
    
    if (total != lastDisplayedTotal ||
        reviews != lastDisplayedReviews ||
        mentions != lastDisplayedMentions ||
        assigned != lastDisplayedAssignments ||
        other != lastDisplayedOther) {
      return true;
    }
  } else if (currentScreen == SCREEN_PROFILE) {
    int repos = profileData.publicRepos;
    int stars = profileData.totalStars;
    int prs = profileData.openPRs;
    int followers = profileData.followers;
    
    if (repos != lastDisplayedPublicRepos ||
        stars != lastDisplayedTotalStars ||
        prs != lastDisplayedProfileOpenPRs ||
        followers != lastDisplayedFollowers) {
      return true;
    }
  } else if (currentScreen == SCREEN_PR_OVERVIEW) {
    int openPRs = prData.openPRs;
    int ready = prData.readyToMerge;
    int awaiting = prData.awaitingReview;
    int changes = prData.changesRequested;
    
    if (openPRs != lastDisplayedOpenPRs ||
        ready != lastDisplayedReadyToMerge ||
        awaiting != lastDisplayedAwaitingReview ||
        changes != lastDisplayedChangesRequested) {
      return true;
    }
  }
  
  return false;
}

void drawNotificationScreen() {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    if (wifiConnected) {
      int total = providers[GITHUB].notificationCount;
      int reviews = providers[GITHUB].reviewRequests;
      int mentions = providers[GITHUB].mentions;
      int assigned = providers[GITHUB].assignments;
      int other = providers[GITHUB].otherReasons;
      
      for (int y = 0; y < LOGO_SIZE; y++) {
        for (int x = 0; x < LOGO_SIZE; x++) {
          int byteIndex = (y * 4) + (x / 8);
          int bitIndex = 7 - (x % 8);
          bool isBlack = !(github_logo_32x32[byteIndex] & (1 << bitIndex));
          if (isBlack) {
            display.drawPixel(LOGO_X + x, LOGO_Y + y, GxEPD_BLACK);
          }
        }
      }
      
      DisplayPrinter printer(&display, &u8g2Fonts);
      printer.setMargins(TEXT_START_X, 30, 10);
      printer.setFont(u8g2_font_helvB10_tf);
      printer.setLineHeight(LINE_HEIGHT);
      
      printer.print("GitHub Notifications  ( ");
      printer.print(total);
      printer.println(" )");
      
      const int col1X = 5;
      const int col2X = 125;
      const int row1Y = LIST_START_Y;
      const int row2Y = LIST_START_Y + 22;
      
      drawCategoryCell(printer, icon_review_16x16, "Review", reviews, col1X, row1Y);
      drawCategoryCell(printer, icon_mention_16x16, "Mention", mentions, col2X, row1Y);
      drawCategoryCell(printer, icon_assign_16x16, "Assigned", assigned, col1X, row2Y);
      drawCategoryCell(printer, icon_other_16x16, "Other", other, col2X, row2Y);
      
      drawFooter(printer);
      
    } else {
      DisplayPrinter printer(&display, &u8g2Fonts);
      printer.setCursorX(65);
      printer.setCursorY(80);
      printer.setFont(u8g2_font_helvB14_tf);
      printer.println("Setup Mode");
      
      printer.setCursorX(70);
      printer.setFont(u8g2_font_helvR10_tf);
      printer.print("192.168.4.1");
    }
  } while (display.nextPage());
}

void updateDisplay(bool forceUpdate) {
  if (!forceUpdate && !shouldUpdateDisplay()) {
    Serial.println("[DISPLAY] No changes, skipping update");
    return;
  }
  
  Serial.println("[DISPLAY] Updating...");
  
  if (currentScreen == SCREEN_NOTIFICATIONS) {
    drawNotificationScreen();
    lastDisplayedTotal = providers[GITHUB].notificationCount;
    lastDisplayedReviews = providers[GITHUB].reviewRequests;
    lastDisplayedMentions = providers[GITHUB].mentions;
    lastDisplayedAssignments = providers[GITHUB].assignments;
    lastDisplayedOther = providers[GITHUB].otherReasons;
  } else if (currentScreen == SCREEN_PROFILE) {
    drawProfileScreen();
    lastDisplayedPublicRepos = profileData.publicRepos;
    lastDisplayedTotalStars = profileData.totalStars;
    lastDisplayedProfileOpenPRs = profileData.openPRs;
    lastDisplayedFollowers = profileData.followers;
  } else if (currentScreen == SCREEN_PR_OVERVIEW) {
    drawPROverviewScreen();
    lastDisplayedOpenPRs = prData.openPRs;
    lastDisplayedReadyToMerge = prData.readyToMerge;
    lastDisplayedAwaitingReview = prData.awaitingReview;
    lastDisplayedChangesRequested = prData.changesRequested;
  }
}

void showWiFiConnecting() {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    DisplayPrinter printer(&display, &u8g2Fonts);
    printer.setCursorX(10);
    printer.setCursorY(50);
    printer.setFont(u8g2_font_helvB12_tf);
    printer.println("Connecting");
    printer.setCursorX(10);
    printer.print("WiFi...");
  } while (display.nextPage());
}

void showConfigMode() {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    DisplayPrinter printer(&display, &u8g2Fonts);
    printer.setMargins(5, 25, 5);
    printer.setFont(u8g2_font_helvB12_tf);
    printer.setLineHeight(20);
    
    printer.println("Setup Mode");
    printer.println("WiFi:");
    printer.println("NotificationHub");
    printer.println("IP:192.168.4.1");
  } while (display.nextPage());
}

void showError(String message) {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    DisplayPrinter printer(&display, &u8g2Fonts);
    printer.setMargins(10, 40, 10);
    printer.setFont(u8g2_font_helvB14_tf);
    printer.setLineHeight(25);
    
    printer.println("Error!");
    printer.setFont(u8g2_font_helvB10_tf);
    printer.printWrapped(message.c_str());
  } while (display.nextPage());
}

void drawProfileScreen() {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    if (wifiConnected) {
      int repos = profileData.publicRepos;
      int stars = profileData.totalStars;
      int prs = profileData.openPRs;
      int followers = profileData.followers;
      
      for (int y = 0; y < LOGO_SIZE; y++) {
        for (int x = 0; x < LOGO_SIZE; x++) {
          int byteIndex = (y * 4) + (x / 8);
          int bitIndex = 7 - (x % 8);
          bool isBlack = !(github_logo_32x32[byteIndex] & (1 << bitIndex));
          if (isBlack) {
            display.drawPixel(LOGO_X + x, LOGO_Y + y, GxEPD_BLACK);
          }
        }
      }
      
      DisplayPrinter printer(&display, &u8g2Fonts);
      printer.setMargins(TEXT_START_X, 30, 10);
      printer.setFont(u8g2_font_helvB10_tf);
      printer.setLineHeight(LINE_HEIGHT);
      
      printer.print("GitHub Profile");
      
      const int col1X = 5;
      const int col2X = 125;
      const int row1Y = LIST_START_Y;
      const int row2Y = LIST_START_Y + 22;
      
      drawCategoryCell(printer, icon_repos_16x16, "Repos", repos, col1X, row1Y);
      drawCategoryCell(printer, icon_stars_16x16, "Stars", stars, col2X, row1Y);
      drawCategoryCell(printer, icon_prs_16x16, "Open PRs", prs, col1X, row2Y);
      drawCategoryCell(printer, icon_followers_16x16, "Followers", followers, col2X, row2Y);
      
      drawFooter(printer);
      
    } else {
      DisplayPrinter printer(&display, &u8g2Fonts);
      printer.setCursorX(65);
      printer.setCursorY(80);
      printer.setFont(u8g2_font_helvB14_tf);
      printer.println("Setup Mode");
      
      printer.setCursorX(70);
      printer.setFont(u8g2_font_helvR10_tf);
      printer.print("192.168.4.1");
    }
  } while (display.nextPage());
}

void drawPROverviewScreen() {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    if (wifiConnected) {
      int openPRs = prData.openPRs;
      int ready = prData.readyToMerge;
      int awaiting = prData.awaitingReview;
      int changes = prData.changesRequested;
      
      for (int y = 0; y < LOGO_SIZE; y++) {
        for (int x = 0; x < LOGO_SIZE; x++) {
          int byteIndex = (y * 4) + (x / 8);
          int bitIndex = 7 - (x % 8);
          bool isBlack = !(github_logo_32x32[byteIndex] & (1 << bitIndex));
          if (isBlack) {
            display.drawPixel(LOGO_X + x, LOGO_Y + y, GxEPD_BLACK);
          }
        }
      }
      
      DisplayPrinter printer(&display, &u8g2Fonts);
      printer.setMargins(TEXT_START_X, 30, 10);
      printer.setFont(u8g2_font_helvB10_tf);
      printer.setLineHeight(LINE_HEIGHT);
      
      printer.print("PR Overview");
      
      const int col1X = 5;
      const int col2X = 125;
      const int row1Y = LIST_START_Y;
      const int row2Y = LIST_START_Y + 22;
      
      drawCategoryCell(printer, icon_pr_open_16x16, "Open", openPRs, col1X, row1Y);
      drawCategoryCell(printer, icon_pr_ready_16x16, "Ready", ready, col2X, row1Y);
      drawCategoryCell(printer, icon_pr_review_16x16, "Review", awaiting, col1X, row2Y);
      drawCategoryCell(printer, icon_pr_changes_16x16, "Changes", changes, col2X, row2Y);
      
      drawFooter(printer);
      
    } else {
      DisplayPrinter printer(&display, &u8g2Fonts);
      printer.setCursorX(65);
      printer.setCursorY(80);
      printer.setFont(u8g2_font_helvB14_tf);
      printer.println("Setup Mode");
      
      printer.setCursorX(70);
      printer.setFont(u8g2_font_helvR10_tf);
      printer.print("192.168.4.1");
    }
  } while (display.nextPage());
}

#endif

