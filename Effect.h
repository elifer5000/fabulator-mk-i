#define NUM_EFFECTS  4
enum EffectsEnum {
  ePulse=0, eVolumeSwell, eVibrato, eRandom
};

class PeriodicEffect {
protected:
  unsigned long period; // ms
  unsigned long startMillis; // ms
  bool toggle;
  bool prevToggle;

public:
  PeriodicEffect() : period(0), toggle(true), prevToggle(true) {}

  void setup(unsigned long _startMillis) {
    startMillis = _startMillis;
  }

  void setPeriod(unsigned long _period) {
    if (period == 0) {
     prevToggle = toggle = true; // If it was off, start from an 'on' state
    }

    period = _period;
  }

  bool isActive() {
    return period > 0;
  }

  virtual void run(unsigned long currentMillis) {
    prevToggle = toggle;
    if (currentMillis - startMillis >= period) {
      startMillis = currentMillis; 
      toggle = !toggle;
    } 
  }

  virtual float getVolume() {
    return 1.0;
  }

  virtual float getSpeedFactor() {
    return 1.0;
  }
};

class PulseEffect : public PeriodicEffect {
public:
  PulseEffect() {};

  float getVolume() {
    return toggle ? 1.0 : 0.0;
  }
};

class VolumeSwellEffect : public PeriodicEffect {
protected:
  unsigned int volume;

public:
  VolumeSwellEffect() : volume(1) {}


  float getVolume() {
    if (prevToggle != toggle)
      volume++;

    return volume % 5;
  }
};

class VibratoEffect : public PeriodicEffect {
public:
  VibratoEffect() {}
};

class RandomEffect : public PeriodicEffect {
public:
  RandomEffect() {}
};

class EffectManager {
protected:
  PeriodicEffect* effects[NUM_EFFECTS];

public:
  EffectManager() {
    effects[ePulse] = new PulseEffect();
    effects[eVolumeSwell] = new VolumeSwellEffect();
    effects[eVibrato] = new VibratoEffect();
    effects[eRandom] = new RandomEffect();
  }

  void setup(unsigned long initialMillis) {
    for (int i = 0; i < NUM_EFFECTS; i++) {
      effects[i]->setup(initialMillis);
    }
  }

  void run(unsigned long currentMillis, int& volume, float& retSpeedFactor) {
    for (int i = 0; i < NUM_EFFECTS; i++) {
      effects[i]->run(currentMillis);
    }

    if (effects[ePulse]->isActive()) {
      volume *= effects[ePulse]->getVolume();
    }
    if (volume > 0 && effects[eVolumeSwell]->isActive()) {
      volume = effects[eVolumeSwell]->getVolume();
    }

    retSpeedFactor = 1.0;
    
    // retSpeedFactor *= effects[i]->getSpeedFactor();
  }

  void setPeriod(EffectsEnum effectType, unsigned long _period) {
    effects[effectType]->setPeriod(_period);
  }

  void setRange(EffectsEnum effectType, unsigned long _range) {

  }
};