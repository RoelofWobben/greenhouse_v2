#include <M5Unified.h>

#include "wifi_conn.h"
#include "mqtt_conn.h"



#include "panel.h"
#include "scroll.h"





void drawStatusCard(const Panel& panel, const uint16_t* iconOn, const uint16_t* iconOff) {


  const uint16_t* icon = (panel.state == STATE_OFF) ? iconOff : iconOn;
  canvas.pushImage(panel.x + 20, panel.y + 10 - scrollOffSet, 32, 32, icon, 0xFFFF);

  canvas.setTextColor(WHITE, panelColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_left);
  canvas.drawString(panel.label, panel.x + 62, panel.y + 26 - scrollOffSet);
}













// Verwerkt een tik op AAN/UIT: alleen toegestaan als het paneel niet al
// aan het wachten is op een eerdere bevestiging.

// Checkt voor alle panelen of een timeout is verstreken; herTekent indien nodig.
void checkAllTimeouts() {
  bool anyTimedOut = false;

  if (panels.checkTimeout(lightPanel)) anyTimedOut = true;
  //if (panels.checkTimeout(pompPanel)) anyTimedOut = true;
  //if (panels.checkTimeout(windowPanel)) anyTimedOut = true;

  if (anyTimedOut) {
    drawPanels();
  }
}

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
    drawPanels();
  }
}

void setup() {
  Serial.begin(115200);

  auto cfg = M5.config();
  M5.begin(cfg);

  panels.begin();

  connectWifi();
  connectMqtt();

  drawPanels();
}

void loop() {
  M5.update();

  ensureMqttConnected();
  MqttClient.loop(); 

  panels.handleTabtouch();

  if (currentScreen == SCREEN_BEDIENING) {
    scroller.handleScroll(panels, panels.drawPanels());

    panels.handlePanelTouch(lightPanel);
    panels.handlePanelTouch(pompPanel);
    panels.handlePanelTouch(windowPanel);
  }
  checkAllTimeouts();
}