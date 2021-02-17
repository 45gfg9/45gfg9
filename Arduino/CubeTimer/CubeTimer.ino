#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define buttonPin 2
#define LEDPin 12
#define actualWritePin 13
#define readRecordPin A0
#define resetRecordPin A1
#define resetAllRecordPin A2
#define dumpToSerialPin A3

#define triggerAnalogValue 1020

LiquidCrystal_I2C LCD(0x3F, 16, 2);

bool actualWrite = false;

struct record {
  word level, hr, min, sec, millisec;
  bool operator<(const record& rec) {
    return (3600000 * hr + 60000 * min + 1000 * sec + millisec) < (3600000 * rec.hr + 60000 * rec.min + 1000 * rec.sec + rec.millisec);
  }
  bool operator>(const record& rec) {
    return (3600000 * hr + 60000 * min + 1000 * sec + millisec) > (3600000 * rec.hr + 60000 * rec.min + 1000 * rec.sec + rec.millisec);
  }
  bool operator<=(const record& rec) {
    return (3600000 * hr + 60000 * min + 1000 * sec + millisec) <= (3600000 * rec.hr + 60000 * rec.min + 1000 * rec.sec + rec.millisec);
  }
  bool operator>=(const record& rec) {
    return (3600000 * hr + 60000 * min + 1000 * sec + millisec) >= (3600000 * rec.hr + 60000 * rec.min + 1000 * rec.sec + rec.millisec);
  }
}
cube, bestRec;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(actualWritePin, INPUT_PULLUP);
  pinMode(LEDPin, OUTPUT);

  LCD.init();
  LCD.backlight();

  LCD.home();

  LCD.print("Waiting...");

  for (;;) {
    if (digitalRead(buttonPin) == LOW) {
      LCD.clear();
      LCD.print("Timer");
      delay(500);
      cubeTimer();
      return;
    }
    else if (analogRead(readRecordPin) > triggerAnalogValue) {
      LCD.clear();
      LCD.print("Read Record");
      delay(500);
      readRec();
      return;
    }
    else if (analogRead(resetRecordPin) > triggerAnalogValue) {
      LCD.clear();
      LCD.print("Reset Record");
      delay(500);
      resetRec();
      return;
    }
    else if (analogRead(resetAllRecordPin) > triggerAnalogValue) {
      LCD.clear();
      LCD.print("Reset All Record");
      LCD.setCursor(0, 1);
      LCD.print("Are you sure?");
      delay(500);
      while (digitalRead(buttonPin) == HIGH);
      resetAllRec();
      return;
    }
    else if (analogRead(dumpToSerialPin) > triggerAnalogValue) {
      LCD.clear();
      LCD.print("Dump To Serial");
      dumpToSerial();
      LCD.setCursor(0, 1);
      LCD.print("Done!");
      return;
    }
  }
}

void cubeTimer() {
  cube.level = chooseLevel();

  LCD.clear();
  LCD.setCursor(2, 1);
  LCD.print("Level ");
  LCD.print(cube.level);
  LCD.print(" cube");
  LCD.home();
  LCD.print("Prepared! ");

  if (digitalRead(actualWritePin) == LOW) {
    actualWrite = true;
    digitalWrite(LEDPin, HIGH);
  }

  delay(200);

  while (digitalRead(buttonPin) == HIGH);
  LCD.print("Ready! ");
  delay(10);
  while (digitalRead(buttonPin) == LOW);

  unsigned long sms = millis(), t = 0;

  unsigned int ms, s, m, h, pm = -1;
  delay(10);
  LCD.home();
  LCD.print("        (H : M) ");
  while (digitalRead(buttonPin) == HIGH) {
    t = millis() - sms;
    ms = t % 1000;
    s = t / 1000 % 60;
    m = t / 60000 % 60;
    h = t / 3600000;
    if (pm + 1 == m) {
      LCD.home();

      if (h < 10)LCD.print("0");
      LCD.print(h);
      LCD.print(":");

      if (m < 10)LCD.print("0");
      LCD.print(m);
      pm++;
    }
  }

  if (t < 1000) {
    actualWrite = false;
    digitalWrite(LEDPin, LOW);
  }

  if ((h > 23) || (m > 59) || (s > 59) || (ms > 999)) {
    LCD.clear();
    LCD.print("Runtime Error");
    LCD.setCursor(0, 1);
    LCD.print("Reset Arduino");
    errblink(5);
    return;
  }

  cube.hr = h;
  cube.min = m;
  cube.sec = s;
  cube.millisec = ms;

  EEPROM.get(16 * (cube.level - 2), bestRec);

  if (cube < bestRec) {
    LCD.clear();
    LCD.print("Congratulations!");
    LCD.setCursor(0, 1);
    LCD.print("New Record!");

    delay(1000);
    if (actualWrite) {
      LCD.clear();
      LCD.print("Level ");
      LCD.print(cube.level);
      LCD.print(" cube");

      EEPROM.put(16 * (cube.level - 2), cube);
      LCD.setCursor(0, 1);
      LCD.print("Data Written!");

      delay(1000);
    }
    delay(500);
  }

  LCD.clear();
  LCD.print("T: ");
  LCD.print(rec2str(cube));

  LCD.setCursor(0, 1);
  LCD.print("B: ");
  LCD.print(rec2str(bestRec));
}

String rec2str(const record& rec) {
  String timeStr = "";

  if (rec.hr < 10)timeStr.concat("0");
  timeStr.concat(rec.hr);
  timeStr.concat(":");

  if (rec.min < 10)timeStr.concat("0");
  timeStr.concat(rec.min);
  timeStr.concat(":");

  if (rec.sec < 10)timeStr.concat("0");
  timeStr.concat(rec.sec);
  timeStr.concat(":");

  if (rec.millisec < 100)timeStr.concat("0");
  if (rec.millisec < 10)timeStr.concat("0");
  timeStr.concat(rec.millisec);

  return timeStr;
}

void resetRec() {
  bestRec.level = chooseLevel();

  LCD.clear();
  LCD.print("Reset Level ");
  LCD.print(bestRec.level);
  LCD.setCursor(0, 1);
  LCD.print("Are you sure?");
  delay(500);
  while (digitalRead(buttonPin) == HIGH);

  bestRec.hr = 23;
  bestRec.min = 59;
  bestRec.sec = 59;
  bestRec.millisec = 999;

  EEPROM.put(16 * (bestRec.level - 2), bestRec);

  LCD.clear();
  LCD.print("Level ");
  LCD.print(bestRec.level);
  LCD.print(" record");
  LCD.setCursor(0, 1);
  LCD.print("Resetted!");
}

void readRec() {
  bestRec.level = chooseLevel();

  EEPROM.get(16 * (bestRec.level - 2), bestRec);

  LCD.clear();
  LCD.print("Level ");
  LCD.print(bestRec.level);
  LCD.print(" record:");
  LCD.setCursor(0, 1);
  LCD.print(rec2str(bestRec));
}

int chooseLevel() {
  String str;
  int choice;
  unsigned long prevMs, currMs;
  prevMs = currMs = millis();
  LCD.clear();
  LCD.setCursor(2, 0);
  LCD.print("Choose Level");

  for (int i = 0;; i++) {
    str = "2 3 4 5 6 7 8 9";
    i %= 8;
    choice = i + 2;
    str[2 * i] = 0xFF;

    LCD.setCursor(0, 1);
    LCD.print(str);

    while (currMs - prevMs < 500) {
      currMs = millis();

      if (digitalRead(buttonPin) == LOW) {
        return choice;
      }
    }
    prevMs = currMs;
  }
}

void resetAllRec() {
  LCD.clear();
  LCD.print("Writing...");

  bestRec.hr = 23;
  bestRec.min = 59;
  bestRec.sec = 59;
  bestRec.millisec = 999;

  for (bestRec.level = 2; bestRec.level <= 9; bestRec.level++) {
    EEPROM.put(16 * (bestRec.level - 2), bestRec);
  }

  LCD.setCursor(0, 1);
  LCD.print("All Reset!");
}

void dumpToSerial() {
  for (bestRec.level = 2; bestRec.level <= 9; bestRec.level++) {
    EEPROM.get(16 * (bestRec.level - 2), bestRec);
    Serial.print("Level ");
    Serial.print(bestRec.level);
    Serial.println(" Record: \n" + rec2str(bestRec) + "\n");
  }
}

void errblink(int repeat) {
  while (repeat--) {
    digitalWrite(LEDPin, HIGH);
    delay(100);
    digitalWrite(LEDPin, LOW);
    delay(100);
  }
}

void loop() {}
