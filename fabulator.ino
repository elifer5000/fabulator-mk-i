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

#include "AccelStepper.h"

bool useMaxSpeed = true;
unsigned long startMillis = 0, currentMillis = 0;
unsigned long period = 0; //ms

class Stepper {
public:
  AccelStepper stepper;
  bool isActive;
  bool useAcceleration;
  int speed;
  int speedFactor;

  Stepper(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin) :
    stepper(AccelStepper::DRIVER, stepPin, dirPin),
    isActive(false),
    speed(0),
    speedFactor(1) {
    stepper.setPinsInverted(false, false, true);
    stepper.setEnablePin(enablePin);
  }
  
  setup() {
    if (useMaxSpeed) {
      stepper.setMaxSpeed(0);
      stepper.setAcceleration(10000);
      stepper.moveTo(1000000);
    } else {
      stepper.setMaxSpeed(10000);
      stepper.setSpeed(0.0);
    }
  }

  run() {
    if (!isActive) return;

    if (useMaxSpeed) {
      if (stepper.distanceToGo() == 0)
          stepper.moveTo(-stepper.currentPosition());
        stepper.run();
    } else {
      stepper.runSpeed();
    }
  }

  setSpeed(int note) {
    speed = note;
    isActive = speed > 0;
    
    if (useMaxSpeed) {
      stepper.setMaxSpeed(speed * speedFactor);
    } else {
      stepper.setSpeed(speed * speedFactor);
    }
  }

  setSpeedFactor(int detune) {
    speedFactor = detune;
    setSpeed(speed);
  }
};

Stepper stepper1(X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN);
Stepper stepper2(Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN);
Stepper stepper3(Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN);
Stepper stepper4(E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN);

void setup() {
  Serial.begin(115200);

//  pinMode(X_MS1_PIN, OUTPUT);
//  pinMode(X_MS2_PIN, OUTPUT);
//  pinMode(X_MS3_PIN, OUTPUT);

  
  
  Serial.println("Ready");

  startMillis = millis();  //initial start time
}
int vol = 0;
float detune1 = 1, detune2 = 1, detune3 = 1, detune4 = 1;
int note1 = 0,  note2 = 0, note3 = 0, note4 = 0;
bool pulseOn = true;
bool mono = true;

void loop () {
  currentMillis = millis();
  handleSerial();

  if (currentMillis - startMillis >= period) {
    startMillis = currentMillis; 
    pulseOn = !pulseOn;
  }

  if (pulseOn) {
      if (stepper1Active) {
//        if (stepper1.distanceToGo() == 0)
//          stepper1.moveTo(-stepper1.currentPosition());
        stepper1.runSpeed();
      }
      if (stepper2Active) {
//        if (stepper2.distanceToGo() == 0)
//          stepper2.moveTo(-stepper2.currentPosition());
        stepper2.runSpeed();
      }
      if (stepper3Active) {
//        if (stepper3.distanceToGo() == 0)
//          stepper3.moveTo(-stepper3.currentPosition());
        stepper3.runSpeed();
      }
      if (stepper4Active) {
//        if (stepper4.distanceToGo() == 0)
//          stepper4.moveTo(-stepper4.currentPosition());
        stepper4.runSpeed();
      }
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

    pulseOn = n > 0 && n < 5;
    
    if (mono && n > 0 && n < 5) {
      note1 = note;
      stepper1.setSpeed(note * detune1);
      stepper1Active = true;
      note2 = note;
      stepper2.setSpeed(note * detune2);
      stepper2Active = true;
      note3 = note;
      stepper3.setSpeed(note * detune3);
      stepper3Active = true;
      note4 = note;
      stepper4.setSpeed(note * detune4);
      stepper4Active = true;
    } else {
      if (n == 1) {
        note1 = note;
        stepper1.setSpeed(note * detune1);
        stepper1Active = true;
      }
      if (n == 2) {
        note2 = note;
        stepper2.setSpeed(note * detune2);
        stepper2Active = true;
      }
      if (n == 3) {
        note3 = note;
        stepper3.setSpeed(note * detune3);
        stepper3Active = true;
      }
      if (n == 4) {
        note4 = note;
        stepper4.setSpeed(note * detune4);
        stepper4Active = true;
      }
    }
    
    if (n == 10) {
      period = ((50000 * lo) / 127) / 100;
      Serial.println(period);
    }
    if (n == 11) {
      float f = 0.059463 * ((lo / 127.0) * 2 - 1);
      if (mono) {
        detune1 = (1 + 0*f);
        detune2 = (1 + 0.25*f);
        detune3 = (1 + 0.5*f);
        detune4 = (1 + 1.0*f);
      } else {
        detune1 = detune2 = detune3 = detune4 = (1 + f);
      }

      Serial.println(detune1);
//      Serial.println(detune2);
//      Serial.println(detune3);
//      Serial.println(detune4);
      stepper1.setSpeed(note1 * detune1);
      stepper2.setSpeed(note2 * detune2);
      stepper3.setSpeed(note3 * detune3);
      stepper4.setSpeed(note4 * detune4);
    }
//    Serial.print(" ");
//    Serial.println(note);
//    switch (ch) {
//      case 'w':
//        stepper1.setSpeed(300);
//        stepper2.setSpeed(300);
//        break;
//      case 'q':
//        stepper1.setSpeed(1000);
//        stepper2.setSpeed(1000);
//        break;
//      case 'a':
//        stepper1.setSpeed(stepper1.speed() + 20);
//        stepper2.setSpeed(stepper2.speed() + 20);
//        break;
//      case 's':
//        stepper1.setSpeed(stepper1.speed() - 20);
//        stepper2.setSpeed(stepper2.speed() - 20);
//        break;
//      case '1':
//        stepper1Active = !stepper1Active;
//        break;
//      case '2':
//        stepper2Active = !stepper2Active;
//        break;
//      case '3':
//        stepper3Active = !stepper3Active;
//        break;
//      case '5': // 1
//        changeVolume(2);
//        break;
//      case '6': // 1/2
//        changeVolume(1);
//        break;
//      case '7': // 1/4
//        changeVolume(3);
//        break;
//      case '8': // 1/8
//        changeVolume(4);
//        break;
//      case '9': // 1/16
//        changeVolume(5);
//        break;
//    }

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

