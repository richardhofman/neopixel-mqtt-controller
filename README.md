# ESP8266 Home Assistant MQTT NeoPixel Controller

## Purpose

This repo contains the source for a simple ESP8266 MQTT controller for NeoPixel/WS2812 strips. I built it for my own personal use with my Home Assistant instance.

## Configuration

### The ESP8266

Update the values in `src/settings.h` (you will need to rename `settings.h.example` initially) with the details of your MQTT broker and NeoPixel strip.

### Home Assistant

Here is the configuration which I use to control the light in my Assistant instance. Most of it is explained very well on the MQTT JSON light [documentation](https://www.home-assistant.io/components/light.mqtt_json).

```yaml
- platform: mqtt_json
  name: NeoPixel Strip
  command_topic: "lights/neopixels/set"
  brightness: true
  brightness_scale: 255
  rgb: true
  effect: true
  effect_list:
    - Static
    - Fire Flicker
    - Fire Flicker Soft
    - Fire Flicker Intense
    - Rainbow Cycle
    - Blink
    - Breath
    - Colour Wipe
    - Random Colour
    - Running Lights
    - Chase Colour
    - Chase Rainbow
    - Chase White
    - Comet
    - Fireworks
    - ICU
```

## Usage

Configure your home controller (or whatever is controlling your strip) to send JSON payloads of the form (the one used by Home Assistant's [MQTT JSON light component](https://www.home-assistant.io/components/light.mqtt_json)):

    {
      "state": "ON",
      "brightness": 128,
      "effect": "Static",
      "color": {
        "r": 128,
        "g": 128,
        "b": 128
      }
    }

Upon receipt, the controller will update the state of the strip according to those values. Home Assistant doesn't usually supply all of the fields in one payload, so the
controller will only update the values that it receives.

If you follow my configuration guidelines above and use Home Assistant, you should pretty much be able to drop it in and go!

## Development Status

This project now works, and I'm probably only going to maintain it with the off improvement and bug fix for myself. I primarily built this for my own home usage, so now that
it works I'll likely leave it mostly untouched. I welcome PRs and feature requests if anyone can think of any!