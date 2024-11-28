# RNBO for the Arduino GIGA R1 Platform

## Author

Stefan Brunner - stb@cycling74.com

## Description

This should hopefully get you started on using RNBO on the Arduino GIGA R1 (https://docs.arduino.cc/tutorials/giga-r1-wifi/giga-getting-started/).

As so often: this is under heavy development, so use this at your own risk, it might suddenly crash, burn, glitch, annoy your cat - anything can happen

Make sure you have the latest RNBO beta installed. Otherwise this might not be working.

## Prerequisites

First make sure you have a working Arduino toolchain by following these steps: https://docs.arduino.cc/tutorials/giga-r1-wifi/giga-getting-started/

Install the Arduino_AdvancedAnalog library (https://github.com/arduino-libraries/Arduino_AdvancedAnalog) via the
IDE library management system.

Have a look at https://docs.arduino.cc/tutorials/giga-r1-wifi/giga-audio/ and try to upload some of 
the examples

## Export

RNBO C++ Source Code Export generated files need to go into the _rnboino/export_ directory. Be aware that you need to use the "Minimal Export" feature introduced on the bare_metal branch to generate code that works with this example.

Consider using a fixed vector size that matches the one set in the example (currently 64 samples, but feel free to 
change it to your needs) for your export.

## Usage

Open the _rnboino.ino_ project and upload to the GIGA.

## Limitations

Currently there is no MIDI and no Audio In (only Audio Out). 

