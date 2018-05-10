#include "Stepper.h"
#include "StepperManager.h"
#include "Config.h"


unsigned long currentMillis = 0;
int m_i;

void setup() {
  Serial.begin(115200);
  
  unsigned long startMillis = millis();
  for (m_i = 0; m_i < NUM_MANAGERS; m_i++) {
    manager[m_i].setup(startMillis, pins[m_i]);
  }
}

void loop () {
  currentMillis = millis();
  handleSerial();

  for (m_i = 0; m_i < NUM_MANAGERS; m_i++) {
    manager[m_i].run(currentMillis);
  }
}

void handleSerial() {
  if (Serial.available() >= 3) {
    byte m0 = Serial.read();
    byte m1 = Serial.read();
    byte m2 = Serial.read();
    
//    Serial.print("Got: ");
//    Serial.println((m0 & 0xF));
//    Serial.print("   ");
//    Serial.print(m1);
//    Serial.print(" ");
//    Serial.println(m2); 

    byte channel = (m0 & 0xF);
    if (channel < NUM_MANAGERS)
      manager[channel].handleMidi(m0, m1, m2);
  }
}



