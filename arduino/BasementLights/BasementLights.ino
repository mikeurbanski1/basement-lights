#define SOLID_MODE -1
#define OFF_MODE -2
#define RAINBOW_MODE -3
#define COLOR CRGB

#include "FastLED.h"

#define NUM_LEDS 32
#define DATA_PIN 3
#define CLOCK_PIN 13

CRGB ledStrip[NUM_LEDS];

CRGB OFF = CRGB(0, 0, 0);

int yellowPin = 4;

bool initialized = false;

COLOR getColorRGB(int r, int g, int b); // definition for use in colors array

/*
 * How to add a mode:
 * 
 * 1. Add a method definition
 * 2. Add the method to the array
 * 3. Set the number of times the mode should be repeated (in autocycle mode)
 * 4. Set the number of iterations that constitute one full cycle of the mode
 * 5. Set the delay between iterations in #4
 * 6. Add a new entry into modeCommands
 * 7. Increment NUM_MODES
 * 8. Implement the method
 */

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
  32, //binaryCount
  2, //alternate
};

// delay between iterations of the inner loop (i.e., delay between invocations of the mode method)
int modeLoopDelay[] = {
  333, //binaryCount
  1000, //alternate
};

// placeholders to accept mode commands
String modeCommands[] = {
  "MODE_0",
  "MODE_1"
};

int NUM_MODES = 2;

// mode state
int mode = SOLID_MODE;
boolean autoCycle = false;
int modeIterationNumber = 0; // the current count of the outer mode repeat loop
int modeLoopNumber = 0; // the current count of the inner loop for one cycle of a mode

int brightness = 40;

// colors

int NUM_COLORS = 9;

String colorNames[] = {
  "RED",
  "BLUE",
  "GREEN",
  "WHITE",
  "PINK",
  "YELLOW",
  "PURPLE",
  "CYAN",
  "ORANGE"
};

COLOR colors[] = {
  getColorRGB(255, 0, 0), //red
  getColorRGB(0, 0, 255), //blue
  getColorRGB(0, 255, 0), //green
  getColorRGB(255, 255, 255), //white
  getColorRGB(255, 20, 115), //pink
  getColorRGB(255, 255, 0), //yellow
  getColorRGB(128, 0, 128), //purple
  getColorRGB(0, 255, 255), //cyan
  getColorRGB(255, 100, 0) //orange
};

// The current color, if a solid mode. Undefined if the mode is not currently solid.
COLOR currentColor;

// misc state variables that CAN be used by different modes (no mode should expect these to be valid if another mode executes);
COLOR savedColor;

void setup() {

  FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, GRB>(ledStrip, NUM_LEDS);
  FastLED.setBrightness(brightness);
  
  pinMode(yellowPin, OUTPUT);
  Serial.begin(9600);

  randomSeed(analogRead(0) * millis());
  reset();

  initialize();
}

int value = 0;

void loop() {

/*  if (!initialized) {
    int initLoopCount = 0;
    while (Serial.available() == 0) {
      digitalWrite(yellowPin, initLoopCount % 2);
      initLoopCount = 1 - initLoopCount;
      delay(200);
    }

    initialize();
    delay(10);
  }
*/

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
}

void initialize() {

//  firstOfOuterMode = true;
  
  String data = Serial.readStringUntil('\n');
  Serial.println("INITIALIZED");

  if (mode == SOLID_MODE) {
    solid();
  }
  else if (mode == OFF_MODE) {
    off();
  }
  initialized = true;
  digitalWrite(yellowPin, LOW);
}

boolean checkCommand() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    if (data.equals("SOLID")) {
      solid();
      mode = SOLID_MODE;
      autoCycle = false;
      return true;
    }
    else if (data.equals("INIT")) {
      // Indicate that the  command was sent, then reset in the current state
      off();
      delay(500);
      setStripColor(getColorRGB(255, 255, 0));
      delay(500);
      off();
      delay(500);
      setStripColor(getColorRGB(255, 255, 0));
      delay(500);
      off();
      delay(500);
      setStripColor(getColorRGB(255, 255, 0));
      delay(500);
      off();
      delay(500);

      if (mode == SOLID_MODE) {
        solid(currentColor);
      }

      return true;
    }
    else if (data.equals("OFF")) {
      off();
      mode = OFF_MODE;
      autoCycle = false;
      return true;
    }
    else if (data.equals("DECREASE_BRIGHTNESS")) {
      brightness -= 25;
      if (brightness < 1) {
        brightness = 1;
      }
      setBrightness();
      return false; // don't interrupt the flow
    }
    else if (data.equals("INCREASE_BRIGHTNESS")) {
      brightness += 25;
      if (brightness > 255) {
        brightness = 255;
      }
      setBrightness();
      return false; // don't interrupt the flow
    }
    else if (data.startsWith("BRIGHTNESS")) {
      String brightnessStr = data.substring(data.indexOf(' ') + 1);
      brightness = brightnessStr.toInt();
      setBrightness();
      return false; // don't interrupt the flow
    }
    else if (data.equals("AUTOCYCLE_ON")) {
      autoCycle = true;
      mode = random(0, NUM_MODES);
      off();
      return true;
    }
    else if (data.equals("RAINBOW")) {
      autoCycle = false;
      mode = RAINBOW_MODE;
      rainbow();
      return true;
    }
    else {
      // check for a matching mode
      for (int m = 0; m < NUM_MODES; m++) {
        if (data.equals(modeCommands[m])) {
          mode = m;
          autoCycle = false;
          off();
          return true;
        }
      }

      // check for a matching color
      for (int m = 0; m < NUM_COLORS; m++) {
        if (data.equals(colorNames[m])) {
          solid(colors[m]);
          mode = SOLID_MODE;
          autoCycle = false;
          return true;
        }
      }
    }
  }

  return false;
}

void setBrightness() {
  FastLED.setBrightness(brightness);
  if (mode == SOLID_MODE) {
    // set the color so the brightness takes effect; otherwise it'll just take effect on the next loop
    solid(currentColor);
  }
  else if (mode == RAINBOW_MODE) {
    rainbow(false);
  }
}

int rainbowStart;

void rainbow() {
  rainbow(true);
}

void rainbow(boolean newColor) {
  if (newColor) {
    rainbowStart = random(0, 384);
  }
  int step = 384 / NUM_LEDS;
  int curColor = rainbowStart;
  for (int led = 0; led < NUM_LEDS; led++) {
    setPixelColor(led, getColor(curColor));
    color += step;
    if (color >= 384) {
      color -= 384;
    }
  }
  show();
}

void reset() {
  turnOff();
}

void solid() {
  COLOR color = randomColor();
  currentColor = color;
  setStripColor(color);
}

void solid(COLOR color) {
  currentColor = color;
  setStripColor(color);
}

void off() {
  turnOff();
}

void binaryCount() {

  if (modeLoopNumber == 0) {
    savedColor = randomColor();
  }

  int v = modeLoopNumber;

  for (int i = 0; i < 5; i++) { //goes up to 2 ^ n
    setPixelColor(i, v % 2 == 0 ? OFF : savedColor);
    v = v / 2;
  }

  show();
}

void alternate() {

  if (modeLoopNumber == 0) {
    savedColor = randomColor();
  }

  setPixelColor(0, modeLoopNumber % 2 ? OFF : savedColor);
  setPixelColor(1, modeLoopNumber % 2 ? savedColor : OFF);
  show();
}

void setStripColor(COLOR color) {
  setStripColor(color, 0, NUM_LEDS);
}

void setStripColor(COLOR color, byte start, byte count) {
  for (int i = 0; i < count; i++) {
    setPixelColor(i + start, color);
  }
  show();
}

void turnOff() {
  setStripColor(OFF, 0, NUM_LEDS);
}

void turnOff(byte start, byte count) {
  setStripColor(OFF, start, count);
}

COLOR getColor(int val) {
  byte r, g, b;
  
  switch(val / 128)
  {
    case 0:
      r = 255 - (val % 128) * 2;   //Red down
      g = (val % 128) * 2;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 255 - (val % 128) * 2;  //green down
      b = (val % 128) * 2;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 255 - (val % 128) * 2;  //blue down 
      r = (val % 128) * 2;      //red up
      g = 0;                  //green off
      break; 
  }
  
  return getColorRGB(r, g, b);
}

COLOR randomColor() {
  int r = random(0, 384);
  return getColor(r);
}

COLOR getColorRGB(int r, int g, int b) {

//  r = r * brightness / 255;
//  g = g * brightness / 255;
//  b = b * brightness / 255;
//  return strip.Color(r, g, b);
  
  return COLOR(r, g, b);
}

void setPixelColor(int pixel, COLOR color) {
  ledStrip[pixel] = color;
}

void setPixelColor(int pixel, COLOR color, boolean update) {
  setPixelColor(pixel, color);

  if (update) {
    show();
  }
}

void show() {
//  strip.show();
  FastLED.show();
}


