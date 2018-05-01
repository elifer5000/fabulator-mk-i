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

#define NUM_STEPPERS      5
#define NOTES_BUFFER_SZ      32

#include "Stepper.h"

unsigned long currentMillis = 0;
bool mono = false;

int pins[][6] = {
  {X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, X_MS1_PIN, X_MS2_PIN, X_MS3_PIN},
  {Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, Y_MS1_PIN, Y_MS2_PIN, Y_MS3_PIN},
  {Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, Z_MS1_PIN, Z_MS2_PIN, Z_MS3_PIN},
  {E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN, E_MS1_PIN, E_MS2_PIN, E_MS3_PIN},
  {Q_STEP_PIN, Q_DIR_PIN, Q_ENABLE_PIN, Q_MS1_PIN, Q_MS2_PIN, Q_MS3_PIN}
};

Stepper* steppers[NUM_STEPPERS];

class StepperManager {
private:
  int numNotes;
  int numOldNotes;
  int oldNotesStack[NOTES_BUFFER_SZ];
  int oldVolumesStack[NOTES_BUFFER_SZ];

public:
  StepperManager() : numNotes(0), numOldNotes(0) {

  }

  void setPolyNoteOn(int note, int volume) {
    int i;
    if (numNotes < NUM_STEPPERS) {
      // Search for next available motor
      for (i = 0; i < NUM_STEPPERS; i++) {
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
      for (i = 0; i < NUM_STEPPERS; i++) {
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
    for (i = 0; i < NUM_STEPPERS; i++) {
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

    if (i == NUM_STEPPERS) { // No active motor with this note
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
      
    for (i = 0; i < NUM_STEPPERS; i++) {
      steppers[i]->setNote(note, volume);
    }  
  }

  void setMonoNoteOff(int note) {
    int i, j, oldNote, oldVolume;
    
    if (note == steppers[0]->getNote()) { // Is this note currently playing?
      if (numOldNotes > 0) {  // Play old note from stack
        oldNote = oldNotesStack[--numOldNotes];
        oldVolume = oldVolumesStack[numOldNotes];
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setNote(oldNote, oldVolume);
        }
      } else {  // Turn off motors
        for (i = 0; i < NUM_STEPPERS; i++) {
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
    if (mono) setMonoNoteOn(note, volume);
    else setPolyNoteOn(note, volume);
  }

  void setNoteOff(int note) {
    if (mono) setMonoNoteOff(note);
    else setPolyNoteOff(note);
  }
};

StepperManager manager;

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

    bool isNote = n == 1 || n == 2;
    int i;
    if (isNote) {
      if (n == 1) 
        manager.setNoteOn(note, convertVolume(speed));
      else
        manager.setNoteOff(note);
    }
    
    if (n == 21) {
      float period = ((50000 * lo) / 127) / 100;
//      Serial.println(period);
      for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setPeriod(period);
      }
    }

    if (n == 22) { 
      float f;
      if (mono) {
        f = detuneCalc(33.3*((lo / 127.0) * 2.0 - 1.0)); // Detune up to 1/3 semitone
        for (i = 1; i < NUM_STEPPERS; i++) {
          steppers[i]->setDetune(((float)i / (NUM_STEPPERS-1))*f);
        }
      } else {
        f = detuneCalc(100.0*((lo / 127.0) * 2.0 - 1.0)); // Detune up to 1 semitone
        for (i = 0; i < NUM_STEPPERS; i++) {
          steppers[i]->setDetune(f);
        }
      }

//      Serial.println(f);
    }

    if (n == 23) {  // Detune up to 1 octave
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
  if (volume > 80) return 5;
  if (volume > 40) return 4;
  if (volume > 20) return 3;
  if (volume > 10) return 2;

  return 1;
}

