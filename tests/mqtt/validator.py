#!/usr/bin/env python3
import json
import sys
import paho.mqtt.client as mqtt

# Topics to listen for
TOPICS = [
    "side-eye/+/stats",
    "side-eye/+/identity"
]

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Connected to MQTT Broker!")
        for topic in TOPICS:
            client.subscribe(topic)
            print(f"Subscribed to {topic}")
    else:
        print(f"Failed to connect, return code {reason_code}")
        sys.exit(1)

import argparse

# ... (TOPICS and other globals remain)

def on_message(client, userdata, msg):
    print(f"\n[Received] Topic: {msg.topic}")
    try:
        payload = json.loads(msg.payload.decode())
        print(json.dumps(payload, indent=2))
        
        success = True
        # Basic validation logic
        if "stats" in msg.topic:
            required_keys = ["version", "uptime"]
            missing = [k for k in required_keys if k not in payload]
            if missing:
                print(f"FAILED: Missing keys in stats: {missing}")
                success = False
            else:
                print("PASSED: Stats payload is well-formed.")
        
        if userdata.get("test_mode"):
            userdata["received"] = True
            userdata["success"] = success
            client.disconnect() # Exit loop
        
    except json.JSONDecodeError:
        print(f"FAILED: Could not decode JSON payload: {msg.payload}")
    except Exception as e:
        print(f"ERROR: {e}")

def main():
    parser = argparse.ArgumentParser(description="MQTT Validator for SideEye")
    parser.add_argument("--test", action="store_true", help="Exit after receiving one valid message")
    parser.add_argument("--timeout", type=int, default=30, help="Timeout in seconds for test mode")
    args = parser.parse_args()

    broker = "localhost"
    port = 1883

    userdata = {"test_mode": args.test, "received": False, "success": False}
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, userdata=userdata)
    client.on_connect = on_connect
    client.on_message = on_message

    print(f"Connecting to broker at {broker}:{port}...")
    try:
        client.connect(broker, port, 60)
    except Exception as e:
        print(f"Could not connect to broker: {e}")
        sys.exit(1)

    if args.test:
        client.loop_start()
        import time
        start_time = time.time()
        while not userdata["received"] and (time.time() - start_time) < args.timeout:
            time.sleep(0.1)
        client.loop_stop()
        
        if userdata["received"] and userdata["success"]:
            print("Test automation: Success")
            sys.exit(0)
        else:
            print("Test automation: Failed or Timed out")
            sys.exit(1)
    else:
        try:
            client.loop_forever()
        except KeyboardInterrupt:
            print("\nStopping validator...")
            client.disconnect()

if __name__ == "__main__":
    main()
