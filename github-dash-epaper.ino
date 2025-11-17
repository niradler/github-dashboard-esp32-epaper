#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>

#define ENABLE_DISPLAY

// Configuration Constants
const int UPDATE_INTERVAL_MS = 1 * 60 * 1000;
const int GITHUB_MAX_PAGES = 25;
const int GITHUB_PER_PAGE = 13;
const int JSON_BUFFER_SIZE = 8192;
const unsigned long HTTP_TIMEOUT_MS = 10000;

#ifdef ENABLE_DISPLAY
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>

GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(/*CS=5*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
#endif

WebServer server(80);

// Preferences (persistent storage)
Preferences preferences;

// Configuration
struct Config {
  String wifi_ssid;
  String wifi_password;
  String admin_password;
  int update_interval;
  bool configured;
} config;

// Provider structure
struct NotificationProvider {
  String name;
  String displayName;
  bool enabled;
  String apiToken;
  String apiUrl;
  int notificationCount;
  String lastError;
  unsigned long lastUpdate;
  int reviewRequests;
  int mentions;
  int assignments;
  int otherReasons;
};

// Supported providers
enum ProviderType {
  GITHUB,
  MAX_PROVIDERS
};

NotificationProvider providers[MAX_PROVIDERS];

// Button pins
#define BUTTON_REFRESH 0
#define BUTTON_WAKEUP 39
#define LED_STATUS 2

// Deep sleep settings
#define SLEEP_ENABLED false
#define WEB_SERVER_TIMEOUT 30000

// Display constants
#ifdef ENABLE_DISPLAY
const int SCREEN_WIDTH = 250;
const int SCREEN_HEIGHT = 122;
const int LOGO_X = 5;
const int LOGO_Y = 5;
const int LOGO_SIZE = 32;
const int TEXT_START_X = LOGO_X + LOGO_SIZE + 10;
const int LIST_START_Y = 55;
const int LINE_HEIGHT = 14;
const int ICON_X = 10;
const int LABEL_X = 32;
const int COUNT_X = SCREEN_WIDTH - 25;
#endif

// Global state
bool isConfigMode = false;
unsigned long lastUpdateTime = 0;
time_t lastUpdateTimestamp = 0;
bool wifiConnected = false;
unsigned long wakeupTime = 0;
bool allowDeepSleep = false;
int totalNotifications = 0;
int activeProviders = 0;

#ifdef ENABLE_DISPLAY
int lastDisplayedTotal = -1;
int lastDisplayedReviews = -1;
int lastDisplayedMentions = -1;
int lastDisplayedAssignments = -1;
int lastDisplayedOther = -1;
#endif

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
#endif

#ifdef ENABLE_DISPLAY
class DisplayPrinter {
private:
  GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>* display;
  U8G2_FOR_ADAFRUIT_GFX* u8g2;
  int16_t cursorX, cursorY;
  int16_t marginLeft, marginRight, marginTop;
  int16_t lineHeight;
  int16_t maxWidth;
  const uint8_t* currentFont;
  
public:
  DisplayPrinter(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>* disp, U8G2_FOR_ADAFRUIT_GFX* u8g2_instance)
    : display(disp), u8g2(u8g2_instance) {
    reset();
  }
  
  void reset() {
    cursorX = 0;
    cursorY = 0;
    marginLeft = 0;
    marginRight = 0;
    marginTop = 0;
    lineHeight = 18;
    maxWidth = SCREEN_WIDTH;
    currentFont = u8g2_font_helvB10_tf;
  }
  
  void setMargins(int16_t left, int16_t top, int16_t right) {
    marginLeft = left;
    marginTop = top;
    marginRight = right;
    maxWidth = SCREEN_WIDTH - marginLeft - marginRight;
    cursorX = marginLeft;
    cursorY = marginTop;
  }
  
  void setFont(const uint8_t* font) {
    currentFont = font;
    u8g2->setFont(font);
  }
  
  void setLineHeight(int16_t height) {
    lineHeight = height;
  }
  
  void setCursorX(int16_t x) {
    cursorX = x;
  }
  
  void setCursorY(int16_t y) {
    cursorY = y;
  }
  
  int16_t getCursorX() {
    return cursorX;
  }
  
  int16_t getCursorY() {
    return cursorY;
  }
  
  void nextLine() {
    cursorY += lineHeight;
    cursorX = marginLeft;
  }
  
  void print(const char* text) {
    if (text == nullptr || strlen(text) == 0) return;
    
    u8g2->setFont(currentFont);
    int16_t textWidth = u8g2->getUTF8Width(text);
    
    if (cursorX + textWidth > marginLeft + maxWidth) {
      nextLine();
    }
    
    u8g2->setCursor(cursorX, cursorY);
    u8g2->print(text);
    cursorX += textWidth;
  }
  
  void print(String text) {
    print(text.c_str());
  }
  
  void print(int num) {
    String numStr = String(num);
    print(numStr.c_str());
  }
  
  void println(const char* text) {
    print(text);
    nextLine();
  }
  
  void println(String text) {
    print(text.c_str());
    nextLine();
  }
  
  void println() {
    nextLine();
  }
  
  void printWrapped(const char* text) {
    if (text == nullptr || strlen(text) == 0) return;
    
    u8g2->setFont(currentFont);
    String str = String(text);
    int lastSpace = -1;
    int lineStart = 0;
    
    for (int i = 0; i <= str.length(); i++) {
      if (i == str.length() || str[i] == ' ' || str[i] == '\n') {
        String word = str.substring(lineStart, i);
        int16_t wordWidth = u8g2->getUTF8Width(word.c_str());
        
        if (cursorX + wordWidth > marginLeft + maxWidth && lineStart > 0) {
          nextLine();
          String restOfLine = str.substring(lineStart, i);
          u8g2->setCursor(cursorX, cursorY);
          u8g2->print(restOfLine.c_str());
          cursorX += u8g2->getUTF8Width(restOfLine.c_str());
        } else {
          u8g2->setCursor(cursorX, cursorY);
          u8g2->print(word.c_str());
          cursorX += wordWidth;
        }
        
        if (i < str.length() && str[i] == ' ') {
          int16_t spaceWidth = u8g2->getUTF8Width(" ");
          u8g2->setCursor(cursorX, cursorY);
          u8g2->print(" ");
          cursorX += spaceWidth;
        }
        
        if (i < str.length() && str[i] == '\n') {
          nextLine();
        }
        
        lineStart = i + 1;
      }
    }
  }
  
  void drawBitmap(const unsigned char* bitmap, int16_t w, int16_t h, int16_t x_override = -1) {
    int16_t x = (x_override >= 0) ? x_override : cursorX;
    int16_t y = cursorY - h + 2;
    
    display->drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);
    
    if (x_override < 0) {
      cursorX += w + 2;
    }
  }
};
#endif

#ifdef ENABLE_DISPLAY
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

void drawFooter(DisplayPrinter& printer) {
  display.drawLine(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20, GxEPD_BLACK);
  
  printer.setCursorY(SCREEN_HEIGHT - 6);
  printer.setCursorX(3);
  printer.setFont(u8g2_font_6x10_tf);
  
  String ssid = WiFi.SSID();
  if (ssid.length() > 10) {
    ssid = ssid.substring(0, 10);
  }
  printer.print(ssid);
  
  int rssi = WiFi.RSSI();
  drawWiFiBars(printer.getCursorX() + 4, SCREEN_HEIGHT - 4, rssi);
  
  printer.setCursorX(100);
  if (lastUpdateTimestamp > 0) {
    printer.print(getFormattedDate(lastUpdateTimestamp));
  } else {
    printer.print("--/-- --:--");
  }
  
  printer.setCursorX(195);
  printer.print(getFormattedTime());
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
    
    lastDisplayedTotal = total;
    lastDisplayedReviews = reviews;
    lastDisplayedMentions = mentions;
    lastDisplayedAssignments = assigned;
    lastDisplayedOther = other;
    
    return true;
  }
  
  return false;
}

void updateDisplay() {
  if (!shouldUpdateDisplay()) {
    Serial.println("[DISPLAY] No changes, skipping update");
    return;
  }
  
  Serial.println("[DISPLAY] Updating...");
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
#endif

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n");
  Serial.println("=====================================");
  Serial.println("=== ESP32 Notification Hub v1.0 ===");
  Serial.println("=====================================");
  Serial.println();
  Serial.println("[SYSTEM] Starting initialization...");
  Serial.println("[SYSTEM] Chip Model: " + String(ESP.getChipModel()));
  Serial.println("[SYSTEM] CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
  Serial.println("[SYSTEM] Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("[SYSTEM] Flash Size: " + String(ESP.getFlashChipSize()) + " bytes");
  
  Serial.println("\n[GPIO] Configuring pins...");
  pinMode(LED_STATUS, OUTPUT);
  pinMode(BUTTON_REFRESH, INPUT_PULLUP);
  pinMode(BUTTON_WAKEUP, INPUT_PULLUP);
  digitalWrite(LED_STATUS, LOW);
  
  #ifdef ENABLE_DISPLAY
  Serial.println("\n[DISPLAY] Initializing...");
  initDisplay();
  #endif
  
  loadConfig();
  initProviders();
  
  Serial.println("\n[STARTUP] Checking WiFi configuration...");
  if (config.configured && config.wifi_ssid.length() > 0) {
    Serial.println("[STARTUP] Device is configured, connecting to WiFi...");
    connectWiFi(config.wifi_ssid, config.wifi_password);
  } else {
    Serial.println("[STARTUP] Device not configured, starting AP mode...");
    startConfigMode();
  }
  
  setupWebServer();
  
  Serial.println("\n[TIME] Configuring NTP...");
  configTime(7200, 0, "pool.ntp.org");
  Serial.println("[TIME] NTP server: pool.ntp.org (GMT+2)");
  
  Serial.println("\n=====================================");
  Serial.println("[SYSTEM] ✓ Setup complete!");
  Serial.println("=====================================");
  if (wifiConnected) {
    Serial.println("[SYSTEM] Access web interface at:");
    Serial.println("[SYSTEM]   http://" + WiFi.localIP().toString());
  } else if (isConfigMode) {
    Serial.println("[SYSTEM] Access web interface at:");
    Serial.println("[SYSTEM]   http://192.168.4.1");
  }
  Serial.println("=====================================\n");
  
  wakeupTime = millis();
  checkWakeupReason();
  allowDeepSleep = wifiConnected && !isConfigMode;
}

void goToDeepSleep() {
  if (!SLEEP_ENABLED || isConfigMode) return;
  
  unsigned long sleepTime = config.update_interval * 60;
  
  Serial.println("\n[POWER] Preparing for deep sleep...");
  Serial.println("[POWER] Sleep duration: " + String(sleepTime / 60) + " minutes");
  Serial.println("[POWER] Wake on: Timer OR Button (GPIO " + String(BUTTON_WAKEUP) + ")");
  
  #ifdef ENABLE_DISPLAY
  display.hibernate();
  #endif
  
  esp_sleep_enable_timer_wakeup(sleepTime * 1000000ULL);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_WAKEUP, 0);
  
  Serial.println("[POWER] Going to sleep now...");
  Serial.flush();
  
  esp_deep_sleep_start();
}

void checkWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("[WAKE] Woke up by button press on GPIO " + String(BUTTON_WAKEUP));
      digitalWrite(LED_STATUS, HIGH);
      delay(100);
      digitalWrite(LED_STATUS, LOW);
      delay(100);
      digitalWrite(LED_STATUS, HIGH);
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("[WAKE] Woke up by timer");
      break;
    default:
      Serial.println("[WAKE] Cold boot - not from deep sleep");
      allowDeepSleep = false;
      break;
  }
}

void loop() {
  server.handleClient();
  
  if (digitalRead(BUTTON_REFRESH) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_REFRESH) == LOW) {
      Serial.println("\n[BUTTON] Refresh button pressed!");
      updateAllProviders();
      Serial.println("[BUTTON] Waiting for button release...");
      while(digitalRead(BUTTON_REFRESH) == LOW) delay(10);
      Serial.println("[BUTTON] Button released");
      wakeupTime = millis();
    }
  }
  
  if (digitalRead(BUTTON_WAKEUP) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_WAKEUP) == LOW) {
      Serial.println("\n[BUTTON] Wakeup button pressed - forcing refresh!");
      updateAllProviders();
      Serial.println("[BUTTON] Waiting for button release...");
      while(digitalRead(BUTTON_WAKEUP) == LOW) delay(10);
      Serial.println("[BUTTON] Button released");
      wakeupTime = millis();
    }
  }
  
  if (wifiConnected && !isConfigMode) {
    unsigned long currentTime = millis();
    unsigned long interval = config.update_interval * 60 * 1000;
    
    if (currentTime - lastUpdateTime >= interval || lastUpdateTime == 0) {
      if (lastUpdateTime == 0) {
        Serial.println("\n[TIMER] First automatic update cycle starting...");
      } else {
        Serial.println("\n[TIMER] Automatic update cycle triggered");
        Serial.println("[TIMER] Interval: " + String(config.update_interval) + " minutes");
      }
      updateAllProviders();
      lastUpdateTime = currentTime;
      unsigned long nextUpdate = (interval - (millis() - lastUpdateTime)) / 1000;
      Serial.println("[TIMER] Next update in " + String(nextUpdate / 60) + " minutes");
    }
    
    if (allowDeepSleep && SLEEP_ENABLED) {
      unsigned long awakeTime = millis() - wakeupTime;
      if (awakeTime > WEB_SERVER_TIMEOUT) {
        goToDeepSleep();
      }
    }
  }
  
  digitalWrite(LED_STATUS, wifiConnected ? HIGH : LOW);
  delay(100);
}

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
  preferences.end();
  
  Serial.println("[CONFIG] Configuration loaded:");
  Serial.println("  WiFi SSID: " + config.wifi_ssid);
  Serial.println("  WiFi Password: " + String(config.wifi_password.length() > 0 ? "***SET***" : "***NOT SET***"));
  Serial.println("  Admin Password: " + String(config.admin_password.length() > 0 ? "***SET***" : "***NOT SET***"));
  Serial.println("  Update Interval: " + String(config.update_interval) + " minutes");
  Serial.println("  Configured: " + String(config.configured ? "YES" : "NO"));
}

void saveConfig() {
  Serial.println("\n[CONFIG] Saving configuration to flash...");
  preferences.begin("config", false);
  preferences.putString("wifi_ssid", config.wifi_ssid);
  preferences.putString("wifi_pass", config.wifi_password);
  preferences.putString("admin_pass", config.admin_password);
  preferences.putInt("update_int", config.update_interval);
  preferences.putBool("configured", config.configured);
  preferences.end();
  Serial.println("[CONFIG] Configuration saved successfully!");
  Serial.println("  WiFi SSID: " + config.wifi_ssid);
  Serial.println("  Update Interval: " + String(config.update_interval) + " minutes");
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

void connectWiFi(String ssid, String password) {
  Serial.println("\n[WIFI] Connecting to WiFi");
  
  #ifdef ENABLE_DISPLAY
  showWiFiConnecting();
  #endif
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  Serial.print("[WIFI] Connecting");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("[WIFI] ✓ Connected!");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_STATUS, HIGH);
    #ifdef ENABLE_DISPLAY
    updateDisplay();
    #endif
  } else {
    wifiConnected = false;
    Serial.println("[WIFI] ✗ Connection failed");
    #ifdef ENABLE_DISPLAY
    showError("WiFi failed");
    #endif
    startConfigMode();
  }
}

void startConfigMode() {
  isConfigMode = true;
  Serial.println("\n[AP MODE] Starting config mode...");
  
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP("NotificationHub", "configure");
  
  if (apStarted) {
    Serial.println("[AP MODE] ✓ AP started");
    Serial.println("[AP MODE] IP: " + WiFi.softAPIP().toString());
    #ifdef ENABLE_DISPLAY
    showConfigMode();
    #endif
  } else {
    Serial.println("[AP MODE] ✗ AP failed");
    #ifdef ENABLE_DISPLAY
    showError("AP failed");
    #endif
  }
}

void updateAllProviders() {
  Serial.println("\n[UPDATE] Starting update...");
  
  activeProviders = 0;
  for (int i = 0; i < MAX_PROVIDERS; i++) {
    if (providers[i].enabled && providers[i].apiToken.length() > 0) {
      activeProviders++;
    }
  }
  
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
  
  time(&lastUpdateTimestamp);
  
  Serial.print("[UPDATE] Complete - Total: ");
  Serial.println(totalNotifications);
  
  #ifdef ENABLE_DISPLAY
  updateDisplay();
  #endif
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
  
  int totalUnread = 0;
  int totalChecked = 0;
  int reviews = 0;
  int mentionCount = 0;
  int assignCount = 0;
  int otherCount = 0;
  int page = 1;
  
  while (page <= GITHUB_MAX_PAGES) {
    HTTPClient https;
    
    String url = "https://api.github.com/notifications?per_page=" + String(GITHUB_PER_PAGE) + "&page=" + String(page);
    
    if (!https.begin(*client, url)) {
      Serial.println("[GITHUB] ✗ Connection failed");
      providers[idx].lastError = "Connection failed";
      delete client;
      return;
    }
    
    https.addHeader("Authorization", "Bearer " + providers[idx].apiToken);
    https.addHeader("User-Agent", "ESP32-NotificationHub");
    https.addHeader("Accept", "application/vnd.github+json");
    https.setTimeout(HTTP_TIMEOUT_MS);
    
    int httpCode = https.GET();
    
    if (httpCode != 200) {
      if (page == 1) {
        Serial.print("[GITHUB] ✗ HTTP ");
        Serial.println(httpCode);
        if (httpCode == 401) providers[idx].lastError = "Invalid token";
        else if (httpCode == 403) providers[idx].lastError = "Rate limited";
        else if (httpCode <= 0) providers[idx].lastError = "Request failed";
        else providers[idx].lastError = "HTTP error";
      }
      https.end();
      break;
    }
    
    Serial.print("[GITHUB] Page ");
    Serial.print(page);
    Serial.print(" - Getting payload...");
    
    int contentLength = https.getSize();
    int freeHeap = ESP.getFreeHeap();
    int maxAlloc = ESP.getMaxAllocHeap();
    
    Serial.printf(" (Size: %d, Free: %d, MaxAlloc: %d)", contentLength, freeHeap, maxAlloc);
    
    if (contentLength > 0 && contentLength > maxAlloc) {
      Serial.println();
      Serial.printf("[GITHUB] ✗ Not enough contiguous memory (%d bytes needed, %d max block)\n", contentLength, maxAlloc);
      https.end();
      break;
    }
    
    String payload = https.getString();
    https.end();
    
    Serial.print(" Got ");
    Serial.println(payload.length());
    
    if (payload.length() == 0) {
      if (page == 1) {
        Serial.println("[GITHUB] ✗ Empty response");
        providers[idx].lastError = "Empty response";
      }
      break;
    }
    
    Serial.print("[GITHUB] Parsing JSON...");
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print(" FAILED: ");
      Serial.println(error.c_str());
      if (page == 1) {
        providers[idx].lastError = "Parse error";
      }
      break;
    } else {
      Serial.println(" OK");
    }
    
    if (!doc.is<JsonArray>()) {
      if (page == 1) {
        Serial.println("[GITHUB] ✗ Not an array");
        providers[idx].lastError = "Invalid response";
      }
      break;
    }
    
    JsonArray notifications = doc.as<JsonArray>();
    int pageCount = notifications.size();
    
    if (pageCount == 0) {
      break;
    }
    
    for (size_t i = 0; i < pageCount; i++) {
      totalChecked++;
      
      JsonVariant unreadVar = notifications[i]["unread"];
      if (!unreadVar.isNull() && unreadVar.as<bool>()) {
        totalUnread++;
        
        String reason = notifications[i]["reason"].as<String>();
        if (reason == "review_requested") {
          reviews++;
        } else if (reason == "mention") {
          mentionCount++;
        } else if (reason == "assign") {
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
    
    page++;
    delay(200);
    yield();
  }
  
  delete client;
  
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
  
#ifdef ENABLE_DISPLAY
  updateDisplay();
#endif
}

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
