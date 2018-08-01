#define CONFIG             1

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

#define PINS_X          {X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, X_MS1_PIN, X_MS2_PIN, X_MS3_PIN}
#define PINS_Y          {Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, Y_MS1_PIN, Y_MS2_PIN, Y_MS3_PIN}
#define PINS_Z          {Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, Z_MS1_PIN, Z_MS2_PIN, Z_MS3_PIN}
#define PINS_E          {E_STEP_PIN, E_DIR_PIN, E_ENABLE_PIN, E_MS1_PIN, E_MS2_PIN, E_MS3_PIN}
#define PINS_Q          {Q_STEP_PIN, Q_DIR_PIN, Q_ENABLE_PIN, Q_MS1_PIN, Q_MS2_PIN, Q_MS3_PIN}

#if CONFIG == 1     // 3 managers (1, 1 and 3 motors)
#define NUM_MANAGERS        3
StepperManager manager[NUM_MANAGERS] = { StepperManager(1), StepperManager(1), StepperManager(3) };

int pins[][3][6] = {
  { PINS_X },
  { PINS_Y },
  { PINS_Z, PINS_E, PINS_Q }
};

#elif CONFIG == 2   // 3 managers (1, 1 and 3 motors as mono)
#define NUM_MANAGERS        3
StepperManager manager[NUM_MANAGERS] = { StepperManager(1), StepperManager(1), StepperManager(3, true) };

int pins[][3][6] = {
  { PINS_X },
  { PINS_Y },
  { PINS_Z, PINS_E, PINS_Q }
};

#elif CONFIG == 3   // 1 manager (5 motors)
#define NUM_MANAGERS        1
StepperManager manager[NUM_MANAGERS] = { StepperManager(5) };

int pins[][5][6] = {
  { PINS_X, PINS_Y, PINS_Z, PINS_E, PINS_Q }
};

#elif CONFIG == 4   // 1 manager (5 motors as mono)
#define NUM_MANAGERS        1
StepperManager manager[NUM_MANAGERS] = { StepperManager(5, true) };

int pins[][5][6] = {
  { PINS_X, PINS_Y, PINS_Z, PINS_E, PINS_Q }
};

#elif CONFIG == 5 // 1 manager - 1 motor (use for testing individual motors)
#define NUM_MANAGERS        1
StepperManager manager[NUM_MANAGERS] = { StepperManager(1) };

int pins[][1][6] = {
  { PINS_X }
};

#endif
