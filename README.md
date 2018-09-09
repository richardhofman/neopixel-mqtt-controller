# ESP8266 MQTT NeoPixel Controller

## Purpose
This repo contains the source for a simple ESP8266 MQTT controller for NeoPixel/WS2812 strips. I built it for my own personal use with my Home Assistant instance.

## Configuration
Update the values in `src/settings.h` (you will need to rename `settings.h.example` initially) with the details of your MQTT broker and NeoPixel strip.

## Usage
Configure your home controller (or whatever is controlling your strip) to send JSON payloads of the form:

    {
      "on": 1,
      "brightness": 128,
      "pat_id": 10,
      "r": 128,
      "g": 128,
      "b": 128
    }

Upon receipt, the controller will update the state of the strip according to those values. Note that r, g and b are only relevant when in
"single colour mode" (`pat_id == 0`) or when using a monochromatic pattern.

## Development State
Currently this project **does not work**. At all! It will connect to wifi/MQTT and successfully decode JSON payloads as described above,
but it is not capable of actually controlling an attached strip outside of playing back hardcoded test animations.

I am actively working on this, and will update the README with more information when I've actually got it working.
