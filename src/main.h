#include <Arduino.h>

void setup();
void loop();
void mqttReconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void shutdownFade(uint8_t wait);
void startupWipe(uint8_t wait);
void updateLightState(uint8_t command, uint8_t patternId, uint8_t brightness, uint8_t* rgbValues);