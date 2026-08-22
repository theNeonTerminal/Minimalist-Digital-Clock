#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Bonezegei_DS3231.h>

const int width = 128, height = 64;

Bonezegei_DS3231 rtc(0x68);
Adafruit_SH1106G display(width, height, &Wire, -1);

const int touch = 23, bLeft = 5, bRight = 19, bTop = 18, bBottom = 4;
bool pomodoroStart = false, stopWatchStart = false;
const int second = 1000, minute = 60;

unsigned long start = 0, timer = 0;

int state = 1;

// 1: Main Clock
// 2: Stopwatch
// 3: 25-5 Pomodoro Timer
// 4: Timer
// 5: ALARM :skull:

String twoDigit(int number) {
  if (number < 10) {
    return "0" + String(number);
  } else {
    return String(number);
  }
}

void setup() {
  Serial.begin(115200);

  delay(250);
  rtc.begin();
  if (!display.begin(0x3C, true)) {
    Serial.println("SH1106 allocation failed");
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);  // REQUIRED: Set text color
  display.setTextSize(1);

  // rtc.setAMPM(1);           //set AM or PM, AM=0 and PM=1
  // rtc.setFormat(12);        //Set 12 Hours Format
  // rtc.setTime("02:46:30");  //Set Time    Hour:Minute:Seconds
  // rtc.setDate("08/22/26");  //Set Date    Month/Date/Year

  pinMode(touch, INPUT);
  pinMode(bLeft, INPUT_PULLUP);
  pinMode(bRight, INPUT_PULLUP);
  pinMode(bTop, INPUT_PULLUP);
  pinMode(bBottom, INPUT_PULLUP);
}

bool buttonPressed(int pin) {
  static int lastReading[40];
  static int stableState[40];
  static unsigned long lastChangeTime[40];

  const unsigned long debounceTime = 50;
  int reading = digitalRead(pin);

  if (reading != lastReading[pin]) {
    lastChangeTime[pin] = millis();
    lastReading[pin] = reading;
  }

  if ((millis() - lastChangeTime[pin]) >= debounceTime) {
    if (reading != stableState[pin]) {
      stableState[pin] = reading;
      if (stableState[pin] == LOW) return true;
    }
  }
  return false;
}

void mainClock() {
  // +--------------------------------+  (128 px)
  // | Wi-Fi              [||||] 85%  |  <- Row 0-8: Tiny status indicators
  // |                                |
  // |          12:12:12 PM           |  <- Row 16-32: Time (Font Size 2)
  // |                                |
  // |        MON  10/10/20xx         |  <- Row 40-48: Date & Day (Font Size 1)
  // | 🔔 07:00 AM            24°C    |  <- Row 56-64: Alarm & Temp/Humidity
  // +--------------------------------+  (64 px)

  // int hour = rtc.getHour() % 12;
  // if (hour == 0) hour = 12;

  // Format Time String
  String timeStr = twoDigit(rtc.getHour()) + ":" + twoDigit(rtc.getMinute()) + ":" + twoDigit(rtc.getSeconds()) + (rtc.getAMPM() == 1 ? "PM" : "AM");

  // Format Date String
  String dateStr = twoDigit(rtc.getDate()) + "/" + twoDigit(rtc.getMonth()) + "/20" + twoDigit(rtc.getYear());

  int timeWidth = timeStr.length() * 12;  // 12px per char
  int timeX = (width - timeWidth) / 2;
  int timeY = (height - 16) / 2 - 4;     // -4 is a trial and errored value to make things look better
  int dateWidth = dateStr.length() * 6;  // 6px per char
  int dateX = (width - dateWidth) / 2;
  int dateY = timeY + 18;  // Placed exactly below time with a 2px gap

  display.setTextSize(2);
  display.setCursor(timeX, timeY);
  display.print(timeStr);

  display.setTextSize(1);
  display.setCursor(dateX, dateY);
  display.print(dateStr);
}

void formatTime(unsigned long timer) {
}

void stopWatch(unsigned long start, unsigned long timer) {
  display.setCursor(0, 0);
  display.println(millis());

  if (stopWatchStart && start != 0) timer = millis() - start;
  else timer = 404;
  display.print("START: ");
  display.println(start);
  display.print("TIME");
  display.println(timer);
}

void pomodoro(int normal = 25, int breakTime = 5) {
  // if (buttonPressed(bTop)) pomodoroStart = true;
  // if (pomodoroStart) {
  //   if (currSecs != 0) currSecs -= 1;
  //   else if (currSecs == 0) { currSecs = 59, currMins -= 1; };
  // }
  //
  // // Positioning and Display
  // display.setTextSize(2);
  // String timeStr = twoDigit(currMins) + ":" + twoDigit(currSecs);
  // int timeWidth = timeStr.length() * 12;
  // int timeX = (width - timeWidth) / 2;
  // int timeY = (height - 16) / 2 - 4;
  // display.setCursor(timeX, timeY);
  // display.print(timeStr);
}

void loop() {
  display.clearDisplay();
  if (state > 3) { state = state - 3; }
  if (state < 1) { state = state + 3; }
  if (buttonPressed(bLeft)) state -= 1;
  if (buttonPressed(bRight)) state += 1;

  if (state == 1) {
    if (rtc.getTime()) {
      mainClock();
    } else {
      Serial.println("Cannot get RTC Data");
      display.println("Cannnot get RTC Data");
    }
  } else if (state == 2) {
    if (buttonPressed(bTop)) {
      start = millis();
      stopWatchStart = true;
    }
    stopWatch(start, timer);
  } else if (state == 3) {
    int currSecs = 0, currMins = 25;
    pomodoro();
  }

  display.display();
  delay(1);
}
