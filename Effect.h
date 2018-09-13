#define NUM_EFFECTS  4
enum EffectsEnum {
  ePulse=0, eVolumeSwell, eVibrato, eRandom
};

class PeriodicEffect {
protected:
  unsigned long period; // ms
  unsigned long startMillis; // ms
  unsigned long currentMillis; // ms
  float range;
  bool toggle;
  bool prevToggle;

public:
  PeriodicEffect() : period(0), range(0.0), toggle(true), prevToggle(true) {}

  void setup(unsigned long _startMillis) {
    startMillis = _startMillis;
    prevToggle = toggle = true;
  }

  void setPeriod(unsigned long _period) {
    if (period == 0) {
      prevToggle = toggle = true; // If it was off, start from an 'on' state
    }

    period = _period;
  }

  void setRange(float _range) {
    range = _range;
  }

  bool isActive() {
    return period > 0;
  }

  virtual void run(unsigned long _currentMillis) {
    prevToggle = toggle;
    currentMillis = _currentMillis;
    if (currentMillis - startMillis >= period) {
      startMillis = currentMillis; 
      toggle = !toggle;
    } 
  }

  virtual unsigned int getVolume() {
    return 1;
  }

  virtual float getSpeedFactor() {
    return 1.0;
  }
};

class PulseEffect : public PeriodicEffect {
public:
  PulseEffect() {};

  unsigned int getVolume() {
    return toggle ? 1 : 0;
  }
};

class VolumeSwellEffect : public PeriodicEffect {
protected:
  unsigned int volume;

public:
  VolumeSwellEffect() : volume(1) {}


  unsigned int getVolume() {
    if (prevToggle != toggle)
      volume++;

    return volume % 6;
  }
};

class VibratoEffect : public PeriodicEffect {
protected:
  int sign;
public:
  VibratoEffect() : sign(2) {}

  float getSpeedFactor() {
    if (prevToggle != toggle)
      sign = -sign;

  //   if (cents > 0)  //range is between 0 and 1
  //   return 1 + 0.059463094*cents;

  // return 1 + 0.056125687*cents;

    //float factor = 10; // Range seems to be too small, maybe need a factor
    float y = range * sign * (-0.5 + (currentMillis - startMillis) / period);
    
    return (y > 0) ? (1 + 0.059463094*y) : (1 + 0.056125687*y);
  }
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
    if (effects[eVibrato]->isActive()) {
      retSpeedFactor = effects[eVibrato]->getSpeedFactor();
    }
  }

  void setPeriod(EffectsEnum effectType, unsigned long _period) {
    effects[effectType]->setPeriod(_period);
  }

  void setRange(EffectsEnum effectType, float _range) {
    effects[effectType]->setRange(_range);
  }
};