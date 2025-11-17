# Development Guide - GitHub Dashboard E-Paper

## Project Overview

ESP32-based GitHub notification dashboard with e-paper display. Single Arduino sketch file (`sketch.ino`) containing ~1350 lines of embedded C++ code.

## Architecture

### Core Components

1. **Hardware Layer**
   - ESP32 microcontroller
   - GxEPD2_213_BN e-paper display (250x122px)
   - GPIO buttons (refresh, wake)
   - Status LED

2. **Software Stack**
   - WiFi management (STA + AP modes)
   - HTTPS client for GitHub API
   - Web server for configuration UI
   - Preferences API for persistent storage
   - Deep sleep power management

3. **Data Flow**
```
Timer/Button → WiFi Connect → GitHub API (HTTPS) → JSON Parse → 
Categorize Notifications → Update Display → Deep Sleep
```

### Key Classes & Structures

**Config Struct**: WiFi credentials, admin password, update interval
**NotificationProvider Struct**: Provider state, API tokens, notification counts
**DisplayPrinter Class**: Custom text layout engine for e-paper

## Working with Arduino MCP

This project uses the Arduino CLI MCP server for development workflows.

### MCP Configuration

Located in `.cursor/mcp.json`:
```json
{
  "mcpServers": {
    "arduino-cli-mcp": {
      "command": "uvx",
      "args": ["arduino-mcp"],
      "env": {}
    }
  }
}
```

### Arduino MCP Capabilities

The Arduino MCP server provides tools for:
- **Board management**: Install/list cores
- **Library management**: Search/install libraries
- **Compilation**: Build sketches with board-specific configs
- **Upload**: Flash compiled code to ESP32
- **Serial monitor**: Debug and monitor device output
- **Board detection**: Auto-detect connected boards
- **Board info**: Get board details (FQBN, port, etc.)

### Common Workflows

#### 1. Initial Board Setup
```
1. Detect board → Get FQBN and port
2. Install ESP32 core if needed
3. Install required libraries
4. Compile sketch
5. Upload to board
```

#### 2. Development Cycle
```
1. Modify sketch.ino
2. Save changes
3. Compile with target board FQBN
4. Check for compilation errors
5. Upload if compilation succeeds
6. Monitor serial output (115200 baud)
7. Repeat
```

#### 3. Debugging
```
1. Open serial monitor at 115200 baud
2. Watch for tagged log messages:
   - [SYSTEM], [WIFI], [GITHUB], [DISPLAY], [WEB], etc.
3. Identify issues from logs
4. Modify code
5. Recompile and upload
```

### Essential Commands via MCP

Ask the AI to:
- "Detect the connected Arduino board"
- "Compile this sketch for ESP32"
- "Upload the code to the board"
- "Open serial monitor at 115200 baud"
- "Install ArduinoJson library"
- "List installed boards"

### Board Configuration

**Target Board**: ESP32 Dev Module (or compatible)
**FQBN**: `esp32:esp32:esp32` (or board-specific variant)
**Upload Speed**: 115200 or higher
**Required Core**: `esp32` by Espressif Systems

### Required Libraries

Must be installed via Arduino Library Manager:
- `GxEPD2` - E-paper display driver
- `ArduinoJson` - JSON parsing (v6+)
- `U8g2_for_Adafruit_GFX` - Font rendering
- Built-in: WiFi, HTTPClient, WebServer, Preferences

## Code Organization

### Main Sections (line ranges approximate)

1. **Includes & Constants** (1-88): Libraries, pins, display dimensions
2. **Display Assets** (109-146): Bitmaps for GitHub logo and icons
3. **DisplayPrinter Class** (149-304): Text layout engine
4. **Display Functions** (308-598): Init, drawing, status screens
5. **Setup/Loop** (600-761): Core Arduino lifecycle
6. **Config Management** (763-847): Load/save preferences
7. **WiFi Functions** (849-907): Connection, AP mode
8. **Provider Updates** (909-1140): GitHub API integration
9. **Web Server** (1142-1347): HTTP handlers and HTML page

### Critical Functions

**updateGitHub()**: Paged API fetching, handles rate limits, parses notifications
**updateDisplay()**: Smart refresh (only updates if data changed)
**goToDeepSleep()**: Power management, wake on timer/button
**DisplayPrinter**: Word wrapping, margin handling, font management

## Development Best Practices

### 1. Memory Management

ESP32 has limited heap (~200KB free typically). Always:
- Monitor `ESP.getFreeHeap()` and `ESP.getMaxAllocHeap()`
- Use `DynamicJsonDocument` with size limits
- Delete `WiFiClientSecure` after use
- Avoid large static buffers
- Page through API results instead of loading all at once

### 2. Display Updates

E-paper displays have limited refresh cycles:
- Use `shouldUpdateDisplay()` to skip unnecessary updates
- Only call `updateDisplay()` when data actually changes
- Avoid partial updates in this implementation (uses full refresh)
- Display hibernates during deep sleep to save power

### 3. Power Optimization

Deep sleep is crucial for battery operation:
- Configure appropriate wake intervals
- Use `esp_sleep_enable_timer_wakeup()` for scheduled wake
- Use `esp_sleep_enable_ext0_wakeup()` for button wake
- Keep awake time minimal (default 30s after boot)
- Hibernate display before sleep

### 4. API Best Practices

GitHub API considerations:
- Use `Bearer` token authentication
- Include `User-Agent` header
- Handle rate limiting (HTTP 403)
- Implement paging for large notification lists
- Add delays between page requests
- Use HTTPS (setInsecure() for simplicity, consider cert validation for production)

### 5. Serial Logging

Structured logging for debugging:
```cpp
Serial.println("[TAG] Message");
// Tags: SYSTEM, WIFI, GITHUB, DISPLAY, WEB, CONFIG, PROVIDERS, etc.
```
- Always flush before deep sleep: `Serial.flush()`
- Log important state changes and errors
- Include metric values (timing, memory, counts)

### 6. Error Handling

Robust error management:
- Check all API response codes
- Validate JSON parsing results
- Handle WiFi disconnections gracefully
- Store error messages in provider struct
- Display errors on e-paper when critical

## Testing Strategies

### 1. Hardware Testing

- **Display**: Call `initDisplay()` and verify initialization messages
- **Buttons**: Test debouncing and response in `loop()`
- **WiFi**: Test both STA and AP modes
- **Power**: Verify deep sleep current draw with multimeter

### 2. API Testing

- **Valid Token**: Should return notifications
- **Invalid Token**: Should show 401 error
- **Rate Limit**: Simulate by making many requests
- **Paging**: Test with account that has 25+ notifications
- **Empty Response**: Test with account with no notifications

### 3. Web Interface Testing

- **Config Mode**: Test initial setup flow
- **WiFi Config**: Change SSID/password and verify reboot
- **Provider Setup**: Enable/disable, update tokens
- **Admin Auth**: Verify password protection on sensitive endpoints
- **Factory Reset**: Test complete data wipe

### 4. Integration Testing

Full lifecycle test:
1. Flash device with fresh code
2. Connect to setup AP
3. Configure WiFi via web interface
4. Device reboots and connects
5. Configure GitHub token
6. Wait for first update
7. Verify display shows notifications
8. Press refresh button
9. Verify display updates
10. Wait for deep sleep
11. Device wakes on timer
12. Repeat cycle

## Common Issues & Solutions

### Issue: Compilation Errors

**Solution**: 
- Verify all libraries are installed
- Check ESP32 core is up to date
- Ensure FQBN matches your board
- Look for missing semicolons or braces (C++ syntax)

### Issue: Upload Fails

**Solution**:
- Check USB cable connection
- Verify correct port is selected
- Hold BOOT button during upload (some ESP32 boards)
- Reduce upload speed if unstable
- Check for other programs using the serial port

### Issue: WiFi Won't Connect

**Solution**:
- Verify SSID/password are correct
- Check 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Monitor serial output for detailed errors
- Move closer to router
- Check router settings (MAC filtering, etc.)

### Issue: Out of Memory

**Solution**:
- Reduce `JSON_BUFFER_SIZE` (line 15)
- Lower `GITHUB_MAX_PAGES` (line 13)
- Check for memory leaks (WiFiClientSecure not deleted)
- Monitor heap with `ESP.getFreeHeap()`

### Issue: Display Not Refreshing

**Solution**:
- Check `shouldUpdateDisplay()` logic
- Verify display wiring (especially BUSY pin)
- Look for display-related errors in serial
- Check if `ENABLE_DISPLAY` is defined
- Verify display initialization succeeds

### Issue: GitHub API Returns 403

**Solution**:
- Rate limited - wait and retry
- Check token permissions (needs `notifications` scope)
- Verify token is still valid on GitHub
- Add longer delays between requests

### Issue: Deep Sleep Not Working

**Solution**:
- Verify `SLEEP_ENABLED` is true
- Check `allowDeepSleep` flag is set
- Ensure not in config mode (`isConfigMode == false`)
- Monitor serial for sleep messages
- Check if web server timeout has elapsed

## Extending the Project

### Adding New Notification Providers

1. Add entry to `ProviderType` enum
2. Initialize in `initProviders()`
3. Implement update function (like `updateGitHub()`)
4. Add case in `updateProvider()` switch
5. Update web interface to show new provider

### Custom Display Layouts

1. Modify `updateDisplay()` function
2. Use `DisplayPrinter` for text layout
3. Use `display.drawBitmap()` for icons
4. Adjust `SCREEN_WIDTH`, `SCREEN_HEIGHT` constants
5. Test wrapping and positioning

### Adding Sensors

1. Define sensor pins in constants
2. Initialize in `setup()`
3. Read values in `loop()` or before sleep
4. Display sensor data in `updateDisplay()`
5. Optionally store in Preferences

### Alternative Displays

1. Replace GxEPD2 includes with new driver
2. Modify `initDisplay()` for new hardware
3. Update draw commands in display functions
4. Adjust screen dimensions
5. Test refresh rates and power consumption

## Code Style & Conventions

### Naming
- **Constants**: `UPPER_SNAKE_CASE`
- **Variables**: `camelCase`
- **Functions**: `camelCase`
- **Structs**: `PascalCase`
- **Defines**: `UPPER_SNAKE_CASE`

### Comments
- Use comments sparingly (per user preference)
- Explain "why" not "what"
- Document complex algorithms
- Add section headers for code organization

### Pin Definitions
Always use `#define` for GPIO pins:
```cpp
#define BUTTON_REFRESH 0
#define LED_STATUS 2
```

### Error Handling
Always check return values:
```cpp
if (!client) {
  Serial.println("[TAG] Error message");
  return;
}
```

## Serial Output Format

Standard log format:
```
[TAG] Message
[TAG] Key: Value
[TAG] ✓ Success message
[TAG] ✗ Error message
```

Common tags:
- `[SYSTEM]`: Boot, reset, system info
- `[GPIO]`: Pin configuration
- `[WIFI]`: WiFi connection events
- `[DISPLAY]`: Display operations
- `[GITHUB]`: API requests/responses
- `[WEB]`: Web server requests
- `[CONFIG]`: Configuration changes
- `[PROVIDERS]`: Provider management
- `[POWER]`: Sleep/wake events
- `[TIMER]`: Scheduled update events
- `[BUTTON]`: Button press events

## MCP-Assisted Development Workflow

### Recommended Workflow with Arduino MCP

1. **Start Session**
   - Ask AI: "Detect the connected board"
   - Verify FQBN and port

2. **Make Changes**
   - Edit `sketch.ino` using AI assistance
   - AI can read/modify code, explain functions, suggest improvements

3. **Compile**
   - Ask AI: "Compile this sketch"
   - Review any compilation errors
   - AI can fix errors automatically

4. **Upload**
   - Ask AI: "Upload the code to the board"
   - Monitor upload progress

5. **Debug**
   - Ask AI: "Open serial monitor"
   - Observe output at 115200 baud
   - AI can interpret logs and suggest fixes

6. **Iterate**
   - Request changes: "Add feature X"
   - AI modifies code, compiles, uploads
   - Repeat until desired behavior achieved

### Advanced MCP Usage

**Library Management**:
- "Search for GxEPD2 library"
- "Install ArduinoJson library"
- "List installed libraries"

**Board Management**:
- "List available ESP32 boards"
- "Show board details for current device"
- "Install ESP32 platform core"

**Multi-file Projects**:
- This project uses single sketch file
- For larger projects, can split into multiple `.ino` files
- Arduino IDE automatically combines `.ino` files

**Configuration**:
- Ask AI to modify constants
- Change pins, intervals, buffer sizes
- AI will recompile and upload automatically

## Troubleshooting with MCP

If compilation fails:
1. Check error messages in MCP response
2. Ask AI: "Fix the compilation error"
3. AI will identify and correct issues

If upload fails:
1. Verify board is connected: "Detect board"
2. Check port is correct
3. Ask AI to retry with different settings

If behavior is unexpected:
1. "Open serial monitor"
2. Share log output with AI
3. Ask: "What's wrong based on these logs?"
4. AI will diagnose and suggest fixes

## Security Notes

### Credential Management

**Stored in Flash** (Preferences API):
- WiFi SSID/password
- Admin password (plaintext)
- GitHub API tokens (plaintext)

**Risks**:
- Anyone with physical access can extract credentials
- Use strong, unique passwords
- Rotate tokens regularly

**Mitigations**:
- Change default credentials immediately
- Don't expose web interface to internet
- Use least-privilege GitHub tokens (only `notifications` scope)
- Consider encrypting preferences (advanced)

### Web Interface Security

**Current Implementation**:
- Simple password authentication
- No HTTPS on web interface
- Credentials sent in POST body

**Production Improvements**:
- Implement HTTPS on ESP32
- Add CSRF tokens
- Rate limit login attempts
- Session management
- BCrypt password hashing

### GitHub Token Security

**Best Practices**:
- Use Classic Personal Access Token
- Enable only `notifications` scope
- Set expiration date
- Monitor token usage on GitHub
- Revoke token if device is compromised

## Performance Metrics

Typical measurements:
- **Boot time**: 2-3 seconds
- **WiFi connect**: 5-15 seconds
- **GitHub API request**: 1-3 seconds per page
- **Display update**: 2-3 seconds (full refresh)
- **Sleep current**: <1mA (deep sleep)
- **Active current**: 80-150mA (WiFi on, display updating)

## Version Control

This project has minimal git structure. Recommendations:

**Essential files**:
- `sketch.ino` (main code)
- `.cursor/mcp.json` (MCP config)
- `README.md` (user documentation)
- `llm.md` (this file)

**Should be ignored** (`.gitignore`):
- `*.bin` (compiled binaries)
- `build/` (compilation artifacts)
- `.DS_Store`, `.vscode/`, etc.

**Commit practices**:
- Commit before major changes
- Test code before committing
- Write descriptive messages
- Tag releases (v1.0, v1.1, etc.)

## Resources

### Documentation
- ESP32 Arduino Core: https://docs.espressif.com/projects/arduino-esp32/
- GxEPD2 Library: https://github.com/ZinggJM/GxEPD2
- ArduinoJson: https://arduinojson.org/
- GitHub API: https://docs.github.com/en/rest/activity/notifications

### Tools
- Arduino CLI: Command-line interface for Arduino
- Arduino MCP: MCP server for Arduino CLI integration
- Serial Monitor: Built into Arduino IDE or use screen/minicom
- ESPTool: For direct ESP32 flashing

### Community
- ESP32 Forums: https://www.esp32.com/
- Arduino Forums: https://forum.arduino.cc/
- GitHub Discussions: For this specific project

## Quick Reference

### Pin Map
| Function | GPIO | Notes |
|----------|------|-------|
| Display CS | 5 | E-paper chip select |
| Display DC | 17 | Data/command |
| Display RST | 16 | Reset |
| Display BUSY | 4 | Busy signal |
| Refresh Button | 0 | Active low, pull-up |
| Wake Button | 39 | Active low, pull-up |
| Status LED | 2 | High = connected |

### Default Settings
| Setting | Value |
|---------|-------|
| Update Interval | 10 minutes |
| Max GitHub Pages | 25 |
| Per Page | 13 notifications |
| HTTP Timeout | 10 seconds |
| Web Server Timeout | 30 seconds |
| Serial Baud | 115200 |

### Key Functions
| Function | Purpose |
|----------|---------|
| `setup()` | Initialize hardware, load config, connect WiFi |
| `loop()` | Handle buttons, update timer, manage sleep |
| `updateAllProviders()` | Fetch notifications from all providers |
| `updateGitHub()` | Fetch GitHub notifications with paging |
| `updateDisplay()` | Refresh e-paper with current data |
| `goToDeepSleep()` | Enter low-power mode |
| `handleRoot()` | Serve web interface HTML |

---

**This is a living document** - update as the project evolves. Always test changes thoroughly before deploying to hardware.

