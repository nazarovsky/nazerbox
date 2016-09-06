# NazerBox Programmable Delay Impulse Generator (Arduino Based)

This project was made as a part of the test bench for the relay protection system.
The aim of the Box was to generate rectangle impulse (or batch of N impulses) to start the waveform generation by the Arbitrary waveform generator (Siglent SDG2042X).
The Box receives 1PPS impulses on the Input BNC connector. 
When the user presses the button on the front of the Box, after the arrival of the next 1PPS impulse,
the Box calculates the specified amount of time and then generates a rectangle impulse (with specified width).

The Box is based on the Arduino Board (Arduino Uno or similar can be used)

# Serial Settings

The Box is configured via the Serial Interface (USB to Serial)
Serial settings are following:

   * 115200 8N1

# Defaults

By default the Box has the following parameters:

   * DELAY = 0 ms
   * WIDTH = 100 ms
   * NUM = 1 (1 impulse)

# SCPI Commands

   The supported SCPI commands are:

   *  *IDN?         -> identify
   *  ARM           -> ARM device to fire on next 1PPS
   *  DELAY (?)     -> GET / SET delay (from 0 to 1000 ms)
   *  WIDTH (?)     -> GET / SET impulse width (from 1 to 1000 ms)
   *  NUM (?)       -> GET / SET impulse count (from 1 to 100)

