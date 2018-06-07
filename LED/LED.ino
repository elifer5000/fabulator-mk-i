#define RED_PIN_1     7
#define GREEN_PIN_1   6
#define BLUE_PIN_1    5

class Color {
public:
  int r;
  int g;
  int b;

  Color() : r(0), g(0), b(0) {}
};

Color globalCol; // reusable color

class LED {
private:
  int redPin;
  int greenPin;
  int bluePin;
  float factor;
  Color color;

public:
  LED(int _r, int _g, int _b) : redPin(_r), greenPin(_g), bluePin(_b), factor(1.0) { 
  }

  void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
  }

  void setColor(int r, int g, int b) {
    if (r != color.r) {
      analogWrite(redPin, r * factor);
      color.r = r;
    }
    if (g != color.g) {
      analogWrite(greenPin, g * factor);
      color.g = g;
    }
    if (b != color.b) {
      analogWrite(bluePin, b * factor);
      color.b = b;
    }
  }

  void writeCurrentColor() {
    analogWrite(redPin, color.r * factor);
    analogWrite(greenPin, color.g * factor);
    analogWrite(bluePin, color.b * factor);
  }

  void setFactor(int volume) {
    factor = min(1.0, volume / 5.0);
  }
};

const uint64_t lightFreqRedLower = 400000000000000;
const uint64_t speedOfLightVacuum = 299792458000000000; // nm/sec

int convertSoundFreqToLightWavelength(int soundFrequency) {
  uint64_t lightFrequency = soundFrequency;
  
  while (lightFrequency < lightFreqRedLower) {
    lightFrequency *= 2;
  }

  return speedOfLightVacuum / lightFrequency;
}

void getColorFromWaveLength(float wavelength) {
  int IntensityMax = 255;

  float Factor;
  
  // Color values in the range -1 to 1
  float Blue;
  float Green;
  float Red;

  if (wavelength >= 350 && wavelength < 440) {
    // From Purple (1, 0, 1) to Blue (0, 0, 1), with increasing intensity (set below)
    Red = -(wavelength - 440.0) / 90;
    Green = 0.0;
    Blue  = 1.0;
    
  } else if (wavelength >= 440 && wavelength < 490) {
    // From Blue (0, 0, 1) to Cyan (0, 1, 1) 
    Red = 0.0;
    Green = (wavelength - 440.0) / 50;
    Blue  = 1.0;
    
  } else if (wavelength >= 490 && wavelength < 510) {
    // From  Cyan (0, 1, 1)  to  Green (0, 1, 0) 
    Red = 0.0;
    Green = 1.0;
    Blue = -(wavelength - 510.0) / 20;
    
  } else if (wavelength >= 510 && wavelength < 580) { 
    // From  Green (0, 1, 0)  to  Yellow (1, 1, 0)
    Red = (wavelength - 510.0) / 70;
    Green = 1.0;
    Blue = 0.0;
    
  } else if (wavelength >= 580 && wavelength < 645) {
    // From  Yellow (1, 1, 0)  to  Red (1, 0, 0)
    Red = 1.0;
    Green = -(wavelength - 645.0) / 65;
    Blue = 0.0;
    
  } else if (wavelength >= 645 && wavelength <= 780) {
    // Solid Red (1, 0, 0), with decreasing intensity (set below)
    Red = 1.0;
    Green = 0.0;
    Blue = 0.0;
    
  } else {
    Red = 0.0;
    Green = 0.0;
    Blue = 0.0;
  }
 
  // Intensity factor goes through the range:
  // 0.1 (350-420 nm) 1.0 (420-645 nm) 1.0 (645-780 nm) 0.2
 
  if (wavelength >= 350 && wavelength < 420) {
    Factor = 0.1 + 0.9*(wavelength - 350.0) / 70;
    
  } else if (wavelength >= 420 && wavelength < 645) {
    Factor = 1.0;
    
  } else if (wavelength >= 645 && wavelength <= 780) {
    Factor = 0.2 + 0.8*(780.0 - wavelength) / 135;
    
  } else {
    Factor = 0.0;
  }

  globalCol.r = factorAdjust(Red, Factor, IntensityMax);
  globalCol.g = factorAdjust(Green, Factor, IntensityMax);
  globalCol.b = factorAdjust(Blue, Factor, IntensityMax);
}

int factorAdjust(float color, float factor, int intensityMax) { 
  if (color == 0.0) {
    return 0;
  } else {
    return round(intensityMax * color * factor);
  }
}

LED led1(RED_PIN_1, GREEN_PIN_1, BLUE_PIN_1);

void setup() {
  Serial2.begin(115200); // Communicates with stepper motor
  // Serial.begin(9600);
  led1.setup();
}

void loop() {
  if (Serial2.available() >= 4) {
    byte ledNumber = Serial2.read();  // LED number
    byte freqHi = Serial2.read();  // Freq hi
    byte freqLo = Serial2.read();  // Freq lo
    byte volume = Serial2.read();  // Volume
    int freq = word(freqHi, freqLo);

    int lightWavelength = convertSoundFreqToLightWavelength(freq);
    getColorFromWaveLength(lightWavelength); // Updates globalCol

    if (ledNumber == 1) {
      led1.setFactor(volume);
      // Serial.println(volume);
      led1.setColor(globalCol.r, globalCol.g, globalCol.b);
    }
  }
}
