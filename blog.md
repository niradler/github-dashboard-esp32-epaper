# Building a GitHub Dashboard on ESP32 E-Paper

I built this mostly because playing with hardware is fun. There's something satisfying about making a physical thing that sits on your desk and actually does something. Plus, let's be honest - having a little e-paper display showing live GitHub stats makes your desk look way more interesting.

Does it solve a problem? Kind of. I can glance at my notifications and contribution stats without opening a browser. But that's the excuse I gave myself. The real reason was wanting to build something cool with an e-paper display and ESP32.

## The Screens

Press GPIO 39 to cycle through:

1. **Notifications** - Reviews, mentions, assignments
2. **Profile** - Repos, stars, followers
3. **PR Overview** - Your open PRs with merge status, CI checks, and review state - helps you spot mergeable PRs without opening GitHub

Each screen fetches fresh data when you switch to it.

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506858274/93307ebe-527a-4486-8140-8a92dc524dad.jpeg align="left")

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506866680/6f13df5f-c80f-445d-acad-7a18df806901.jpeg align="left")

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506873050/43055dd7-a97a-4c02-a726-83e72fdc327f.jpeg align="left")

## Hardware

**LILYGO T5 V2.3.1** (~$15-20) - ESP32 with 2.13" e-paper and two built-in buttons:

- GPIO 39: Cycle screens
- GPIO 0: Force refresh

That's it. Flash and go.

## How It Works

**Data fetching:**

- REST API for notifications
- GraphQL for profile/PRs (smaller payloads)
- Only fetches active screen data
- Only refreshes display when data changes

**Power:**

- Deep sleep between updates (default 10 min)
- Wakes on timer or button press
- Web server runs 30 seconds after boot for config access

## Web Interface

First boot creates AP: "NotificationHub" (password: `configure`)  
Connect and go to `192.168.4.1`

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506896772/d9d701a3-3408-4718-b04f-686530807c94.png align="left")

Four tabs:

- **Dashboard** - Status, refresh button
- **WiFi** - Network config, admin password
- **Providers** - GitHub token and username
- **Settings** - Update interval

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506901536/bab16d42-1bc4-4391-8cf9-31435a7f44f8.png align="left")

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506916311/fa3744ad-3551-4d3b-a300-08e0d5e9b8ed.png align="left")

![](https://cdn.hashnode.com/res/hashnode/image/upload/v1763506906405/e61fb79f-855b-4eb1-a83a-3e28cc3a977a.png align="left")

## GitHub Token

Generate a Classic token with:

- `notifications`
- `read:user`

Add it in Providers tab.

## Setup

1. Flash firmware
2. Connect to "NotificationHub" (password: `configure`)
3. Go to `192.168.4.1` (or whatever your network assigns)
4. Configure WiFi and admin password
5. Add GitHub token and username
6. Reboot

Updates every 10 minutes. Press buttons for manual control.

## Built with Arduino MCP

Used [arduino-mcp](https://github.com/niradler/arduino-mcp) - connects Claude/Cursor to Arduino CLI:

```bash
"Compile for ESP32"
"Upload to board"
"Convert this PNG to C array"
```

Way faster than copy-pasting commands.

Claude Desktop config:

```json
{
  "mcpServers": {
    "arduino-uvx": {
      "command": "uvx",
      "args": ["arduino-mcp"],
      "env": {}
    }
  }
}
```

## API Endpoints

```bash
GET /api/status          # Current status
POST /api/refresh        # Force refresh
POST /api/reset          # Factory reset
```

## Memory Handling

ESP32 RAM is tight:

- Paged API requests
- Dynamic JSON buffers
- Independent screen state
- Display hibernation during sleep

## What's Next

Could add:

- GitLab/Bitbucket support
- Larger displays (4.2" or 7.5")
- Historical graphs
- Battery indicator
- Webhooks for instant updates
- 3D printed [case](https://www.printables.com/model/412141-lilygo-ttgo-t5-213-case)

## Stack

**Hardware:** [LILYGO T5 V2.3.1](https://lilygo.cc/products/t5-2-13inch-e-paper)

**Software:** ESP32 Arduino, GxEPD2, ArduinoJson

**APIs:** GitHub REST + GraphQL, NTP

## Links

**Code:** [github.com/niradler/github-dashboard-esp32-epaper](https://github.com/niradler/github-dashboard-esp32-epaper)

**Arduino MCP:** [github.com/niradler/arduino-mcp](https://github.com/niradler/arduino-mcp)

**License:** MIT | **Cost:** ~$15-20 | **Time:** 1-2 hours
