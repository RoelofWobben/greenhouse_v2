#include <M5Unified.h>

#include "wifi_conn.h"
#include "mqtt_conn.h"

#include "panel.h"
#include "scroll.h"

void setup() {
  Serial.begin(115200);

  auto cfg = M5.config();
  M5.begin(cfg);

  panels.begin();

  connectWifi();
  connectMqtt();

  panels.drawPanels();
}

void loop() {
  M5.update();

  ensureMqttConnected();
  MqttClient.loop(); 

  panels.handleTabtouch();

  if (currentScreen == SCREEN_BEDIENING) {
    scroller.handleScroll(panels, [&]() {panels.drawPanels();});

    panels.handlePanelTouch(lightPanel);
    panels.handlePanelTouch(pompPanel);
    panels.handlePanelTouch(windowPanel);
  }
  panels.checkAllTimeouts();
}