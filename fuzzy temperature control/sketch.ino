#include <Arduino.h>
#include "DHT.h"

// ============ PIN & HARDWARE DEFINITIONS ============

// DHT11 sensor data pin
#define DHT_PIN 4

// DHT type
#define dht_type DHT22

// Peltier Control (PWM) 
// We use an N-channel MOSFET with its Gate on this pin.
#define PELTIER_OUT_PIN  5     
#define PELTIER_PWM_CHANNEL  0 
#define PELTIER_PWM_FREQ     5000 // 5 kHz
#define PELTIER_PWM_RES      8    // 8-bit (0-255)

// Fan pin (digital on/off), also controlling via an N-channel MOSFET if needed
#define FAN_PIN 18

// Mist maker pin (digital on/off) 
#define MIST_PIN 19

// ============ TEMPERATURE SETPOINTS & TIMING ============
#define TEMP_FULL_ON        20.0 
#define TEMP_FULL_OFF       15.0 
#define TEMP_MIST_THRESHOLD 23.0

#define MIST_CHECK_DELAY_MS (10UL * 1000UL) // 10 SECONDS after start
#define MIST_ON_TIME_MS     (2UL * 1000UL)        // 2 seconds ON
#define MIST_OFF_TIME_MS    (5UL * 1000UL)       // 5 seconds OFF

// ============ GLOBAL STATE VARIABLES ============


bool hasCheckedMist = false;
bool mistOn = false;
uint32_t mistToggleStart = 0;
uint32_t systemStartTime = 0;

// ============ FUZZY-LIKE DUTY CALCULATION ============
// Return a duty cycle [0..1] based on temperature.
double getFuzzyDuty(double temp) {
  if (temp <= TEMP_FULL_OFF) {
    // Below 15 => off
    return 0.0;
  }
  if (temp >= TEMP_FULL_ON) {
    // Above (or at) 20 => full power
    return 1.0;
  }
  // Between 15 and 20 => linear ramp from 0..1
  return (temp - TEMP_FULL_OFF) / (TEMP_FULL_ON - TEMP_FULL_OFF);
}

//DHT initialize
DHT dht(DHT_PIN, dht_type);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ============ SETUP PWM FOR PELTIER ============
  // Using ESP32 LEDC driver
  pinMode(PELTIER_OUT_PIN, OUTPUT);
  // ============ SETUP I/O FOR FAN & MIST ============
  pinMode(FAN_PIN, OUTPUT);
  pinMode(MIST_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(MIST_PIN, LOW);

  // DHT begining
  dht.begin();

  // Record the system start time
  systemStartTime = millis();

  Serial.println("[INFO] System initialized. Starting control loop...");
}

void loop() {
  // 1) Read temperature from DHT11
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("[WARN] DHT11 read failed; skipping this cycle.");
    delay(2000);
    return;
  }

  // 2) Calculate duty cycle for Peltier via fuzzy logic
  double duty = getFuzzyDuty(temperature);

  // 3) Apply duty to Peltier (0..1 => 0..255 for 8-bit)
  uint32_t pwmValue = (uint32_t)(duty * 255.0);
  analogWrite(PELTIER_OUT_PIN, pwmValue);


  // 4) Fan on if duty > 0
  digitalWrite(FAN_PIN, (duty > 0.0) ? HIGH : LOW);

  // 5) Timed Mist Logic
  uint32_t now = millis();

  // (a) Wait 5 minutes after system start before first mist check
  if (!hasCheckedMist && (now - systemStartTime >= MIST_CHECK_DELAY_MS)) {
    hasCheckedMist = true;
    // If still above 23 => start mist cycle
    if (temperature > TEMP_MIST_THRESHOLD) {
      mistOn = true;
      mistToggleStart = now;
      digitalWrite(MIST_PIN, HIGH);
      Serial.println("[INFO] Mist turned ON (first cycle).");
    }
  }

  // (b) If we've begun mist cycles, keep going if temperature > 23
  if (hasCheckedMist && (temperature > TEMP_MIST_THRESHOLD)) {
    if (mistOn) {
      // Are we past the ON duration?
      if ((now - mistToggleStart) >= MIST_ON_TIME_MS) {
        // Turn mist off
        mistOn = false;
        mistToggleStart = now;
        digitalWrite(MIST_PIN, LOW);
        Serial.println("[INFO] Mist turned OFF, starting OFF period...");
      }
    } else {
      // We are in OFF period
      if ((now - mistToggleStart) >= MIST_OFF_TIME_MS) {
        // Turn mist on again
        mistOn = true;
        mistToggleStart = now;
        digitalWrite(MIST_PIN, HIGH);
        Serial.println("[INFO] Mist turned ON, starting ON period...");
      }
    }
  } else {
    // If temperature <= 23, ensure mist is off
    if (mistOn) {
      mistOn = false;
      digitalWrite(MIST_PIN, LOW);
      Serial.println("[INFO] Mist turned OFF (temp below threshold).");
    }
  }

  // 6) Debug prints
  Serial.print("[INFO] Temp: ");
  Serial.print(temperature);
  Serial.print("°C, Duty: ");
  Serial.print(duty * 100.0);
  Serial.println("%");

  // 7) Wait before next cycle
  delay(1000);
}