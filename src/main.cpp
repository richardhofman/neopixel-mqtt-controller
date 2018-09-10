#include <Arduino.h>
#include <WS2812FX.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <settings.h>
#include <main.h>

#include <map>
#include <utility>


#ifdef __AVR__
  #include <avr/power.h>
#endif

// Globals required for NeoPixel strip control.
WS2812FX strip = WS2812FX(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
bool strip_off = true;

// Globals required for MQTT connectivity.
WiFiClient mqttClient;
PubSubClient mqtt(mqttClient);

// Globals required for JSON parsing.
StaticJsonBuffer<210> jsonBuffer;
char* mqttPayloadBuffer[201]; // can store 200-char null-terminated JSON string.

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  #endif

  /* Wifi stuff */
  WiFiManager wifiManager;
  wifiManager.autoConnect("NeoPixel-Controller", "neopixelconfig");

  /* MQTT stuff */
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  /* WS2812B stuff */
  strip.init();
  strip.setBrightness(0);
  strip.setSpeed(50);
}

void loop() {
  strip.service();

  if (!mqtt.loop()) {
    #ifdef SERIAL_DEBUG
    Serial.println("[INFO] (Re)connecting to MQTT server.");
    #endif
    mqttReconnect();
  }
}

/*
  Handler function for incoming MQTT messages. This function will receive the topic and content of any MQTT messages
  passed from the broker.
  
  In the case of lighting control, it will expect a JSON payload with fields: "on", "brightness", "pat_id", "r", "g"
  and "b".
  
  Upon receiving a valid payload, it will update the LEDs' states as per the specified field values.
*/
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Fail gracefully if received message is too large for JSON buffer.
  if (length > 200) {
    #ifdef SERIAL_DEBUG
    Serial.print("[ERROR] Rejecting MQTT payload of size: ");
    Serial.print(length);
    Serial.println(" bytes");
    #endif

    return;
  }

  // Copy MQTT payload from weird non-null-terminated buffer into a dedicated buffer.
  // (And add null term).
  memcpy_P(mqttPayloadBuffer, (char*)payload, length);
  mqttPayloadBuffer[length] = '\0'; // null-terminate payload

  #ifdef SERIAL_DEBUG
  Serial.print("[INFO] Received payload on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println((char*)mqttPayloadBuffer);
  #endif

  // Parse the JSON received in the MQTT payload.
  JsonObject& root = jsonBuffer.parseObject((char*)mqttPayloadBuffer);

  // Fail gracefully if invalid JSON received.
  if (!root.success()) {
    #ifdef SERIAL_DEBUG
    Serial.println("ERROR: Failed to parse JSON object.");
    #endif
    return;
  } else {
    #ifdef SERIAL_DEBUG
    Serial.println("[INFO] Successfully parsed payload.");
    #endif
  }

  // Extract state information from the parsed JSON.
  uint8_t rgbValues[3];
  bool hasColours = false;

  bool powerOn          = root["state"] == "ON" ? true : false;
  uint8_t patternId     = pattern_id_from_name((const char*)root["effect"]);
  uint8_t brightness    = root["brightness"];
  if (root.containsKey("color")) {
    rgbValues[0]          = root["color"]["r"];
    rgbValues[1]          = root["color"]["g"];
    rgbValues[2]          = root["color"]["b"];
    hasColours = true;
  }

  // Reset the buffer for the next message.
  jsonBuffer.clear();

  #ifdef SERIAL_DEBUG
  Serial.println("JSON parse results:");
  Serial.print("On: "); Serial.println(powerOn);
  if (patternId > 0) Serial.print("Pattern ID: "); Serial.println(patternId);
  if (brightness > 0) Serial.print("Brightness: "); Serial.println(brightness);
  if (hasColours) {
    Serial.print("R: "); Serial.println(rgbValues[0]);
    Serial.print("G: "); Serial.println(rgbValues[1]);
    Serial.print("B: "); Serial.println(rgbValues[2]);
  }
  #endif

  // Apply the received state description to the NeoPixel strip.
  updateLightState(powerOn, patternId, brightness, rgbValues, hasColours);
}

uint8_t pattern_id_from_name(const char* effect) {
  #ifdef SERIAL_DEBUG
  Serial.print("Received pattern message: ");
  Serial.println(effect);
  #endif
  if (effect == NULL) {
    #ifdef SERIAL_DEBUG
    Serial.println("No pattern in message, returning 0.");
    #endif
    return 0;
  } else {
    // Here comes strcmp... :(
    if (strcmp_P(effect, "Static") == 0) {
      return FX_MODE_STATIC;
    } else if (strcmp_P(effect, "Blink") == 0) {
      return FX_MODE_BLINK;
    } else if (strcmp_P(effect, "Breath") == 0) {
      return FX_MODE_BREATH;
    } else if (strcmp_P(effect, "Colour Wipe") == 0) {
      return FX_MODE_COLOR_WIPE;
    } else if (strcmp_P(effect, "Random Colour") == 0) {
      return FX_MODE_RANDOM_COLOR;
    } else if (strcmp_P(effect, "Rainbow Cycle") == 0) {
      return FX_MODE_RAINBOW_CYCLE;
    } else if (strcmp_P(effect, "Running Lights") == 0) {
      return FX_MODE_RUNNING_LIGHTS;
    } else if (strcmp_P(effect, "Chase Colour") == 0) {
      return FX_MODE_CHASE_COLOR;
    } else if (strcmp_P(effect, "Chase Rainbow") == 0) {
      return FX_MODE_CHASE_RAINBOW;
    } else if (strcmp_P(effect, "Chase White") == 0) {  
      return FX_MODE_CHASE_WHITE;
    } else if (strcmp_P(effect, "Comet") == 0) {
      return FX_MODE_COMET;
    } else if (strcmp_P(effect, "Fireworks") == 0) {
      return FX_MODE_FIREWORKS;
    } else if (strcmp_P(effect, "Fire Flicker") == 0) {
      return FX_MODE_FIRE_FLICKER;
    } else if (strcmp_P(effect, "Fire Flicker Soft") == 0) {
      return FX_MODE_FIRE_FLICKER_SOFT;
    } else if (strcmp_P(effect, "Fire Flicker Intense") == 0) {
      return FX_MODE_FIRE_FLICKER_INTENSE;
    } else if (strcmp_P(effect, "ICU") == 0) {
      return FX_MODE_ICU;
    }
    return 0;
  }
}

/*
  Handler function for MQTT disconnects. If the network connection is lost, or the MQTT server becomes
  otherwise unavailable temporarily, this function will attempt to re-establish a connection.
*/
void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    // Create a random client ID
    String clientId = "Ziplights-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      mqtt.subscribe(MQTT_SUB_TOPIC);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
  Updates the physical state of the attached array. Handles trandition animations and steady-state definition.
*/
void updateLightState(bool on, uint8_t patternId, uint8_t brightness, uint8_t* rgbValues, bool updateColours) {
  if (on) {
    if (strip_off) {
      if (updateColours) {
        startupWipe(10, strip.Color(rgbValues[0], rgbValues[1], rgbValues[2]));
      } else {
        startupWipe(10, strip.Color(255, 255, 255));
      }
      strip.start();
      strip_off = false;
    }
    if (updateColours) strip.setColor(rgbValues[0], rgbValues[1], rgbValues[2]);
    if (brightness > 0) strip.setBrightness(brightness);
    strip.setMode(patternId);
  } else {
    strip.stop();
    strip_off = true;
  }
}

/*
  Custom animation for light strip power on.
  (Pulse of "hot" colours, shooting down the strip, settling into a light blue.)

  TODO: adjust to work with WS2812FX library.
  TODO: adjust to support settling on the starting colour.
*/
void startupWipe(uint8_t wait, uint32_t colour) {
  uint32_t colours[] = {
    strip.Color(255, 60, 21),
    strip.Color(255, 94, 0),
    strip.Color(255, 163, 0),
    strip.Color(255, 223, 0),
    strip.Color(102, 255, 0),
    strip.Color(0, 255, 142),
    strip.Color(0, 251, 255),
    colour
  };
  
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    if (i > 8) {
      for (uint16_t j = 0; j < 8; j++) {
        strip.setPixelColor(i-j, colours[j]);
      }
      strip.show();
      delay(wait);
    }
  }
}
