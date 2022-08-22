#define SOLID_MODE -1
#define OFF_MODE -2
#define RAINBOW_MODE -3
#define COLOR CRGB

#include "FastLED.h"

#define NUM_LEDS 96
#define DATA_PIN 3
#define CLOCK_PIN 13

#define STATUS_LEDS 32

int initSteps = 4;
int ledsPerInit = STATUS_LEDS / initSteps;
boolean initializing = false; // whether we are in an initialization routine

CRGB ledStrip[NUM_LEDS];

CRGB OFF = CRGB(0, 0, 0);

COLOR getColorRGB(int r, int g, int b); // definition for use in colors array
COLOR initColor = getColorRGB(255, 255, 0);

/*
   How to add a mode:

   1. Add a method definition
   2. Add the method to the array
   3. Set the number of times the mode should be repeated (in autocycle mode)
   4. Set the number of iterations that constitute one full cycle of the mode
   5. Set the delay between iterations in #4
   6. Add a new entry into modeCommands
   7. Increment NUM_MODES
   8. Implement the method
   9. Enter a boolean for autocycleSkip
*/

// mode definition
void progressiveRainbow();
void progressiveRainbowSimple();
void progressiveSolid();
void starryNight();
void breathing();
void breathingRainbow();
void breathingSolid();
void marqueeSolid();
void marqueeRainbow();
void zip();
void randomZip();
void partialRainbow();
void isu();
void fireplace();


void (*modes[])() = {
  progressiveRainbow,
  progressiveSolid,
  progressiveRainbow,
  progressiveSolid,
  starryNight,
  breathing,
  breathingRainbow,
  breathingSolid,
  zip,
  randomZip,
  partialRainbow,
  partialRainbow,
  fireplace
  isu
};

// the number of times the mode should be repeated
int modeRepeat[] = {
  1, //progressiveRainbow
  1, //progressiveSolid
  1, //progressiveRainbow fast
  1, //progressiveSolid fast
  1, //starryNight
  10, //breathing
  10, //breathingRainbow
  10, //breathingSolid
  10, // zip
  10, //random zip
  1,
  1,
  1,
  1
};

// the number of iterations for one complete execution of the mode
int modeLoops[] = {
  384, //progressiveRainbow - the progressive modes should be multiples of 384 for best results
  384, //progressiveSolid
  384, //progressiveRainbow fast
  384, //progressiveSolid fast
  500, //starryNight
  2, //breathing - the breathing ones will use two loops - one to "inhale", one to "exhale"
  2, //breathingRainbow
  2, //breathingSolid
  1, //zip
  1, //random zip
  384, //partial rainbow
  384,  //partial rainbow fast
  1200 // fireplace
  3
};

// delay between iterations of the inner loop (i.e., delay between invocations of the mode method)
int modeLoopDelay[] = {
  0, //progressiveRainbow - delays handled within function
  1000, //progressiveSolid
  0, //progressiveRainbow fast
  100, //progressiveSolid fast
  50, //starryNight
  1000, //breathing - the breathing modes manage some of their own delays, so this is only the time "between breaths"
  1000, //breathingRainbow
  1000, //breathingSolid
  1,
  1,
  1000,
  100,
  100,
  0
};

boolean autocycleSkip[] = {
  false,
  false,
  true,
  true,
  false,
  false,
  true,
  true,
  false,
  false,
  true,
  true,
  false,
  true
};

// placeholders to accept mode commands
String modeCommands[] = {
  "MODE_0",
  "MODE_1",
  "MODE_2",
  "MODE_3",
  "MODE_4",
  "MODE_5",
  "MODE_6",
  "MODE_7",
  "MODE_8",
  "MODE_9",
  "MODE_10",
  "MODE_11",
  "MODE_12",
  "MODE_13"
};

int NUM_MODES = 14;

// mode state
int mode = 12;
boolean autoCycle = false;
int modeIterationNumber = 0; // the current count of the outer mode repeat loop
int modeLoopNumber = 0; // the current count of the inner loop for one cycle of a mode
boolean firstLoop = true;
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
int savedInt;
int rainbowStart;

void setup() {

  FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, GRB>(ledStrip, NUM_LEDS);
  FastLED.setBrightness(brightness);

  Serial.begin(9600);

  randomSeed(analogRead(0));
  reset();

  initialize();
}

int value = 0;

void loop() {

  if (initializing) {
    checkCommand();
    delay(100);
    return;
  }

  if (autoCycle) {
    while (true) {
      firstLoop = true;
      for (modeIterationNumber = 0; modeIterationNumber < modeRepeat[mode] && !autocycleSkip[mode]; modeIterationNumber++) {
        for (modeLoopNumber = 0; modeLoopNumber < modeLoops[mode]; modeLoopNumber++) {
          modes[mode]();
          firstLoop = false;
          delay(modeLoopDelay[mode]);

          // check if a command was issued; if so, the command will reset the state and we'll start at the top
          if (checkCommand()) {
            return;
          }
        }
      }
      mode++;
      if (mode == NUM_MODES) {
        mode = 0;
      }
      reset();
    }
  }
  else if (mode >= 0 && mode < NUM_MODES) {
    // a mode was explicitly selected, so just run the inner loop repeatedly
    modeIterationNumber = 0;
    firstLoop = true;
    while (true) {
      for (modeLoopNumber = 0; modeLoopNumber < modeLoops[mode]; modeLoopNumber++) {
        modes[mode]();
        firstLoop = false;
        delay(modeLoopDelay[mode]);
        if (checkCommand()) {
          return;
        }
      }
      modeIterationNumber++;
    }
  }
  else {
    // SOLID, RAINBOW, or OFF were selected, and would have already been set, so do nothing
    delay(10);

    checkCommand(); // we'll always start at the top of the loop so no need for an if here
  }
}

void initialize() {

  //  firstOfOuterMode = true;

  if (mode == SOLID_MODE) {
    solid();
  }
  else if (mode == RAINBOW_MODE) {
    rainbow();
  }
  else if (mode == OFF_MODE) {
    off();
  }
}

int initDelayLength = 1000 / ledsPerInit;

boolean checkCommand() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    if (data.equals("SOLID")) {
      solid();
      mode = SOLID_MODE;
      autoCycle = false;
      return true;
    }
    else if (data.equals("INIT_0")) {
      initializing = true;
      off();
      rainbowStart = random(0, 384); //pick a rainbow start color
      savedInt = 384 / NUM_LEDS; //rainbow color step size
      for (int led = 0; led < ledsPerInit; led++) {
        setPixelColor(led, getColor(rainbowStart), true);
        show();
        delay(initDelayLength);

        rainbowStart += savedInt;
        if (rainbowStart >= 384) {
          rainbowStart -= 384;
        }
      }

      show();
      return true;
    }
    else if (data.equals("INIT_1")) {
      for (int led = ledsPerInit; led < 2 * ledsPerInit; led++) {
        setPixelColor(led, getColor(rainbowStart), true);
        show();
        delay(initDelayLength);

        rainbowStart += savedInt;
        if (rainbowStart >= 384) {
          rainbowStart -= 384;
        }
      }
      show();
      return true;
    }
    else if (data.equals("INIT_2")) {
      for (int led = 2 * ledsPerInit; led < 3 * ledsPerInit; led++) {
        setPixelColor(led, getColor(rainbowStart), true);
        show();
        delay(initDelayLength);

        rainbowStart += savedInt;
        if (rainbowStart >= 384) {
          rainbowStart -= 384;
        }
      }
      show();
      return true;
    }
    else if (data.equals("INIT_FINAL")) {
      // Indicate that the command was sent, then reset back to the previous actual state
      //      off();
      //      delay(500);
      //      setStripColor(initColor);
      //      delay(500);
      //      off();
      //      delay(500);
      //      setStripColor(initColor);
      //      delay(500);
      //      off();
      //      delay(500);
      //      setStripColor(initColor);
      //      delay(500);
      //      off();
      //      delay(500);

      int delayLength = 2000 / (NUM_LEDS - (3 * ledsPerInit));

      for (int led = 3 * ledsPerInit; led < NUM_LEDS; led++) {
        setPixelColor(led, getColor(rainbowStart));
        show();
        delay(delayLength);

        rainbowStart += savedInt;
        if (rainbowStart >= 384) {
          rainbowStart -= 384;
        }
      }

      if (mode == SOLID_MODE) {
        solid(currentColor);
      }
      else if (mode == RAINBOW_MODE) {
        rainbow(false); //reuse the rainbow we just generated during init
      }

      initializing = false;

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
    else if (data.startsWith("SET_COLOR")) {
      if (mode == SOLID_MODE) {
        int space1 = data.indexOf(' ');
        int space2 = data.lastIndexOf(' ');
        String colorStr = data.substring(space1 + 1, space2);
        colorStr.toLowerCase();
        char c = colorStr.charAt(0);
        int value = data.substring(space2 + 1).toInt();
        COLOR curColor = ledStrip[0];
        byte r = c == 'r' ? value : getColorComponent(curColor, 'r');
        byte g = c == 'g' ? value : getColorComponent(curColor, 'g');
        byte b = c == 'b' ? value : getColorComponent(curColor, 'b');
        setStripColor(getColorRGB(r, g, b));
      }
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

void progressiveRainbowSimple() {
  if (firstLoop) {
    savedInt = random(0, 384); // rainbow starting color
  }
  int startLed = modeLoopNumber % NUM_LEDS;
  int step = 384 / NUM_LEDS;
  int curColor = savedInt;

  for (int cnt = 0; cnt < NUM_LEDS; cnt++) {
    int led = cnt + startLed;
    if (led >= NUM_LEDS) {
      led -= NUM_LEDS;
    }
    setPixelColor(led, getColor(curColor));
    curColor += step;
    if (curColor >= 384) {
      curColor -= 384;
    }
  }

  show();
}

void progressiveRainbow() {
  if (firstLoop) {
    savedInt = random(0, 384); // rainbow starting color
  }
  int startLed = modeLoopNumber % NUM_LEDS;
  int step = 384 / NUM_LEDS;
  int stepDelay = (mode == 0 ? 1000 : 100) / step; // the number of ms for an LED to change by 1
  int curColor = savedInt;

  // spread the step size out over the number of ms above
  for (int shadeOffset = 0; shadeOffset < step; shadeOffset++) {
    for (int cnt = 0; cnt < NUM_LEDS; cnt++) {
      int led = cnt + startLed;
      if (led >= NUM_LEDS) {
        led -= NUM_LEDS;
      }
      setPixelColor(led, getColor(curColor - shadeOffset));
      curColor += step;
      if (curColor >= 384) {
        curColor -= 384;
      }
    }
    show();
    delay(stepDelay);
  }

  //  show();
}

void partialRainbow() {
  if (firstLoop) {
    savedInt = random(0, 384); // rainbow starting color
  }
  else {
    savedInt--;
  }

  for (int led = 0; led < NUM_LEDS; led++) {
    setPixelColor(led, getColor(savedInt + led));
  }
  show();

}

void progressiveSolid() {
  if (firstLoop) {
    savedInt = random(0, 384); // starting color
  }
  COLOR color = getColor(savedInt);
  setStripColor(color);
  savedInt += 1; // step size for next color
  if (savedInt >= 384) {
    savedInt -= 384;
  }
}

void breathingSolid() {
  if (firstLoop) { // pick one color and repeat it until the mode changes
    savedColor = randomColor();
  }

  // The idea here is to do integer math to keep it simple while adjusting for drastic jumps in brightness. We'll start with a target step
  // count (granularity of changes)  to be 40, which is arbitrary. Then we'll recalculate to smooth it out.
  // For example, if brightness is 50, then step size will be 1 and # steps will be 40. So, we'd go from 50 down to 10 brightness
  // and then turn it off, and it would look like a sudden jump from 10 brightness to 0. So, we'll go gracefully down to 0.

  int duration = 2000; //length of time a breath should last (ms)
  int steps = min(brightness, 40); //preliminary step count value to get us started
  int stepSize = brightness / steps; //this will likely be a value of 1 or 2 (typical brightness will be < 80)
  steps = brightness / stepSize; //find the true number of steps. if brightness was 50, then stepSize == 1 so steps = 50. If brightness is 90, then stepSize == 2 so steps = 45.
  int stepDelay = duration / steps;


  if (modeLoopNumber == 0) {
    // inhale - go from off up to set brightness
    for (int step = 0; step < steps; step++) {
      FastLED.setBrightness(step * stepSize);
      setStripColor(savedColor);
      FastLED.setBrightness(brightness); //this is just in case the mode gets reset; it keeps the old brightness
      delay(stepDelay);
    }
    setStripColor(savedColor);
  }
  else {
    //exhale - go from go from set brightness to off
    for (int step = 0; step < steps; step++) {
      FastLED.setBrightness(brightness - (step * stepSize));
      setStripColor(savedColor);
      FastLED.setBrightness(brightness); //this is just in case the mode gets reset; it keeps the old brightness
      delay(stepDelay);
    }
    off();
  }
  //end the loop with the strip solid and brightness fully reset
}

void breathing() {
  if (modeLoopNumber == 0) { // pick a new color each breath
    savedColor = randomColor();
  }

  // see comments in breathingSolid

  int duration = 2000;
  int steps = min(brightness, 40);
  int stepSize = brightness / steps;
  steps = brightness / stepSize;
  int stepDelay = duration / steps;

  if (modeLoopNumber == 0) {
    // inhale - go from off up to set brightness
    for (int step = 0; step < steps; step++) {
      FastLED.setBrightness(step * stepSize);
      setStripColor(savedColor);
      FastLED.setBrightness(brightness); //this is just in case the mode gets reset; it keeps the old brightness
      delay(stepDelay);
    }
    setStripColor(savedColor);
  }
  else {
    //exhale - go from go from set brightness to off
    for (int step = 0; step < steps; step++) {
      FastLED.setBrightness(brightness - (step * stepSize));
      setStripColor(savedColor);
      FastLED.setBrightness(brightness); //this is just in case the mode gets reset; it keeps the old brightness
      delay(stepDelay);
    }
    off();
  }
  //end the loop with the strip solid and brightness fully reset
}

void breathingRainbow() {
  if (modeLoopNumber == 0) { // pick a new rainbow each breath
    rainbow(true, false);
  }

  // see comments in breathingSolid

  int duration = 2000;
  int steps = min(brightness, 40);
  int stepSize = brightness / steps;
  steps = brightness / stepSize;
  int stepDelay = duration / steps;

  if (modeLoopNumber == 0) {
    // inhale - go from off up to set brightness
    for (int step = 0; step < steps; step++) {
      FastLED.setBrightness(step * stepSize);
      rainbow(false);
      FastLED.setBrightness(brightness); //this is just in case the mode gets reset to keep the old brightness
      delay(stepDelay);
    }
    rainbow(false);
  }
  else {
    //exhale - go from go from set brightness to off
    for (int step = 0; step < steps; step++) {
      FastLED.setBrightness(brightness - (step * stepSize));
      rainbow(false);
      FastLED.setBrightness(brightness); //this is just in case the mode gets reset to keep the old brightness
      delay(stepDelay);
    }
    off();
  }
}

void starryNight() {
  if (modeLoopNumber == 0) {
    savedColor = randomColor(); // "background" color
    setStripColor(savedColor);
  }

  // we'll pick a random LED; if it's the background color, set it to a random color; otherwise, reset its color
  int led = random(0, NUM_LEDS);
  if (ledStrip[led] == savedColor) {
    COLOR c = randomColor();
    setPixelColor(led, randomColor(), true);
  }
  else {
    setPixelColor(led, savedColor, true);
  }
}

void zip() {
  off();
  COLOR color = randomColor();
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixelColor(i, color, true);
    delay(2);
    setPixelColor(i, OFF);
  }
}

void randomZip() {
  COLOR color = randomColor();
  off();
  for (int i = 0; i < NUM_LEDS; i++) {
    int pixel = random(0, NUM_LEDS);
    setPixelColor(pixel, color, true);
    delay(2);
    setPixelColor(pixel, OFF);
  }
}

void rainbow() {
  rainbow(true);
}

void rainbow(boolean newColor) {
  rainbow(newColor, true);
}

void fireplace() {
  if (firstLoop) {
    for (int pixel = 0; pixel < NUM_LEDS; pixel++) {
      COLOR color = getRandomFireColor();
      setPixelColor(pixel, color);
    }
  }
  else {
    for (int i = 0; i < 10; i++) {
      int pixel = random(0, NUM_LEDS);
      COLOR color = getRandomFireColor();
      setPixelColor(pixel, color);
    }
  }

  show();
}

void isu() {

  COLOR cardinal = getColorRGB(255, 0, 0);
  COLOR gold = getColorRGB(150, 150, 0);
  COLOR c;

  while (true) {
    for (int i = 0; i < 4; i++) {
      if (i < 8) {
        if (i % 2 == 0) {
          c = cardinal;
        }
        else {
          c = gold;
        }

        for (int led = 0; led < NUM_LEDS; led++) {
          setPixelColor(led, c, true);
          delay(800 / 96);
        }
      }
    }

    for (int i = 0; i < 8; i++) {
      for (int led = 0; led < NUM_LEDS; led++) {
        setPixelColor(led, ((i + led) % 2 == 0) ? cardinal : gold);
      }
      show();
      delay(500);
    }
  }
}

void rainbow(boolean newColor, boolean update) {
  if (newColor) {
    rainbowStart = random(0, 384);
  }
  int step = 384 / NUM_LEDS;
  int curColor = rainbowStart;
  for (int led = 0; led < NUM_LEDS; led++) {
    setPixelColor(led, getColor(curColor));
    curColor += step;
    if (curColor >= 384) {
      curColor -= 384;
    }
  }
  if (update) {
    show();
  }
}

COLOR getRandomFireColor() {
  int green = random(5, 65);
  int scale = random(10, 255);
  COLOR color = COLOR(255, green, 0) % scale;
  return color;
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

  while (val >= 384) {
    val -= 384;
  }
  while (val < 0) {
    val += 384;
  }

  byte r, g, b;

  switch (val / 128)
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

byte getColorComponent(int val, char color) {
  byte r, g, b;

  switch (val / 128)
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

  if (color == 'r') {
    return r;
  }
  else if (color == 'g') {
    return g;
  }
  else {
    return b;
  }
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
