#include "AccelStepper.h"

class Stepper {
protected:
    AccelStepper stepper;
    bool isActive;
    bool useAcceleration;
    int speed;
    float speedFactor;
    int acceleration;
    unsigned long period; // ms
    unsigned long startMillis; // ms
    bool usePulse;
    bool pulseOn;

    setupStepper() {
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
  Stepper(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin) :
    stepper(AccelStepper::DRIVER, stepPin, dirPin),
    isActive(false),
    useAcceleration(true),
    speed(0),
    speedFactor(1.0),
    acceleration(10000),
    period(0),
    startMillis(0),
    pulseOn(true),
    usePulse(true) {
    stepper.setPinsInverted(false, false, true);
    stepper.setEnablePin(enablePin);
  }
  
  setup(unsigned long initialMillis) {
    startMillis = initialMillis;  //initial start time
    setupStepper();
  }

  run(unsigned long currentMillis) {
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
  setNote(int note) {
    speed = note;
    isActive = speed > 0;
    pulseOn = isActive;
    
    if (useAcceleration) {
      stepper.setMaxSpeed(speed * speedFactor);
    } else {
      stepper.setSpeed(speed * speedFactor);
    }
  }

  setDetune(float detune) {
    speedFactor = detune;
    setNote(speed);
  }

  setPeriod(unsigned long p) {
    period = p;
  }
};
