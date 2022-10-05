/*
  The circuit:
  1 : GND
  2 : 5V
  3 : Backlight adjust
  4 : RS = pin 8
  5 : GND
  6 : E = pin 9
  7 : D0 = NC
  8 : D1 = NC
  9 : D2 = NC
  10: D3 = NC
  11: D4 = pin 4
  12: D5 = pin 5
  13: D6 = pin 6
  13: D7 = pin 7
  13: LED A 5V
  13: LED K GND
*/

#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
const int pd = A1;
const int sw = A5;

// Global variable
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
bool displayEnable = true;
int next_wakeup_d = 0;
int next_wakeup_h = 0;
int next_wakeup_m = 0;
int next_wakeup_s = 0;


void powerOffDisplay() {
  digitalWrite(pd, HIGH);
  digitalWrite(rs, LOW);
  digitalWrite(en, LOW);
  digitalWrite(d4, LOW);
  digitalWrite(d5, LOW);
  digitalWrite(d6, LOW);
  digitalWrite(d7, LOW);
}

void powerOnDisplay() {
  digitalWrite(pd, LOW); // Must be done BEFORE init display
  lcd.begin(16, 2);
  lcd.print("Next charging:");
}

void updateWakeupTimer(const long cur_time) {
  static long last_inc_time = 0;
  if (cur_time - last_inc_time > 1000) {
    last_inc_time = cur_time;
  
    if (!next_wakeup_d && !next_wakeup_h && !next_wakeup_m && !next_wakeup_s) {
      // Timer is at 0;
    } else { // Remove 1 second
      if (next_wakeup_s > 0 ) {
        next_wakeup_s--;
      } else {
        next_wakeup_s=59;
        if (next_wakeup_m > 0 ) {
          next_wakeup_m--;
        } else {
          next_wakeup_m=59;
          if (next_wakeup_h > 0 ) {
            next_wakeup_h--;
          } else {
            next_wakeup_h=23;
            if (next_wakeup_d > 0 ) {
              next_wakeup_d--;
            }
          }
        }
      }
    }
  }
}

void printWakeupTimer() {
  lcd.setCursor(0, 1);
  lcd.print(next_wakeup_d);
  lcd.print("d ");
  if (next_wakeup_h < 10) {
    lcd.print(" ");
  }
  lcd.print(next_wakeup_h);
  lcd.print("h ");
  if (next_wakeup_m < 10) {
    lcd.print(" ");
  }
  lcd.print(next_wakeup_m);
  lcd.print("m ");
  if (next_wakeup_s < 10) {
    lcd.print(" ");
  }
  lcd.print(next_wakeup_s);
  lcd.print("s ");
}

void readUserSwitches(const long cur_time) {
  static long last_read_time = 0;
  if (cur_time - last_read_time > 500) {
    if (displayEnable) {
      int val = analogRead(A0);
      Serial.println(val);
      if (val >= 0 && val<=50) {
        // R: +1H
        if (next_wakeup_h < 23) {
          next_wakeup_h += 1;
        }
        last_read_time = cur_time;
      } else if(val >= 50 && val <= 200) {
        // U: +24H
        next_wakeup_d +=1;
        if (next_wakeup_d > 9) {
          next_wakeup_d = 9;
          next_wakeup_h = 23;
          next_wakeup_m = 59;
          next_wakeup_s = 59;
        }
        last_read_time = cur_time;
      } else if(val >= 200 && val <= 400) {
        // D: -24H
        if (next_wakeup_d > 0) {
          next_wakeup_d -= 1;
        } else {
          next_wakeup_d = 0;
          next_wakeup_h = 0;
          next_wakeup_m = 0;
          next_wakeup_s = 0;
        }
        last_read_time = cur_time;
      } else if(val >= 400 && val <= 600) {
        // L: Reset timer
        next_wakeup_d = 0;
        next_wakeup_h = 0;
        next_wakeup_m = 0;
        next_wakeup_s = 0;
        last_read_time = cur_time;
      } else if(val >= 600 && val <= 800) {
        // S: 
        next_wakeup_s = 10;
        last_read_time = cur_time;
      }
    }

    if (digitalRead(sw) == LOW) {
      if (displayEnable) {
        Serial.println("User power OFF screen");
        powerOffDisplay();
        displayEnable = false;
      } else {
        Serial.println("User power ON screen");
        powerOnDisplay();
        displayEnable = true;
      }
      last_read_time = cur_time;
    }
  }
}

void setup() {
  pinMode(pd, OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  powerOnDisplay();
}

void loop() {
  long cur_time = millis();

  updateWakeupTimer(cur_time);

  if (displayEnable) {
    Serial.println("Update screen");
    printWakeupTimer(); 
  }

  readUserSwitches(cur_time);

  delay(50);
}
