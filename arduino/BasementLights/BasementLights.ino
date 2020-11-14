#define SOLID -1
#define OFF -2

int bluePin = 2;
int greenPin = 3;
int yellowPin = 4;

int blueState = LOW;
int greenState = LOW;

bool initialized = false;

// mode definition
void binaryCount();
void alternate();

void (*modes[])() = {
  binaryCount,
  alternate
};

// the number of times the mode should be repeated
int modeRepeat[] = {
  4, //binaryCount
  8, //alternate
};

// the number of iterations for one complete execution of the mode
int modeLoops[] = {
  4, //binaryCount
  2, //alternate
};

int modeLoopDelay[] = {
  1000, //binaryCount
  1000, //alternate
};

String modeCommands[] = {
  "MODE_0",
  "MODE_1"
};

// mode state
int mode = 0;
int NUM_MODES = 2;
boolean autoCycle = true;
int modeIterationNumber = 0; // the current count of the outer mode repeat loop
int modeLoopNumber = 0; // the current count of the inner loop for one cycle of a mode

//boolean firstOfMode;
//boolean firstOfOuterMode;


void setup() {
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  Serial.begin(9600);

  randomSeed(analogRead(0) * millis());
}

int value = 0;

void loop() {

  if (!initialized) {
    int initLoopCount = 0;
    while (Serial.available() == 0) {
      digitalWrite(yellowPin, initLoopCount % 2);
      initLoopCount = 1 - initLoopCount;
      delay(200);
    }

    initialize();
    delay(10);
  }

  if (autoCycle) {
    for (mode = 0; mode < NUM_MODES; mode++) {
      for (modeIterationNumber = 0; modeIterationNumber < modeRepeat[mode]; modeIterationNumber++) {
        for (modeLoopNumber = 0; modeLoopNumber < modeLoops[mode]; modeLoopNumber++) {
          modes[mode]();
          delay(modeLoopDelay[mode]);

          // check if a command was issued; if so, the command will reset the state and we'll start at the top
          if (checkCommand()) {
            return;
          }
        }
      }
      reset();
    }
  }
  else if (mode >= 0 && mode < NUM_MODES) {
    // a mode was explicitly selected, so just run the inner loop repeatedly
    for (modeLoopNumber = 0; modeLoopNumber < modeLoops[mode]; modeLoopNumber++) {
      modes[mode]();
      delay(modeLoopDelay[mode]);

      if (checkCommand()) {
        return;
      }
    }
  }
  else {
    // SOLID or OFF were selected, and would have already been set, so do nothing
    delay(10);

    checkCommand(); // we'll always start at the top of the loop so no need for an if here
  }


//  if (Serial.available() > 0) {
//    String data = Serial.readStringUntil('\n');
//    if (data.equals("BLUE_ON")) {
//      if (blueState == LOW) {
//        digitalWrite(bluePin, HIGH);
//        Serial.println(data + " SUCCESS");
//        blueState = HIGH;
//      }
//      else {
//        Serial.println(data + " NO_CHANGE");
//      }
//    }
//    else if (data.equals("GREEN_ON")) {
//      if (greenState == LOW) {
//        digitalWrite(greenPin, HIGH);
//        Serial.println(data + " SUCCESS");
//        greenState = HIGH;
//      }
//      else {
//        Serial.println(data + " NO_CHANGE");
//      }
//    }
//    else if (data.equals("BLUE_OFF")) {
//      if (blueState == HIGH) {
//        digitalWrite(bluePin, LOW);
//        Serial.println(data + " SUCCESS");
//        blueState = LOW;
//      }
//      else {
//        Serial.println(data + " NO_CHANGE");
//      }
//    }
//    else if (data.equals("GREEN_OFF")) {
//      if (greenState == HIGH) {
//        digitalWrite(greenPin, LOW);
//        Serial.println(data + " SUCCESS");
//        greenState = LOW;
//      }
//      else {
//        Serial.println(data + " NO_CHANGE");
//      }
//    }
//    else {
//      Serial.println("Unknown command: '" + data + "'");
//    }
//    Serial.flush();
//    delay(10);
//  }
}

void initialize() {

//  firstOfOuterMode = true;
  
  String data = Serial.readStringUntil('\n');
  Serial.println("INITIALIZED");
  initialized = true;
  digitalWrite(yellowPin, LOW);
}

boolean checkCommand() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

//    boolean validCommand = false;

    if (data.equals("SOLID")) {
      solid();
      mode = SOLID;
      autoCycle = false;
      return true;
//      validCommand = true;
    }
    else if (data.equals("OFF")) {
      off();
      mode = OFF;
      autoCycle = false;
      return true;
//      validCommand = true;
    }
    else if (data.equals("AUTOCYCLE_ON")) {
      autoCycle = true;
      mode = random(0, NUM_MODES);
      return true;
//      validCommand = true;
    }
    else {
      for (int m = 0; m < NUM_MODES; m++) {
        if (data.equals(modeCommands[m])) {
          mode = m;
          autoCycle = false;
          return true;
//          validCommand = true;
          break;
        }
      }
    }
  }

  return false;
}

void reset() {
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
}

void solid() {
  digitalWrite(bluePin, HIGH);
  digitalWrite(greenPin, HIGH);
}

void off() {
  reset();
}

void binaryCount() {
  digitalWrite(bluePin, modeLoopNumber % 2);
  digitalWrite(greenPin, (modeLoopNumber / 2) % 2);
}

void alternate() {
  digitalWrite(bluePin, modeLoopNumber % 2);
  digitalWrite(greenPin, 1 - (modeLoopNumber % 2));
}

