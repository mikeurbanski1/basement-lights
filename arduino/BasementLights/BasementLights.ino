
int bluePin = 2;
int greenPin = 3;
int yellowPin = 4;

int blueState = LOW;
int greenState = LOW;

bool initialized = false;

void setup() {
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  Serial.begin(9600);
}

int value = 0;

void loop() {

  if (initialized) {
    delay(100);
    return;
  }

  int initLoopCount = 0;
  while (Serial.available() == 0) {
    digitalWrite(yellowPin, initLoopCount % 2);
    initLoopCount = 1 - initLoopCount;
    delay(200);
  }

  initialized = true;
  digitalWrite(yellowPin, LOW);
  digitalWrite(bluePin, HIGH);

  delay(1);
//
//  if (initializing) {
//    digitalWrite(yellowPin, HIGH);
//  }
//  else {
//    digitalWrite(yellowPin, LOW);
//  }
//
//  if (Serial.available() > 0) {
//    String data = Serial.readStringUntil('\n');
//    if (initializing) {
//      Serial.println("Hello");
//      initializing = false;
//      delay(10);
//    }
//    else {
//      Serial.println("'" + data + "'");
//      if (data.equals("BLUE_ON")) {
//        if (blueState == LOW) {
//          digitalWrite(bluePin, HIGH);
//          Serial.println("SUCCESS");
//          blueState = HIGH;
//        }
//        else {
//          Serial.println("NO_CHANGE");
//        }
//      }
//      else if (data.equals("GREEN_ON")) {
//        if (greenState == LOW) {
//          digitalWrite(greenPin, HIGH);
//          Serial.println("SUCCESS");
//          greenState = HIGH;
//        }
//        else {
//          Serial.println("NO_CHANGE");
//        }
//      }
//      if (data.equals("BLUE_OFF")) {
//        if (blueState == HIGH) {
//          digitalWrite(bluePin, LOW);
//          Serial.println("SUCCESS");
//          blueState = LOW;
//        }
//        else {
//          Serial.println("NO_CHANGE");
//        }
//      }
//      else if (data.equals("GREEN_OFF")) {
//        if (greenState == HIGH) {
//          digitalWrite(greenPin, LOW);
//          Serial.println("SUCCESS");
//          greenState = LOW;
//        }
//        else {
//          Serial.println("NO_CHANGE");
//        }
//      }
//      else {
//        Serial.println("Unknown command: '" + data + "'");
//      }
//      delay(10);
//    }
//  }
//  else if (initializing) {
//    delay(100);
//    digitalWrite(yellowPin, LOW);
//    delay(100);
//    digitalWrite(yellowPin, HIGH);
//    delay(100);
//    digitalWrite(yellowPin, LOW);
//    delay(100);
//    digitalWrite(yellowPin, HIGH);
//  }

}
