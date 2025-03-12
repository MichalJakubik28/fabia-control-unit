# fabia-control-unit
This repository contains the firmware for a Teensy microcontroller installed into a 2000 Skoda Fabia, enabling it to do the following:
- Communication with multifunction steering wheels through the LIN bus, thus supporting newer multifunction steering wheels to be installed (from Octavia, Superb MK3,4, Golf MK7,8, Passat B8 etc.)
- Maxidot (full color display instead of only warning lights) showing:
  - current speed
  - selected gear
  - amount of fuel left in the tank
  - battery voltage
  - speed set on the cruise control
  - warning lights that were replaced by the maxidot display

Most of this functionality is specific to this car and cannot be replicated on other cars, mostly because of the CAN IDs that the individual control units use.
