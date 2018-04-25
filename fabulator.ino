#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2
#define X_MS1_PIN           4 // TBD
#define X_MS2_PIN           5  // TBD
#define X_MS3_PIN           6 // TBD

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56
#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_MIN_PIN          18
#define Z_MAX_PIN          19

#define E_STEP_PIN         26
#define E_DIR_PIN          28
#define E_ENABLE_PIN       24

#define Q_STEP_PIN         36
#define Q_DIR_PIN          34
#define Q_ENABLE_PIN       30

#define SDPOWER            -1
#define SDSS               53
#define LED_PIN            13

#define FAN_PIN            9

#define PS_ON_PIN          12
#define KILL_PIN           -1

#define HEATER_0_PIN       10
#define HEATER_1_PIN       8
#define TEMP_0_PIN          13   // ANALOG NUMBERING
#define TEMP_1_PIN          14   // ANALOG NUMBERING

#define NUM_STEPPERS      4

#include "Stepper.h"

unsigned long currentMillis = 0;
bool mono = true;

Stepper stepper1(X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN);
Stepper stepper2(Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN);
Stepper stepper3(Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN);
Stepper stepper4(E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN);

Stepper* steppers[NUM_STEPPERS];

void setup() {
  Serial.begin(115200);

  steppers[0] = &stepper1;
  steppers[1] = &stepper2;
  steppers[2] = &stepper3;
  steppers[3] = &stepper4;

//  pinMode(X_MS1_PIN, OUTPUT);
//  pinMode(X_MS2_PIN, OUTPUT);
//  pinMode(X_MS3_PIN, OUTPUT);  

  unsigned long startMillis = millis();
  int i;
  for (i = 0; i < NUM_STEPPERS; i++) {
    steppers[i]->setup(startMillis);
  }
  
  Serial.println("Ready");
}

void loop () {
  currentMillis = millis();
  handleSerial();

  int i;
  for (i = 0; i < NUM_STEPPERS; i++) {
    steppers[i]->run(currentMillis);
  }
}

void handleSerial() {
  if (Serial.available() >= 3) {
    char nlo = Serial.read();
    char lo = Serial.read();
    char hi = Serial.read();
    int note = word(hi, lo);
    int n = word(0, nlo);
    Serial.print("Got: ");
    Serial.println(note);

    bool isNote = n > 0 && n < 5;
    int i;
    if (isNote) {
      if (mono) {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setNote(note);
        }
      } else {
        steppers[n-1]->setNote(note);
      }
    }
    
    if (n == 10) {
      float period = ((50000 * lo) / 127) / 100;
      Serial.println(period);
      for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setPeriod(period);
      }
    }

    if (n == 11) {
      float f = 0.059463 * ((lo / 127.0) * 2 - 1);
      if (mono) {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setDetune(1 + ((float)i / (NUM_STEPPERS-1))*f);
        }
      } else {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setDetune(1 + f);
        }
      }

      Serial.println(f);
    }
  }
}

void changeVolume(int val) {
  switch (val) {
        case 1: // 1
          digitalWrite(X_MS1_PIN, LOW);
          digitalWrite(X_MS2_PIN, LOW);
          digitalWrite(X_MS3_PIN, LOW);
          break;
        case 2: // 1/2
          digitalWrite(X_MS1_PIN, HIGH);
          digitalWrite(X_MS2_PIN, LOW);
          digitalWrite(X_MS3_PIN, LOW);
          break;
        case 3: // 1/4
          digitalWrite(X_MS1_PIN, LOW);
          digitalWrite(X_MS2_PIN, HIGH);
          digitalWrite(X_MS3_PIN, LOW);
          break;
        case 4: // 1/8
          digitalWrite(X_MS1_PIN, HIGH);
          digitalWrite(X_MS2_PIN, HIGH);
          digitalWrite(X_MS3_PIN, LOW);
          break;
        case 5: // 1/16
          digitalWrite(X_MS1_PIN, HIGH);
          digitalWrite(X_MS2_PIN, HIGH);
          digitalWrite(X_MS3_PIN, HIGH);
          break;
  }
}

