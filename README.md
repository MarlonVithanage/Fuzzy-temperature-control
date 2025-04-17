# Fuzzy-temperature-control

🌿 ESP32 Fuzzy Temperature Control System for Strawberry Cultivation
This project is a fuzzy logic-based temperature control system designed to optimize environmental conditions for strawberry plant growth in controlled spaces like greenhouses or urban farms.

▶️ Run in Wokwi

Click here to open and simulate:

👉 https://wokwi.com/projects/428437776049680385

🚀 Project Overview

Platform: ESP32 (simulated using Wokwi)

Sensor: DHT22 (temperature and humidity)

Actuators:

Peltier cooler (simulated with red LED)

Exhaust fans (simulated with blue LEDs)

Mist maker (simulated with light blue LED)

The system uses fuzzy logic to:

Maintain optimal temperature (~20°C) using a Peltier cooler with PWM control.

Trigger misting cycles if the environment doesn't cool fast enough.

Regulate fans based on cooling demand.

🔧 Features
Fuzzy Logic Control: Smoothly varies the Peltier’s PWM duty cycle between 0% and 100% based on temperature.

Mist Trigger Logic: If temperature remains above 23°C for 5 minutes, misting starts in 5s ON / 30s OFF cycles until temperature drops.

Real-Time Monitoring: Serial output logs temperature and PWM duty, and shows mist/fan activity.

🧪 Components Simulated in Wokwi

ESP32 board

DHT22 sensor (connected to GPIO 4)

LEDs (simulate actuators)

Red LED → Peltier

Blue LEDs → Exhaust fans

Light blue LED → Mist maker

220Ω resistors for LEDs

Breadboard connections with GND and 3.3V from ESP32

🖥️ How to Use

📁 Files
fuzzy_control.ino: Main Arduino sketch

diagram.json: Wokwi wiring configuration

README.md: This file

📦 How It Works

1. System starts, waits 5 minutes.

2. If temp > 23°C, mist starts pulsing.

3. Peltier cooler is always on full power above 20°C.

4. Below 20°C, fuzzy logic modulates the power down to zero at 15°C.

5. Fans run whenever Peltier is active.
   
🧠 Logic Snippet (Pseudocode)

if (temp >= 20) duty = 1.0;

else if (temp <= 15) duty = 0.0;

else duty = (temp - 15) / (20 - 15);

🛠️ To-Do / Future Work

Add humidity fuzzy logic control.

Integrate real-time web dashboard.

Extend to greenhouse control with multiple zones.

