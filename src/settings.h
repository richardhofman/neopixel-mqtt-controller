#define SERIAL_DEBUG    // Comment out to disable Serial output.
#define PIN 0           // D3 on WEMOS D1 mini.
#define NUM_PIXELS 60   // Number of pixels on the LED strip.

#define MQTT_SERVER "mqtt.example.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "user"
#define MQTT_PASSWORD "password"
#define MQTT_SUB_TOPIC "lights/neopixels/set"