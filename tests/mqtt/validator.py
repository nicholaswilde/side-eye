#!/usr/bin/env python3
import json
import sys
import paho.mqtt.client as mqtt

# Topics to listen for
TOPICS = [
    "side-eye/+/stats",
    "side-eye/+/identity"
]

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        for topic in TOPICS:
            client.subscribe(topic)
            print(f"Subscribed to {topic}")
    else:
        print(f"Failed to connect, return code {rc}")
        sys.exit(1)

def on_message(client, userdata, msg):
    print(f"\n[Received] Topic: {msg.topic}")
    try:
        payload = json.loads(msg.payload.decode())
        print(json.dumps(payload, indent=2))
        
        # Basic validation logic can be added here
        if "stats" in msg.topic:
            required_keys = ["version", "uptime"]
            missing = [k for k in required_keys if k not in payload]
            if missing:
                print(f"FAILED: Missing keys in stats: {missing}")
            else:
                print("PASSED: Stats payload is well-formed.")
        
    except json.JSONDecodeError:
        print(f"FAILED: Could not decode JSON payload: {msg.payload}")
    except Exception as e:
        print(f"ERROR: {e}")

def main():
    broker = "localhost"
    port = 1883

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    print(f"Connecting to broker at {broker}:{port}...")
    try:
        client.connect(broker, port, 60)
    except Exception as e:
        print(f"Could not connect to broker: {e}")
        print("Make sure the Mosquitto container is running (sudo docker compose up -d)")
        sys.exit(1)

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nStopping validator...")
        client.disconnect()

if __name__ == "__main__":
    main()
