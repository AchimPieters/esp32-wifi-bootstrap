# ESP32 WiFi Bootstrap

**ESP32 WiFi Bootstrap** is a lightweight, high-performance Wi-Fi provisioning solution for the ESP32. It provides a user-friendly captive portal experience with fast configuration, persistent storage, and full compatibility with ESP-IDF 5.4 or later.



## Simple. Reliable. Ready.

ESP32 WiFi Bootstrap makes network provisioning seamless. Whether you're deploying devices in the field or at home, this component ensures that your ESP32 can connect quickly and reliably—even after power loss.



## Features

- Captive portal with CNA detection (iOS/macOS compatible)  
- DNS redirect for easy browser launch  
- Wi-Fi scan with secure/unsecure distinction  
- Persistent NVS storage for saved networks  
- Auto-reconnect on boot  
- SoftAP fallback with configurable SSID  
- Lightweight embedded UI (chunked HTML output)  
- Fully compatible with **ESP-IDF 5.4+**    



## Example App

The [`example`](example/) folder contains a full working project.

- Starts Wi-Fi bootstrap on boot.
- Attempts to reconnect to saved networks.
- If no known networks are available, it launches SoftAP + captive portal.
- After configuration, the device automatically reconnects and reboots into station mode.

### Build & Flash

```bash
idf.py set-target esp32
idf.py menuconfig     # (Optional)
idf.py build
idf.py flash monitor
```

Once running, the ESP32 will broadcast a SoftAP (default: `Homekit`). Connect from any device and navigate to any URL. The captive portal will appear.



## UI Development

The interface lives in [`content/index.html`](content/index.html), written in **Jinja2** template syntax. It’s embedded into firmware using a custom converter.

You can preview and test the UI locally with a built-in Flask development server.

### Setup

```bash
pip install flask
```

### Run server

```bash
python tools/server.py
```

### Preview in browser

- `http://localhost:8080/settings` — shows UI with found networks  
- `http://localhost:8080/settings0` — simulates empty scan results



## Template Compilation

The file `tools/generate_index_html_header.py` transforms the HTML template into C headers.

### How it works:

- Splits `index.html` into parts using `<!-- part PART_NAME -->` comments.
- Removes all `{% ... %}` Jinja2 logic blocks.
- Replaces `{{ ... }}` output expressions with `%s` placeholders.
- Outputs the following sections:
  - `HTML_SETTINGS_HEADER`
  - `HTML_SETTINGS_FOOTER`
  - `HTML_NETWORK_ITEM`

`HTML_NETWORK_ITEM` expects two `%s` placeholders:
1. `"secure"` or `"unsecure"`
2. The SSID name

These fragments are injected into the firmware and rendered efficiently using `printf()`-style formatting.



## Integration

Copy the `esp32-wifi-bootstrap` component into your ESP-IDF project and add it to your `CMakeLists.txt`.  

> Coming soon: Available via the [ESP Component Registry](https://components.espressif.com)

