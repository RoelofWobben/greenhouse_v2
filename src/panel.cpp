#include "panel.h"
#include "scroll.h"

#include "pump_off.h"
#include "pump_on.h"
#include "window_closed.h"
#include "window_open.h"
#include "light_off.h"
#include "light_on.h"
#include "mqtt_conn.h"

int tabBarY = 200;
int tabBarHeight = 40;

Screen currentScreen = SCREEN_BEDIENING;  

// drie panelen -- let op: geen "status"-bool meer los, die zit nu IN de Panel
// (state, previousState, waitStartMillis)
Panel lightPanel = { 10, 10, 300, 100, "Light", "On", "Off", "...", "greenhouse/light/set", "greenhouse/light/status" };
Panel pompPanel = { 10, 130, 300, 100, "Pomp", "On", "Off", "...", nullptr, nullptr };
Panel windowPanel = { 10, 250, 300, 100, "Window", "Open", "Closed", "...", nullptr, nullptr };

// Meerdere panelen voor de status 
Panel wifiStatusPanel = {10,10,300,100, "WiFi","verbonden", "Niet verbonden", nullptr, nullptr, nullptr};

PanelSystem panels;
ScrollSystem scroller;





PanelSystem::PanelSystem() : canvas(&M5.Display) {}

void PanelSystem::begin() {
  canvas.setColorDepth(16);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setSwapBytes(true);
}

void PanelSystem::flush() {
  canvas.pushSprite(0, 0);
}

void PanelSystem::setScrollOffset(int newOffset) {
  scrollOffSet = constrain(newOffset, minScrollOffSet, maxScrollOffSet);
}

int PanelSystem::getScrollOffset() const {
  return scrollOffSet;
}

// Wordt aangeroepen zodra er een MQTT-bericht binnenkomt.
void PanelSystem::mqttCallback(char* topic, byte* payload, unsigned int length) {
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


void PanelSystem::drawStatusCard(const Panel& panel, const uint16_t* iconOn, const uint16_t* iconOff) {


  const uint16_t* icon = (panel.state == STATE_OFF) ? iconOff : iconOn;
  canvas.pushImage(panel.x + 20, panel.y + 10 - scrollOffSet, 32, 32, icon, 0xFFFF);

  canvas.setTextColor(WHITE, panelColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_left);
  canvas.drawString(panel.label, panel.x + 62, panel.y + 26 - scrollOffSet);
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


void PanelSystem::drawPanels() {
  getCanvas().fillScreen(BLACK);

  drawPanel(lightPanel, lightIconOn, lightIcon);
  drawButtons(lightPanel);

  drawPanel(pompPanel, pumpIconOn, pumpIconOff);
  drawButtons(pompPanel);

  drawPanel(windowPanel, windowIconOpen, windowIconClosed);
  drawButtons(windowPanel);

  drawTabBar();
  flush();
}


void handlePanelTouch(Panel& panel) {
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

bool PanelSystem::isTabTouched(int tabIndex) {

  if (M5.Touch.getCount() == 0) return false;

  auto detail = M5.Touch.getDetail(0);

  if (!detail.wasPressed()) return false;

  int tabX = tabIndex * 160;

  return (detail.x >= tabX && detail.x <= tabX + 160 && detail.y >= tabBarY && detail.y <= tabBarY + tabBarHeight);
}



void PanelSystem::handleTabtouch() {

  if (isTabTouched(0) && currentScreen != SCREEN_STATUS) {
    currentScreen = SCREEN_STATUS;
    drawCurrentScreen();
  }

  if (isTabTouched(1) && currentScreen != SCREEN_BEDIENING) {
    currentScreen = SCREEN_BEDIENING;
    drawCurrentScreen();
  }
}




void PanelSystem::drawPanel(const Panel& panel, const uint16_t* iconOn, const uint16_t* iconOff) {
  canvas.fillRoundRect(panel.x, panel.y - scrollOffSet, panel.w, panel.h, 12, panelColor);

  const uint16_t* icon = (panel.state == STATE_OFF) ? iconOff : iconOn;
  canvas.pushImage(panel.x + 20, panel.y + 10 - scrollOffSet, 32, 32, icon, 0xFFFF);

  canvas.setTextColor(WHITE, panelColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_left);
  canvas.drawString(panel.label, panel.x + 62, panel.y + 26 - scrollOffSet);
}

void PanelSystem::drawStatusScreen() {

  panels.getCanvas().fillScreen(BLACK);

  panels.drawPanel(wifiStatusPanel, nullptr, nullptr);





  

  //bool mqttOk = (MqttClient.connected());
  //drawStatusCard(130, "MQTT", mqttOk ? "Verbonden" : "Niet verbonden", nullptr, mqttOk ? GREEN : RED);

  //bool lightOn = (lightPanel.state == STATE_ON);
  //const uint16_t* lightIcon2 = lightOn ? lightIconOn : lightIcon;
  //drawStatusCard(250, "Licht", lightOn ? "Aan" : "Uit", lightIcon2, lightOn ? GREEN : 0x39C7);


  drawTabBar();
  panels.flush();
}


void PanelSystem::drawCurrentScreen() {

  if (currentScreen == SCREEN_STATUS) {
    drawStatusScreen();
  } else {
    drawPanels();
  }
}



void PanelSystem::drawTabBar() {

  uint16_t activeColor = 0x03df;
  uint16_t inactiveColor = 0x0000;

  uint16_t statusColor = (currentScreen == SCREEN_STATUS) ? activeColor : inactiveColor;
  uint16_t bedieningsColor = (currentScreen == SCREEN_BEDIENING) ? activeColor : inactiveColor;

  canvas.fillRect(0, tabBarY, 160, tabBarHeight, statusColor);
  canvas.fillRect(160, tabBarY, 160, tabBarHeight, bedieningsColor);

  canvas.setTextColor(WHITE, statusColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_center);
  canvas.drawString("Status", 80, tabBarY + tabBarHeight / 2);

  canvas.setTextColor(WHITE, bedieningsColor);
  canvas.drawString("Bediening", 240, tabBarY + tabBarHeight / 2);
}


void PanelSystem::drawSingleButton(const RectButton& button, uint16_t color) {
  canvas.fillRect(button.x - 2, button.y - 2, button.w - 4, button.h + 4, panelColor);

  canvas.fillRoundRect(button.x, button.y, button.w, button.h, 10, color);

  canvas.setTextColor(WHITE, color);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_center);
  canvas.drawString(button.label, button.x + button.w / 2, button.y + button.h / 2);
}

RectButton PanelSystem::getOnButton(const Panel& panel) {
  return { panel.x, panel.y + 50 - scrollOffSet, 120, 40, panel.textOn };
}

RectButton PanelSystem::getOffButton(const Panel& panel) {
  return { panel.x + 160, panel.y + 50 - scrollOffSet, 120, 40, panel.textOff };
}

void PanelSystem::drawButtons(const Panel& panel) {
  RectButton onButton = getOnButton(panel);
  RectButton offButton = getOffButton(panel);

  if (panel.state == STATE_WAIT) {
    drawSingleButton(onButton, waitColor);
    drawSingleButton(offButton, waitColor);
  } else {
    bool isOn = (panel.state == STATE_ON);
    drawSingleButton(onButton, isOn ? GREEN : grey);
    drawSingleButton(offButton, isOn ? grey : GREEN);
  }
}

bool PanelSystem::isButtonTouched(const RectButton& button) {
  if (M5.Touch.getCount() == 0) return false;

  auto detail = M5.Touch.getDetail(0);
  if (!detail.wasPressed()) return false;

  return (detail.x >= button.x && detail.x <= button.x + button.w &&
          detail.y >= button.y && detail.y <= button.y + button.h);
}

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

M5Canvas& PanelSystem::getCanvas() {
  return canvas;
}