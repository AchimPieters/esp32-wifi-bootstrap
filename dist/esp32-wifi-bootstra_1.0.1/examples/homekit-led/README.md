# HomeKit LED Example

This example demonstrates how to use the `esp-wifi-config` component together with
HomeKit to control an LED accessory. The application starts a Wi-Fi configuration
portal if no Wi-Fi credentials are stored, and once connected it launches the
HomeKit server.

```
idf.py -p PORT flash monitor
```

Default configuration values are provided via `sdkconfig.defaults`.
