#include "mqtt_conn.h"
#include "panel.h"

// Wordt aangeroepen zodra er een MQTT-bericht binnenkomt.
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("M5 ontving op ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (String(topic) == lightPanel.mqttStatusTopic) {
    LightState confirmed = (message == "ON") ? STATE_ON : STATE_OFF;
    panels.confirmState(lightPanel, confirmed);
    panels.drawPanels();
  }

  if (String(topic) == moistureStatusPanel.mqttStatusTopic) {
    // Hier kun je de status van de vochtigheidssensor verwerken
    // Bijvoorbeeld: als de sensor een waarde terugstuurt, kun je die hier gebruiken
    Serial.print("Vochtigheidssensor status: ");
    Serial.println(message);
  }
}