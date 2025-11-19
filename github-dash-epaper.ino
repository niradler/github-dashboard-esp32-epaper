#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>

#define ENABLE_DISPLAY

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
Preferences preferences;

struct Config {
  String wifi_ssid;
  String wifi_password;
  String admin_password;
  int update_interval;
  bool configured;
} config;

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
  String username;
};

struct PRData {
  int openPRs;
  int readyToMerge;
  int awaitingReview;
  int changesRequested;
  String lastError;
  unsigned long lastUpdate;
};

enum ProviderType {
  GITHUB,
  MAX_PROVIDERS
};

NotificationProvider providers[MAX_PROVIDERS];
PRData prData;

#define BUTTON_REFRESH 0
#define BUTTON_WAKEUP 39
#define LED_STATUS 2
#define BATTERY_ADC 35

#define SLEEP_ENABLED false
#define WEB_SERVER_TIMEOUT 30000

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

enum ScreenType {
  SCREEN_NOTIFICATIONS,
  SCREEN_PROFILE,
  SCREEN_PR_OVERVIEW,
  MAX_SCREENS
};

int currentScreen = SCREEN_NOTIFICATIONS;
bool isConfigMode = false;
unsigned long lastUpdateTime = 0;
time_t lastUpdateTimestamp = 0;
bool wifiConnected = false;
unsigned long wakeupTime = 0;
bool allowDeepSleep = false;
int totalNotifications = 0;
int activeProviders = 0;
unsigned long lastScreenSwitchTime = 0;
const unsigned long SCREEN_SWITCH_DEBOUNCE = 4000;

#ifdef ENABLE_DISPLAY
void updateDisplay(bool forceUpdate = false);
#endif

#ifdef ENABLE_DISPLAY
int lastDisplayedTotal = -1;
int lastDisplayedReviews = -1;
int lastDisplayedMentions = -1;
int lastDisplayedAssignments = -1;
int lastDisplayedOther = -1;
int lastDisplayedOpenPRs = -1;
int lastDisplayedReadyToMerge = -1;
int lastDisplayedAwaitingReview = -1;
int lastDisplayedChangesRequested = -1;
int lastDisplayedPublicRepos = -1;
int lastDisplayedTotalStars = -1;
int lastDisplayedProfileOpenPRs = -1;
int lastDisplayedFollowers = -1;

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
  pinMode(BUTTON_WAKEUP, INPUT);
  pinMode(BATTERY_ADC, INPUT);
  digitalWrite(LED_STATUS, LOW);
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
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
  
  #ifdef ENABLE_DISPLAY
  float battVoltage = getBatteryVoltage();
  int battPercent = getBatteryPercentage();
  Serial.print("\n[BATTERY] Voltage: ");
  Serial.print(battVoltage, 2);
  Serial.print("V, Level: ");
  Serial.print(battPercent);
  Serial.println("%");
  #endif
  
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
      unsigned long currentTime = millis();
      if (currentTime - lastScreenSwitchTime < SCREEN_SWITCH_DEBOUNCE) {
        Serial.println("\n[BUTTON] Debounced - please wait");
        while(digitalRead(BUTTON_WAKEUP) == LOW) delay(10);
        return;
      }
      
      Serial.println("\n[BUTTON] Wakeup button pressed - switching screen!");
      lastScreenSwitchTime = currentTime;
      
      currentScreen = (currentScreen + 1) % MAX_SCREENS;
      saveCurrentScreen();
      Serial.print("[BUTTON] Switched to screen: ");
      if (currentScreen == SCREEN_NOTIFICATIONS) {
        Serial.println("Notifications");
      } else if (currentScreen == SCREEN_PROFILE) {
        Serial.println("Profile");
      } else {
        Serial.println("PR Overview");
      }
      
      #ifdef ENABLE_DISPLAY
      updateDisplay(true);
      Serial.println("[BUTTON] Screen shown instantly, now fetching fresh data...");
      #endif
      
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
      
      #ifdef ENABLE_DISPLAY
      float battVoltage = getBatteryVoltage();
      int battPercent = getBatteryPercentage();
      Serial.print("[BATTERY] Voltage: ");
      Serial.print(battVoltage, 2);
      Serial.print("V, Level: ");
      Serial.print(battPercent);
      Serial.println("%");
      #endif
      
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
    updateDisplay(true);
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
