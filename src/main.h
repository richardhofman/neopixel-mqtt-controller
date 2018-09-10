#include <Arduino.h>

void setup();
void loop();
void mqttReconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void startupWipe(uint8_t wait, uint32_t colour);
void updateLightState(bool on, uint8_t patternId, uint8_t brightness, uint8_t* rgbValues, bool hasColours);
uint8_t pattern_id_from_name(const char* effect);