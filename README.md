- 👋 Hi, I’m Phil
- 👀 I’m interested in  ARDUPILOT on esp32
- 📫 How to reach me ...discord:   philo #8240

Platformio VS Code Project
esp32 & LORA modules RC joystick base station and vehicle devices 
low bandwidth  8 byte packet over lora which converts to IBUS protocol on vehicle for feeding ardupilot RC input
uses  2 joysticks with calibrate and centering functions 
oled screen display shows joystick posns, radio signal strength at base AND vehicle and gps posn of vehicle
self contained except for OLED Adafruit library
uses two esp32, two Lora modules, GPS module and two joysticks/switches
To use: compile either base or vehicle by commenting the   #define for either PLANE or BASE

Holding button down while starting will calibrate joystick ranges (move to extremities then release button)
Pressing button again later will center the joystick to the currently held positions thus enabling trim
Has a mode switch but cannot recall how I  configured it. This is a 6 mth stale project and  now have only a working base module, the vehicle module used elsewhere





<!---
calphil/calphil is a ✨ special ✨ repository because its `README.md` (this file) appears on your GitHub profile.
You can click the Preview link to take a look at your changes.
--->
