#include "Stepper.h"
#include "StepperManager.h"
#include "Config.h"
#include "MIDI/src/MIDI.h"

// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

unsigned long currentMillis = 0;
int m_i;


void handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (--channel < NUM_MANAGERS) // channel is 1-16 range
    manager[channel].handleNoteOn(pitch, velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (--channel <= NUM_MANAGERS) // channel is 1-16 range
    manager[channel].handleNoteOff(pitch);
}

void handleControlChange(byte channel, byte number, byte value) {
  if (--channel <= NUM_MANAGERS) // channel is 1-16 range
    manager[channel].handleControlChange(number, value);
}

void setup() {
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  Serial.begin(115200);   // For MIDI
  
  // Serial2.begin(115200);  // For LEDs
  unsigned long startMillis = millis();
  for (m_i = 0; m_i < NUM_MANAGERS; m_i++) {
    manager[m_i].setup(startMillis, pins[m_i]);
  }
}

// int colorNote = 440;
// byte col[] = {1, highByte(colorNote), lowByte(colorNote), 5};
void loop () {
  currentMillis = millis();
  MIDI.read();

  for (m_i = 0; m_i < NUM_MANAGERS; m_i++) {
    manager[m_i].run(currentMillis);
  }
  // Serial2.write(col, 4);
}




