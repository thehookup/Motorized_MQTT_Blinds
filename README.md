# Motorized_MQTT_Blinds


This repository is to accompany my Motorized_MQTT_Blinds video:

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/1O_1gUFumQM/0.jpg)](https://www.youtube.com/watch?v=1O_1gUFumQM)

## Parts List
Stepper Motors: https://amzn.to/2D5rVsF
Stepper Drivers: https://amzn.to/2OZqW1W
NodeMCU: https://amzn.to/2I89xDF
12V Power Supply: https://amzn.to/2G2ZJrf
Buck Converter: https://amzn.to/2UsQ7jA

## 3D Printing

Download the correct STL file for your style of tilt rod

## Wiring Schematic

![alt text](https://github.com/thehookup/Motorized_MQTT_Blinds/blob/master/Schematic.jpg?raw=true)

## File setup

Fill out the entire USER CONFIGURATION section of the code.

You should leave "STEPS_TO_CLOSE" at 12 to start with.  It can be adjusted for your specific blinds

## Home Assistant YAML

Replace "BlindsMCU" with your MQTT_CLIENT_ID if you changed it in the file setup

```yaml
cover:
  - platform: mqtt
    name: "Motorized Blinds"
    command_topic: "BlindsMCU/blindsCommand"
    set_position_topic: "BlindsMCU/positionCommand"
    position_topic: "BlindsMCU/positionState"
    state_topic: "BlindsMCU/positionState"
    retain: true
    payload_open: "OPEN"
    payload_close: "CLOSE"
    payload_stop: "STOP"
    position_open: 0
    position_closed: 12
  ```
## Alexa only support (no hub required) coming soon
