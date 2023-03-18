#include <Keypad.h>

const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
bool foundCode = false;
int button_end = A2;
int button_servo = A1;

int latchPin = 19;
int clockPin = 18;
int dataPin = 12;

byte leds = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 500;

bool servoZeroSent = false;
bool winGameSent = false;
bool servoNineSent = false;
bool goodCodeSent = false;
bool wrongCodeSent = false;

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[ROW_NUM] = { 9, 8, 7, 6 };
byte pin_column[COLUMN_NUM] = { 5, 4, 3, 2 };

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

void updateShiftRegister() {
  digitalWrite(latchPin, LOW);
  for (int i = 0; i < 8; i++) {
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, (leds >> i) & 1);
    digitalWrite(clockPin, HIGH);
  }
  digitalWrite(latchPin, HIGH);
}

void setup() {
  Serial.begin(9600);

  pinMode(button_end, INPUT_PULLUP);
  pinMode(button_servo, INPUT_PULLUP);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}

void loop() {
  int buttonStateEnd = digitalRead(button_end);
  int buttonStateServo = digitalRead(button_servo);
  static int count = 0;
  char code[5];
  char key = keypad.getKey();
  unsigned long currentMillis = millis();

  if (foundCode) {
    if (!servoZeroSent) {
      delay(2000);
      Serial.write("servoZero", 9);
      servoZeroSent = true;
    }
    if (buttonStateEnd == HIGH && !winGameSent) {
      Serial.write("winGame", 7);
      winGameSent = true;
    }
    if (buttonStateServo == LOW && winGameSent) {
      foundCode = false;
      servoNineSent = false;
    }
  } else {
    if (!servoNineSent) {
      Serial.write("servoNine", 9);
      servoNineSent = true;
    }
    if (key) {
      wrongCodeSent = false;
      code[count] = key;
      count++;
      if (count == 4) {
        code[4] = '\0';
        if (strcmp(code, "2473") == 0) {
          if (!goodCodeSent) {
            Serial.write("goodCode", 8);
            goodCodeSent = true;
          }
          foundCode = true;
        } else {
          if (!wrongCodeSent) {
            Serial.write("wrongCode", 9);
            wrongCodeSent = true;
          }
          count = 0;
        }
      }
    }
  }

  if (winGameSent) {
    leds = 255;  // Set all LEDs on
    updateShiftRegister();
  } else {
    if (currentMillis - previousMillis >= interval) {
      byte ledByte = random(256);
      leds = ledByte;
      updateShiftRegister();
      previousMillis = currentMillis;
    }
    if (currentMillis - previousMillis >= interval / 2) {
      leds = 0;
      updateShiftRegister();
    }
  }
}
