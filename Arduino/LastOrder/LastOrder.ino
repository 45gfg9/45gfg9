#include <EEPROM.h>
#include <RTClib.h>
#include <Seg7List.h>

#define R_PIN 10
#define B_PIN 11
#define L_PIN A5

#define T_ADDR 0x00
#define L_ADDR 0x10

DS1302 RTC(8, 6, 7);
Seg7 S7(2, 3, 4);

const DateTime UPLOAD(__DATE__, __TIME__);
DateTime target;
DateTime currdt;

int L_base, i;
int lefts;
bool state, ts;
byte ms = 0, ndx;
char str[32];

void setup() {
  delay(2000);
  Serial.begin(9600);

  RTC.begin();

  if (RTC.now() < UPLOAD) {
    RTC.adjust(UPLOAD);
  }

  EEPROM.get(T_ADDR, target);
  currdt = RTC.now();

  Serial.println(dt2string(target) + " T");
  Serial.println(dt2string(UPLOAD) + " C");
  Serial.println(dt2string(currdt) + " N");
  Serial.setTimeout(0);

  EEPROM.get(L_ADDR, L_base);

  pinMode(B_PIN, INPUT_PULLUP);

  update();
}

void loop() {
  if (state != ts) {
    state = ts;

    state ? S7.show(lefts) : S7.show();
  }

  ts = getState();
  currdt = RTC.now();

  if (!((10 * (target - currdt).totalseconds() + ms) % 600)) {
    update();

    Serial.println(dt2string(currdt) + " N");
  }

  ++ms %= 10;
  delay(100);
}

void readStr() {
  char ch;
  ndx = 0;

  while (Serial.available()) {
    ch = Serial.read();

    if (ch != '\n' && ch != '\r') {
      str[ndx++] = ch;
    }
    else {
      str[ndx] = '\0';
      return;
    }
  }
}

void serialEvent() {
  while (Serial.available()) {
    switch (Serial.read()) {
      case 'R':
        RTC.begin();
        delay(100);
        pinMode(R_PIN, OUTPUT);
        digitalWrite(R_PIN, LOW);
        break;

      case 'C':
        Serial.println(dt2string(UPLOAD) + " C");
        break;

      case 'T':
        readStr();
        if (strlen(str) == 17) {
          target = DateTime(atoi(str), atoi(str + 3), atoi(str + 6), atoi(str + 9), atoi(str + 12), atoi(str + 15));
          EEPROM.put(T_ADDR, target);
          update();
        }
        Serial.println(dt2string(target) + " T");
        break;

      case 'S':
        snprintf_P(str, 32, PSTR("S:%u B:%u L:%u"), state, !digitalRead(B_PIN), analogRead(L_PIN));
        Serial.println(str);
        break;

      case 'N':
        readStr();
        if (strlen(str) == 17) {
          RTC.adjust(DateTime(atoi(str), atoi(str + 3), atoi(str + 6), atoi(str + 9), atoi(str + 12), atoi(str + 15)));
          currdt = RTC.now();
          update();
        }
        Serial.println(dt2string(currdt) + " N");
        break;

      case 'D':
        i = Serial.parseInt();
        readStr();
        S7.show(str);
        i += millis();
        while (millis() < i) {
          serialEvent();
        }
        update();
        break;

      case 'L':
        i = Serial.parseInt();
        if (0 <= i - 50 && i + 50 <= 1023) {
          L_base = i;
          EEPROM.put(L_ADDR, L_base);
        }
        Serial.println(L_base);
        break;
    }
  }
}

void update() {
  // if (currdt < target) {
  //   lefts = ((target - currdt).totalseconds() + .1 * ms) / 86400;
  // }
  // else {
  //   lefts = 0;
  // }
  lefts = currdt < target ? (10 * (target - currdt).totalseconds() + ms) / 600 : 0;
  state = false;
}

bool getState() {
  return (analogRead(L_PIN) < L_base + (state ? 50 : -50) || !digitalRead(B_PIN));
}

String dt2string(DateTime dt) {
  snprintf_P(str, 20, PSTR("%04u/%02u/%02u %02u:%02u:%02u"),
             dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
  return str;
}
