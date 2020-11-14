#!/bin/bash

PORT=$(ls /dev/ttyACM*)
FQBN=arduino:avr:uno
arduino-cli compile -b arduino:avr:uno arduino/BasementLights
arduino-cli upload -p $PORT -b arduino:avr:uno arduino/BasementLights