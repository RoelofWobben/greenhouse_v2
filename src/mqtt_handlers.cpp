#include "mqtt_conn.h"
#include "panel.h"

MoistureLevel currentMoistureLevel = MOISTURE_DRY;
String lastMoistureValue = "-";

// Wordt aangeroepen zodra er een MQTT-bericht binnenkomt.
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.print("M5 ontving op ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (String(topic) == lightPanel.mqttStatusTopic)
  {
    LightState confirmed = (message == "ON") ? STATE_ON : STATE_OFF;
    panels.confirmState(lightPanel, confirmed);
    panels.drawPanels();
  }

  if (String(topic) == moistureStatusPanel.mqttStatusTopic)
  {
    // De sensor publiceert een percentage (0-100), niet de ruwe ADC-waarde.
    float value = message.toFloat();

    if (value < 30.0f) {
      currentMoistureLevel = MOISTURE_DRY;
    } else if (value < 70.0f) {
      currentMoistureLevel = MOISTURE_GOOD;
    } else {
      currentMoistureLevel = MOISTURE_WET;
    }

    lastMoistureValue = message; 

    if (currentScreen == SCREEN_STATUS) {
      panels.drawStatusScreen();
    } 


  }
}