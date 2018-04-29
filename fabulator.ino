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
bool mono = false;

int pins[][6] = {
  {X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, X_MS1_PIN, X_MS2_PIN, X_MS3_PIN},
  {Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, -1, -1, -1},
  {Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, -1, -1, -1},
  {E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN, -1, -1, -1}
};

Stepper* steppers[NUM_STEPPERS];

void setup() {
  Serial.begin(115200);
  
  unsigned long startMillis = millis();
  int i;
  for (i = 0; i < NUM_STEPPERS; i++) {
    steppers[i] = new Stepper(pins[i][0], pins[i][1], pins[i][2], pins[i][3], pins[i][4], pins[i][5]);
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

float detuneCalc(float cents) {
  return pow(2, cents/1200.0);
}

void handleSerial() {
  if (Serial.available() >= 5) {
    char nlo = Serial.read();
    char lo = Serial.read();
    char hi = Serial.read();
    char lo2 = Serial.read();
    char hi2 = Serial.read();
    int note = word(hi, lo);
    int speed = word(hi2, lo2);
    int n = word(0, nlo);
//    Serial.print("Got: ");
//    Serial.println(note);

    bool isNote = n > 0 && n < NUM_STEPPERS + 1;
    int i;
    if (isNote) {
      if (mono) {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setNote(note, convertVolume(speed));
        }
      } else {
        steppers[n-1]->setNote(note, convertVolume(speed));
      }
    }
    
    if (n == 10) {
      float period = ((50000 * lo) / 127) / 100;
//      Serial.println(period);
      for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setPeriod(period);
      }
    }

    if (n == 11) {  // Detune up to 1 semitone
      float f = detuneCalc(100.0*((lo / 127.0) * 2.0 - 1.0));
      if (mono) {
        for (i = 1; i < NUM_STEPPERS; i++) {
          steppers[i]->setDetune(((float)i / (NUM_STEPPERS-1))*f);
        }
      } else {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setDetune(f);
        }
      }

//      Serial.println(f);
    }

    if (n == 12) {  // Detune up to 1 octave
      float f = detuneCalc(1200.0*((lo / 127.0) * 2.0 - 1.0));
      if (mono) {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setPitchShift(f);
        }
      } else {
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setPitchShift(f);
        }
      }

//      Serial.println(f);
    }
  }
}

int convertVolume(int volume) {
  if (volume > 90) return 5;
  if (volume > 65) return 4;
  if (volume > 50) return 3;
  if (volume > 30) return 2;

  return 1;
}

