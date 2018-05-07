// Arduino PINS
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MS1_PIN           4 
#define X_MS2_PIN           5
#define X_MS3_PIN           6

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56
#define Y_MS1_PIN          40 
#define Y_MS2_PIN          42
#define Y_MS3_PIN          44

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_MS1_PIN          41 
#define Z_MS2_PIN          43
#define Z_MS3_PIN          45

#define E_STEP_PIN         26
#define E_DIR_PIN          28
#define E_ENABLE_PIN       24
#define E_MS1_PIN          31 
#define E_MS2_PIN          33
#define E_MS3_PIN          35

#define Q_STEP_PIN         36
#define Q_DIR_PIN          34
#define Q_ENABLE_PIN       30
#define Q_MS1_PIN          23 
#define Q_MS2_PIN          25
#define Q_MS3_PIN          27

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

// MIDI constants
#define NOTE_OFF            0x80
#define NOTE_ON             0x90
#define CONTROLLER_CHANGE   0xB0

#define NOTES_BUFFER_SZ      32

#include "Stepper.h"

unsigned long currentMillis = 0;

int pins[][6] = {
  {X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, X_MS1_PIN, X_MS2_PIN, X_MS3_PIN},
  {Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, Y_MS1_PIN, Y_MS2_PIN, Y_MS3_PIN},
  {Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, Z_MS1_PIN, Z_MS2_PIN, Z_MS3_PIN},
  {E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN, E_MS1_PIN, E_MS2_PIN, E_MS3_PIN},
  {Q_STEP_PIN, Q_DIR_PIN, Q_ENABLE_PIN, Q_MS1_PIN, Q_MS2_PIN, Q_MS3_PIN}
};

float detuneCalc(float maxDetune, int val) {
  float cents = maxDetune*((val / 127.0) * 2.0 - 1.0);

  return pow(2.0, cents/1200.0);
}

int freqCalc(int note) {
  return pow(2.0, (note - 69) / 12.0) * 440.0;
}

int convertVolume(int volume) {
  if (volume > 80) return 5;
  if (volume > 40) return 4;
  if (volume > 20) return 3;
  if (volume > 10) return 2;

  return 1;
}

class StepperManager {
private:
  Stepper** steppers;
  int numSteppers;
  int numNotes;
  int numOldNotes;
  int oldNotesStack[NOTES_BUFFER_SZ];
  int oldVolumesStack[NOTES_BUFFER_SZ];
  bool isMono;

public:
  StepperManager(int _numSteppers) : numSteppers(_numSteppers), numNotes(0), numOldNotes(0), isMono(false) {
    steppers = new Stepper*[numSteppers];
  }

  void setup(int startMillis, int pins[][6]) {
    for (int i = 0; i < numSteppers; i++) {
      steppers[i] = new Stepper(pins[i][0], pins[i][1], pins[i][2], pins[i][3], pins[i][4], pins[i][5]);
      steppers[i]->setup(startMillis);
    }
  }

  void run(int currentMillis) {
    for (int i = 0; i < numSteppers; i++) {
      steppers[i]->run(currentMillis);
    }
  }

  void setPolyNoteOn(int note, int volume) {
    int i;
    if (numNotes < numSteppers) {
      // Search for next available motor
      for (i = 0; i < numSteppers; i++) {
        if (!steppers[i]->getIsActive()) {
          steppers[i]->setNote(note, volume);
          numNotes++;
          break;
        } 
      }    
    } else {
      // Search for active motor with closest distance
      int closestDist = 32000;
      int dist, idx = -1;
      for (i = 0; i < numSteppers; i++) {
        dist = note - steppers[i]->getNote();
        dist = abs(dist);
        if (dist < closestDist) {
          closestDist = dist;
          idx = i;
        }
      }
      
      if (numOldNotes < NOTES_BUFFER_SZ) {
        oldNotesStack[numOldNotes] = steppers[idx]->getNote();
        oldVolumesStack[numOldNotes++] = steppers[idx]->getVolume();
      }
      steppers[idx]->setNote(note, volume);
    }
  }
  
  void setPolyNoteOff(int note) {
    int i, j, oldNote, oldVolume;
    // Search for motor with this note
    for (i = 0; i < numSteppers; i++) {
      if (note == steppers[i]->getNote()) {
        if (numOldNotes > 0) {  // Play old note from stack
          oldNote = oldNotesStack[--numOldNotes];
          oldVolume = oldVolumesStack[numOldNotes];
          steppers[i]->setNote(oldNote, oldVolume);       
        } else {  // Turn off motor
          steppers[i]->setNote(0, 0);
          numNotes--;
        }
        break;
      }
    }

    if (i == numSteppers) { // No active motor with this note
      for (i = 0; i < numOldNotes; i++) {
        if (oldNotesStack[i] == note) {
          // Remove from stack
          numOldNotes--;
          for (j = i; j < numOldNotes; j++) {
            oldNotesStack[j] = oldNotesStack[j+1];
            oldVolumesStack[j] = oldVolumesStack[j+1];
          }
          break;
        }
      }
    }
  }

  void setMonoNoteOn(int note, int volume) {
    int i;

    if (steppers[0]->getIsActive() && numOldNotes < NOTES_BUFFER_SZ) {
      oldNotesStack[numOldNotes] = steppers[0]->getNote();
      oldVolumesStack[numOldNotes++] = steppers[0]->getVolume();
    }
      
    for (i = 0; i < numSteppers; i++) {
      steppers[i]->setNote(note, volume);
    }  
  }

  void setMonoNoteOff(int note) {
    int i, j, oldNote, oldVolume;
    
    if (note == steppers[0]->getNote()) { // Is this note currently playing?
      if (numOldNotes > 0) {  // Play old note from stack
        oldNote = oldNotesStack[--numOldNotes];
        oldVolume = oldVolumesStack[numOldNotes];
        for (i = 0; i < numSteppers; i++) {
          steppers[i]->setNote(oldNote, oldVolume);
        }
      } else {  // Turn off motors
        for (i = 0; i < numSteppers; i++) {
          steppers[i]->setNote(0, 0);
        }
      }
    } else {
      for (i = 0; i < numOldNotes; i++) {
        if (oldNotesStack[i] == note) {
          // Remove from stack
          numOldNotes--;
          for (j = i; j < numOldNotes; j++) {
            oldNotesStack[j] = oldNotesStack[j+1];
            oldVolumesStack[j] = oldVolumesStack[j+1];
          }
          break;
        }
      }
    }
  }

  void setNoteOn(int note, int volume) {
    if (isMono) setMonoNoteOn(note, volume);
    else setPolyNoteOn(note, volume);
  }

  void setNoteOff(int note) {
    if (isMono) setMonoNoteOff(note);
    else setPolyNoteOff(note);
  }

  void handleMidi(byte m0, byte m1, byte m2) {
    int i;
    float f;
    if ((m0 & 0xF0) == NOTE_ON)
      setNoteOn(freqCalc(m1), convertVolume(m2));
    else if ((m0 & 0xF0) == NOTE_OFF)
      setNoteOff(freqCalc(m1));
    else if ((m0 & 0xF0) == CONTROLLER_CHANGE) {
      switch (m1) {
        case 21:
          f = ((50000 * m2) / 127) / 100;  // Up to 500ms
          for (i = 0; i < numSteppers; i++) {
            steppers[i]->setPeriod(f);
          }
          break;
        case 22:
          if (isMono) {
            f = detuneCalc(33.3, m2); // Detune up to 1/3 semitone
            for (i = 1; i < numSteppers; i++) {
              steppers[i]->setDetune(((float)i / (numSteppers-1))*f);
            }
          } else {
            f = detuneCalc(100.0, m2); // Detune up to 1 semitone
            for (i = 0; i < numSteppers; i++) {
              steppers[i]->setDetune(f);
            }
          }
          break;
        case 23:
          f = detuneCalc(1200.0, m2);  // Detune up to 1 octave
          for (i = 0; i < numSteppers; i++) {
            steppers[i]->setPitchShift(f);
          }           
          break;
      }
    }
  }
};

StepperManager manager(5);

void setup() {
  Serial.begin(115200);
  
  unsigned long startMillis = millis();
  manager.setup(startMillis, pins);
  
  Serial.println("Ready");
}

void loop () {
  currentMillis = millis();
  handleSerial();

  manager.run(currentMillis);
}

void handleSerial() {
  if (Serial.available() >= 3) {
    byte m0 = Serial.read();
    byte m1 = Serial.read();
    byte m2 = Serial.read();
    
//    Serial.print("Got: ");
//    Serial.print((m0 & 0xF0) == NOTE_ON);
//    Serial.print("   ");
//    Serial.print(m1);
//    Serial.print(" ");
//    Serial.println(m2); 

    manager.handleMidi(m0, m1, m2);
  }
}



