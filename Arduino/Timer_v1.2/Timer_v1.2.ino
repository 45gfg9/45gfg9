/*******************
  Author: 45gfg9

  Timer v1.2
*******************/

/********************Seg7List********************/
#include <Seg7List.h>

#define SDI 4
#define LCK 3
#define CLK 2

Seg7 S7(SDI, LCK, CLK);

#undef SDI
#undef LCK
#undef CLK

/*********************Keypad*********************/
#include <Keypad.h>

#define rows 4
#define cols 4

const char keys[rows][cols] = {
  { '1', '2', '3', '+' },
  { '4', '5', '6', '-' },
  { '7', '8', '9', 'P' },
  { 'X', '0', 'S', 'S' },
};

const byte rowPins[rows] = { 8, 7, 6, 5 };
const byte colPins[cols] = { 9, 10, 11, 12 };

Keypad buttonPad(makeKeymap(keys), rowPins, colPins, rows, cols);

#undef rows
#undef cols

/**********************Main**********************/
const int LEDpin = 13;

unsigned long pastMs, previousMs = millis(), currentMs = millis();

void setup() {
  pinMode(LEDpin, OUTPUT);
  mode();
}

void loop() {
  digitalWrite(LEDpin, HIGH);
  delay(1000);
  digitalWrite(LEDpin, LOW);
  delay(1000);
}

/********************Function********************/
void mode() {
  for (;;) {
    if (getButton() == '+') {
      S7.show(0);
      countup();
      return;
    }

    if (getButton() == '-') {
      S7.show("-");
      countdown();
      return;
    }
  }
}

void countup() {
  double t = 0;
  char ch;

  while (getButton() != 'S');

  // TODO
  // Auto adjust decimal places
  for (;;) {
    checkPause();
    currentMs = millis();
    if (currentMs - previousMs >= 100) {
      previousMs = currentMs;
      S7.show(t);
      t += 0.1;
    }
  }
}

void countdown() {
  double t = timeset();

  while (t >= 0) {
    checkPause();

    currentMs = millis();

    if (currentMs - previousMs >= 1000) {
      previousMs = currentMs;
      S7.show(--t);
    }
  }

  S7.show("-End");
}

int timeset() {
  char ch;
  String str = "";

  while (str.length() == 0 | ch != 'S') {
    ch = getButton();
    if (isDigit(ch)) {
      str = str + ch;
      S7.show(str);
    }
    if (ch == 'X' | str.length() > 4) {
      str = str.substring(0, str.length() - 1);
      S7.show(str);
    }
  }

  return str.toInt();
}

void checkPause() {
  pastMs = currentMs - previousMs;
  if (getButton() == 'P') {
    while (getButton() != 'S') {
      currentMs = millis();
      previousMs = currentMs - pastMs;
    }
  }
}

inline char getButton() {
  return buttonPad.getKey();
}
