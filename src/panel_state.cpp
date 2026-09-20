#include "panel.h"
#include "mqtt_conn.h"

void PanelSystem::requestState(Panel& panel) {
  panel.previousState = panel.state;
  panel.state = STATE_WAIT;
  panel.waitStartMillis = millis();
}

void PanelSystem::confirmState(Panel& panel, LightState confirmedState) {
  panel.state = confirmedState;
}

bool PanelSystem::checkTimeout(Panel& panel) {
  if (panel.state == STATE_WAIT && (millis() - panel.waitStartMillis > TIMEOUT_MS)) {
    panel.state = panel.previousState;
    return true;
  }
  return false;
}

// Checkt voor alle panelen of een timeout is verstreken; herTekent indien nodig.
void PanelSystem::checkAllTimeouts() {
  bool anyTimedOut = false;

  if (panels.checkTimeout(lightPanel)) anyTimedOut = true;
  //if (panels.checkTimeout(pompPanel)) anyTimedOut = true;
  //if (panels.checkTimeout(windowPanel)) anyTimedOut = true;

  if (anyTimedOut) {
    drawPanels();
  }
}

void PanelSystem::handlePanelTouch(Panel& panel) {
  if (panel.state == STATE_WAIT) return;  // al bezig, negeer nieuwe tikken

  RectButton onButton = panels.getOnButton(panel);
  RectButton offButton = panels.getOffButton(panel);

  if (panel.state != STATE_ON && panels.isButtonTouched(onButton)) {
    panels.requestState(panel);
    panels.drawPanels();

    panels.publishRequest(panel, STATE_ON);
  }

  if (panel.state != STATE_OFF && panels.isButtonTouched(offButton)) {
    panels.requestState(panel);
    panels.drawPanels();

    panels.publishRequest(panel, STATE_OFF);
  }
}

// Publiceert het GEVRAAGDE commando (nog geen bevestigde status)
void PanelSystem::publishRequest(const Panel& panel, LightState requested) {
  if (panel.mqttTopic == nullptr) return;

  const char* payload = (requested == STATE_ON) ? "ON" : "OFF";
  MqttClient.publish(panel.mqttTopic, payload);

  Serial.print(panel.label);
  Serial.print(" verzoek -> ");
  Serial.println(payload);
}