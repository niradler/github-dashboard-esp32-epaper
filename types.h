#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

struct Config {
  String wifi_ssid;
  String wifi_password;
  String admin_password;
  int update_interval;
  bool configured;
  String wifi_ap_password;
};

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

struct ProfileData {
  int publicRepos;
  int totalStars;
  int openPRs;
  int followers;
};

enum ProviderType {
  GITHUB,
  MAX_PROVIDERS
};

enum ScreenType {
  SCREEN_NOTIFICATIONS,
  SCREEN_PROFILE,
  SCREEN_PR_OVERVIEW,
  MAX_SCREENS
};

struct HttpResponse {
  int code;
  bool success;
};

#endif

