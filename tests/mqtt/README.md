# MQTT Integration Test Harness

This harness allows you to test the SideEye firmware's MQTT functionality using a local Mosquitto broker.

## Setup

1.  **Run automated tests (recommended):**
    ```bash
    task test:mqtt
    ```

2.  **Manual setup (optional):**
    ```bash
    cd tests/mqtt
    sudo docker compose up -d
    python3 -m venv .venv
    source .venv/bin/activate
    pip install paho-mqtt
    ```

3.  **Run the validator:**
    ```bash
    ./.venv/bin/python validator.py
    ```

## Firmware Configuration

To point your SideEye device to this local broker:

1.  Identify your computer's local IP address (e.g., `192.168.1.50`).
2.  Edit your `firmware/include/secrets.h` (or similar configuration):
    ```cpp
    #define MQTT_BROKER "192.168.1.50" // Your host IP
    #define MQTT_PORT 1883
    #define MQTT_USER ""
    #define MQTT_PASS ""
    ```
3.  Flash the firmware to your device.

## Verifying Results

When the device connects and publishes, you should see output in the `validator.py` terminal.

To simulate a message without `mosquitto_pub` installed on your host:
```bash
sudo docker compose exec mosquitto mosquitto_pub -h localhost -t "side-eye/test-device/stats" -m '{"version": "1.0.0", "uptime": 100}'
```

You should see:
```text
[Received] Topic: side-eye/test-device/stats
{
  "version": "1.0.0",
  "uptime": 100
}
PASSED: Stats payload is well-formed.
```

## Simulating Downtime

To test reconnection logic:
1. Stop the broker: `sudo docker compose stop`
2. Observe the device logs (via `task firmware:monitor`).
3. Restart the broker: `sudo docker compose start`
4. The device should automatically reconnect and resume publishing.
