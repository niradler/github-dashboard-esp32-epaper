# GitHub Notification Dashboard - E-Paper Edition

A smart notification dashboard for GitHub notifications running on ESP32 with an e-paper display. This device fetches your GitHub notifications and displays them in a clean, categorized format on a 2.13" e-paper display.

## Features

- **Real-time GitHub Notifications**: Fetches and displays unread GitHub notifications
- **Category Breakdown**: Organizes notifications by type:
  - Review Requests
  - Mentions
  - Assignments
  - Other notifications
- **E-Paper Display**: Low-power 2.13" black & white e-paper screen
- **Deep Sleep Support**: Energy-efficient operation with configurable wake intervals
- **Web Interface**: Configure WiFi, API tokens, and settings via browser
- **WiFi Auto-Connect**: Remembers your WiFi credentials
- **Persistent Storage**: All settings saved to flash memory

## Hardware Requirements

- **ESP32** development board
- **GxEPD2_213_BN** 2.13" e-paper display (250x122 pixels)
- **Wiring**:
  - CS: GPIO 5
  - DC: GPIO 17
  - RST: GPIO 16
  - BUSY: GPIO 4
- **Optional Buttons**:
  - Refresh: GPIO 0
  - Wake: GPIO 39
- **Status LED**: GPIO 2

## Getting Started

### Initial Setup

1. Upload the firmware to your ESP32
2. The device will start in **Setup Mode** (creates WiFi AP)
3. Connect to WiFi network: `NotificationHub` (password: `configure`)
4. Open browser and navigate to: `http://192.168.4.1`

### Configuration

#### WiFi Setup
1. Go to the **WiFi** tab in the web interface
2. Enter your WiFi SSID and password
3. Set an admin password for security
4. Click **Save WiFi** (device will reboot)

#### GitHub Integration
1. Generate a GitHub Personal Access Token:
   - Go to GitHub Settings → Developer settings → Personal access tokens
   - Create a **Classic token** with `notifications` scope
2. In the web interface, go to **Providers** tab
3. Enable GitHub provider
4. Paste your token
5. Enter admin password and click **Save Providers**

#### Settings
- **Update Interval**: How often to check for notifications (default: 10 minutes)
- Adjust in the **Settings** tab

## Usage

### Display Information

The e-paper screen shows:
- GitHub logo
- Total notification count
- Category breakdown with icons:
  - 🔍 Reviews
  - @ Mentions
  - 👤 Assignments
  - ! Other
- Footer with:
  - WiFi signal strength (dBm)
  - Last update time
  - Current time

### Manual Refresh

- Press the **Refresh button** (GPIO 0) to force update
- Press the **Wake button** (GPIO 39) to wake from sleep and refresh
- Use the **Refresh Now** button in the web interface

### Web Interface Endpoints

- `GET /` - Main configuration page
- `GET /api/status` - Current status and notification counts
- `POST /api/config` - Save WiFi/settings
- `GET /api/providers` - List configured providers
- `POST /api/providers` - Update provider settings
- `POST /api/refresh` - Trigger manual refresh
- `POST /api/reset` - Factory reset (requires admin password)

## Power Management

### Deep Sleep Mode

When enabled (default), the device:
1. Wakes up every N minutes (configurable)
2. Connects to WiFi
3. Fetches notifications
4. Updates display
5. Enters deep sleep

**Wake Sources**:
- Timer (based on update interval)
- Button press on GPIO 39

**Web Server Timeout**: Device stays awake for 30 seconds after boot to allow web access, then sleeps

### Disabling Sleep
Set `SLEEP_ENABLED false` in the code to disable deep sleep (useful for debugging)

## Configuration Constants

```cpp
UPDATE_INTERVAL_MS      = 9 * 60 * 1000  // Main loop interval (deprecated in favor of config)
GITHUB_MAX_PAGES        = 25              // Max pages to fetch from GitHub
GITHUB_PER_PAGE         = 13              // Notifications per page
JSON_BUFFER_SIZE        = 8192            // JSON parsing buffer
HTTP_TIMEOUT_MS         = 10000           // HTTP request timeout
WEB_SERVER_TIMEOUT      = 30000           // Stay awake time (ms)
```

## Default Credentials

**Initial Config** (stored in Preferences):
- WiFi SSID: _(empty - configure in setup mode)_
- WiFi Password: _(empty - configure in setup mode)_
- Admin Password: `admin`

**AP Mode** (when not configured):
- SSID: `NotificationHub`
- Password: `configure`

**⚠️ WARNING**: Change default admin password immediately in production!

## Serial Monitor Output

Connect at **115200 baud** to see detailed logs:
- System initialization
- WiFi connection status
- API requests and responses
- Notification parsing
- Display updates
- Power management events

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password in web interface
- Check signal strength (device may be too far from router)
- Monitor Serial output for connection errors

### GitHub API Issues
- **401 Error**: Invalid token - regenerate GitHub token
- **403 Error**: Rate limited - wait or check token permissions
- **Empty Response**: Network issue or API down

### Display Not Updating
- Check wiring connections (CS, DC, RST, BUSY)
- Verify `ENABLE_DISPLAY` is defined
- Look for display errors in Serial monitor

### Memory Issues
- Reduce `GITHUB_MAX_PAGES` if running out of heap
- Monitor "Free Heap" in Serial output
- Consider reducing `JSON_BUFFER_SIZE`

### Factory Reset
1. Go to web interface
2. Use `/api/reset` endpoint
3. Provide admin password
4. Device will clear all settings and reboot

## Development

### Required Libraries

```cpp
WiFi.h                      // ESP32 WiFi
WiFiClientSecure.h          // HTTPS support
HTTPClient.h                // HTTP requests
WebServer.h                 // Web interface
Preferences.h               // Persistent storage
ArduinoJson.h              // JSON parsing
time.h                     // Time functions
GxEPD2_BW.h                // E-paper display driver
U8g2_for_Adafruit_GFX.h    // Font rendering
```

### Display Architecture

**DisplayPrinter Class**: A text layout manager that handles:
- Automatic line wrapping
- Margin management
- Font switching
- Cursor positioning
- Bitmap drawing

### Memory Optimization

The code uses several techniques to manage ESP32's limited memory:
- Dynamic JSON documents with size limits
- Paged API requests to avoid large payloads
- String reuse and careful buffer management
- Display hibernation during sleep

## API Rate Limits

GitHub API rate limits:
- **5,000 requests/hour** for authenticated requests
- This device makes 1 request per page per update cycle
- With default settings (10-minute intervals, 25 max pages):
  - Worst case: 25 requests every 10 minutes = 150 requests/hour
  - Well under the limit

## Security Considerations

1. **Change default passwords** immediately
2. Store GitHub tokens securely (they're in flash memory)
3. Use HTTPS for all API requests (✓ implemented)
4. Consider enabling WPA3 on your WiFi network
5. Don't expose the web interface to the internet

## Contributing

This is an embedded device project. When modifying:
- Test memory usage thoroughly
- Verify deep sleep functionality
- Check display refresh behavior
- Monitor Serial output during changes

## License

MIT License - Feel free to modify and distribute

## Acknowledgments

- GxEPD2 library for e-paper display support
- ArduinoJson for efficient JSON parsing
- ESP32 Arduino framework
- GitHub API v3

---

**Version**: 1.0  
**Last Updated**: November 2025

