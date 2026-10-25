#include <Arduino.h>
#include <ESP32Servo.h>

// Pin mapping
const int IR_SENSOR_PIN = 19;
const int SEAL_BUTTON_PIN = 25;
const int SERVO_PIN = 13;
const int HEATER_RELAY_PIN = 14;

// Timing constants
const unsigned long LID_CLOSE_DELAY_MS = 4000;
const unsigned long LID_SETTLE_MS = 1000;
const unsigned long HEATING_TIME_MS = 3500;
const unsigned long COOLING_TIME_MS = 2000;
const unsigned long BUTTON_DEBOUNCE_MS = 50;

enum class SealState {
  IDLE,
  CLOSING_LID,
  HEATING,
  COOLING
};

Servo lidServo;
SealState sealState = SealState::IDLE;

unsigned long lastHandDetectedTime = 0;
unsigned long sealStateStartedAt = 0;
unsigned long lastButtonChangedAt = 0;

bool isLidOpen = false;
bool lastSealButtonReading = HIGH;
bool stableSealButtonState = HIGH;

void openLid() {
  lidServo.write(90);
  isLidOpen = true;
}

void closeLid() {
  lidServo.write(0);
  isLidOpen = false;
}

void startSealing(unsigned long now) {
  // Safety interlock: the lid must be closed before the heater can turn on.
  digitalWrite(HEATER_RELAY_PIN, LOW);
  closeLid();
  sealState = SealState::CLOSING_LID;
  sealStateStartedAt = now;
  Serial.println("Bat dau niem phong: dang dong nap...");
}

void updateSealButton(unsigned long now) {
  bool reading = digitalRead(SEAL_BUTTON_PIN);

  if (reading != lastSealButtonReading) {
    lastButtonChangedAt = now;
    lastSealButtonReading = reading;
  }

  if ((now - lastButtonChangedAt >= BUTTON_DEBOUNCE_MS) &&
      (reading != stableSealButtonState)) {
    stableSealButtonState = reading;
    if (stableSealButtonState == LOW && sealState == SealState::IDLE) {
      startSealing(now);
    }
  }
}

void updateSealing(unsigned long now) {
  switch (sealState) {
    case SealState::IDLE:
      return;

    case SealState::CLOSING_LID:
      if (now - sealStateStartedAt >= LID_SETTLE_MS) {
        digitalWrite(HEATER_RELAY_PIN, HIGH);
        sealState = SealState::HEATING;
        sealStateStartedAt = now;
        Serial.println("Nap da dong -> Bat mayso trong 3.5 giay");
      }
      break;

    case SealState::HEATING:
      if (now - sealStateStartedAt >= HEATING_TIME_MS) {
        digitalWrite(HEATER_RELAY_PIN, LOW);
        sealState = SealState::COOLING;
        sealStateStartedAt = now;
        Serial.println("Da tat mayso -> Cho nguoi trong 2 giay");
      }
      break;

    case SealState::COOLING:
      if (now - sealStateStartedAt >= COOLING_TIME_MS) {
        // Open only after the heater is off and the seal has cooled.
        digitalWrite(HEATER_RELAY_PIN, LOW);
        openLid();
        lastHandDetectedTime = now;
        sealState = SealState::IDLE;
        Serial.println("Niem phong hoan tat -> Mo nap de lay tui rac");
      }
      break;
  }
}

void updateAutomaticLid(unsigned long now) {
  // Ignore the hand sensor while sealing is in progress.
  if (sealState != SealState::IDLE) {
    return;
  }

  int sensorState = digitalRead(IR_SENSOR_PIN);
  if (sensorState == LOW) {
    if (!isLidOpen) {
      openLid();
      Serial.println("Phat hien tay -> Mo nap");
    }
    lastHandDetectedTime = now;
  } else if (isLidOpen && now - lastHandDetectedTime >= LID_CLOSE_DELAY_MS) {
    closeLid();
    Serial.println("Da qua 4s khong co tay -> Dong nap");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(SEAL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HEATER_RELAY_PIN, OUTPUT);

  // Heater must always be off during startup.
  digitalWrite(HEATER_RELAY_PIN, LOW);

  lidServo.attach(SERVO_PIN);
  closeLid();

  Serial.println("He thong da khoi dong!");
  Serial.println("Nhan nut GPIO 25 de bat dau niem phong tui rac.");
}

void loop() {
  unsigned long now = millis();
  updateSealButton(now);
  updateSealing(now);
  updateAutomaticLid(now);
  delay(10);
}
