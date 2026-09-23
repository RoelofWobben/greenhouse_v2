#include "mqtt_conn.h"
#include "panel.h"

const char* MQTT_SERVER = "mosquitto.local";
const int MQTT_PORT = 8883;
const char* MQTT_CLIENT_ID = "greenhouse-m5";

WiFiClientSecure espClientM5;
PubSubClient MqttClient(espClientM5);

namespace {
  constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000;
  bool lastMqttConnectionState = false;
  unsigned long lastMqttReconnectAttempt = 0;
}

bool connectMqtt() {
  espClientM5.setCACert(ca_cert); 
  MqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  MqttClient.setCallback(mqttCallback);

  Serial.println("Verbinden met MQTT .....");

  if (MqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
    Serial.println("MQTT verbonden");
    MqttClient.subscribe("greenhouse/light/status");
    bool subscribed = MqttClient.subscribe("greenhouse/sensor/moisture");

    Serial.print("Moisture subscription: ");
    Serial.println(subscribed ? "gelukt" : "mislukt");
    return true;
  } else {
    Serial.print("MQTT verbinden mislukt, state: ");
    Serial.println(MqttClient.state());
    return false;
  }
}

void ensureMqttConnected() {
  MqttClient.loop();

  bool mqttConnected = MqttClient.connected();

  if (!mqttConnected && millis() - lastMqttReconnectAttempt >= MQTT_RECONNECT_INTERVAL_MS) {
    lastMqttReconnectAttempt = millis();
    connectMqtt();
    mqttConnected = MqttClient.connected();
  }

  if (mqttConnected != lastMqttConnectionState) {
    lastMqttConnectionState = mqttConnected;
    Serial.print("MQTT status gewijzigd: ");
    Serial.println(mqttConnected ? "verbonden" : "verbinding verbroken");

    if (currentScreen == SCREEN_STATUS) {
      panels.drawStatusScreen();
    }
  }
}