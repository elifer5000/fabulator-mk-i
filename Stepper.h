#include "AccelStepper.h"

class Stepper {
protected:
    AccelStepper stepper;
    uint8_t ms1;
    uint8_t ms2;
    uint8_t ms3;
    bool isActive;
    bool useAcceleration;
    int speed;
    float speedFactor1;
    float speedFactor2;
    int acceleration;
    unsigned long period; // ms
    unsigned long startMillis; // ms
    bool usePulse;
    bool pulseOn;
    int volume;

    void setupStepper() {
      if (useAcceleration) {
        stepper.setMaxSpeed(0);
        stepper.setAcceleration(acceleration);
        stepper.moveTo(1000000);
      } else {
        stepper.setSpeed(0.0);
        stepper.setMaxSpeed(10000);        
      }
    }

public:
  Stepper(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin, uint8_t _ms1 = -1, uint8_t _ms2 = -1, uint8_t _ms3 = -1) :
    stepper(AccelStepper::DRIVER, stepPin, dirPin),
    ms1(_ms1), ms2(_ms2), ms3(_ms3),  
    isActive(false),
    useAcceleration(false),
    speed(0),
    speedFactor1(1.0),
    speedFactor2(1.0),
    acceleration(10000),
    period(0),
    startMillis(0),
    pulseOn(true),
    usePulse(true),
    volume(5) {
    stepper.setPinsInverted(false, false, true);
    stepper.setEnablePin(enablePin);

    pinMode(ms1, OUTPUT);
    pinMode(ms2, OUTPUT);
    pinMode(ms3, OUTPUT);  
  }
  
  void setup(unsigned long initialMillis) {
    startMillis = initialMillis;  //initial start time
    setupStepper();
  }

  void run(unsigned long currentMillis) {
    if (!isActive) return;

    if (currentMillis - startMillis >= period) {
      startMillis = currentMillis; 
      pulseOn = !pulseOn;
    } 

    if (usePulse && !pulseOn) return;
    
    if (useAcceleration) {
      if (stepper.distanceToGo() == 0)
          stepper.moveTo(-stepper.currentPosition());
        stepper.run();
    } else {
      stepper.runSpeed();
    }
  }

  // note is in Hz
  void setNote(int note, int volume) {
    speed = note;
    isActive = speed > 0;
    pulseOn = isActive;
    startMillis = millis();
    
    if (useAcceleration) {
      stepper.setMaxSpeed(speed * speedFactor1 * speedFactor2);
    } else {
      stepper.setSpeed(speed * speedFactor1 * speedFactor2);
    }

     setVolume(volume);
  }

  void setDetune(float detune) {
    speedFactor1 = detune;
    setNote(speed, volume);
  }

  void setPitchShift(float detune) {
    speedFactor2 = detune;
    setNote(speed, volume);
  }

  void setPeriod(unsigned long p) {
    period = p;
  }

  void setVolume(int val) {
    switch (val) {
          case 5: // 1
            digitalWrite(ms1, LOW);
            digitalWrite(ms2, LOW);
            digitalWrite(ms3, LOW);
            break;
          case 4: // 1/2
            digitalWrite(ms1, HIGH);
            digitalWrite(ms2, LOW);
            digitalWrite(ms3, LOW);
            break;
          case 3: // 1/4
            digitalWrite(ms1, LOW);
            digitalWrite(ms2, HIGH);
            digitalWrite(ms3, LOW);
            break;
          case 2: // 1/8
            digitalWrite(ms1, HIGH);
            digitalWrite(ms2, HIGH);
            digitalWrite(ms3, LOW);
            break;
          case 1: // 1/16
            digitalWrite(ms1, HIGH);
            digitalWrite(ms2, HIGH);
            digitalWrite(ms3, HIGH);
            break;
    }
  }

  bool getIsActive() {
    return isActive;
  }

  int getNote() {
    return speed;
  }

  int getVolume() {
    return volume;
  }
};
