// MIDI constants
#define NOTE_OFF            0x80
#define NOTE_ON             0x90
#define CONTROLLER_CHANGE   0xB0

#define NOTES_BUFFER_SZ      32

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
  StepperManager(int _numSteppers, bool _mono = false) : numSteppers(_numSteppers), numNotes(0), numOldNotes(0), isMono(_mono) {
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