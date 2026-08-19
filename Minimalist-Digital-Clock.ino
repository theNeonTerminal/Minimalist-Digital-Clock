#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Bonezegei_DS3231.h>

const int width = 128, height = 64;

Bonezegei_DS3231 rtc(0x68);
Adafruit_SH1106G display(width, height, &Wire, -1);

const int touch = 32, bLeft = 25, bRight = 27;
int state = 1;
// 1: Main Clock
// 2: Stopwatch
// 3: 25-5 Pomodoro Timer

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

  // rtc.setAMPM(0);           //set AM or PM, AM=0 and PM=1
  // rtc.setFormat(6);         //Set 12 Hours Format
  // rtc.setTime("06:50:50");  //Set Time    Hour:Minute:Seconds
  // rtc.setDate("08/19/26");  //Set Date    Month/Date/Year

  pinMode(touch, INPUT);
  pinMode(bLeft, INPUT_PULLUP);
  pinMode(bRight, INPUT_PULLUP);
}

void mainClock() {
  int hour = rtc.getHour() % 12;
  if (hour == 0) hour = 12;

  // Format Time String
  String timeStr = twoDigit(hour) + ":" + twoDigit(rtc.getMinute()) + ":" + twoDigit(rtc.getSeconds()) + (rtc.getAMPM() == 1 ? "AM" : "PM");

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

void loop() {
  display.clearDisplay();
  if (rtc.getTime()) {
    mainClock();
  } else {
    Serial.println("Cannot Get RTC Data");
  }
  display.display();
  delay(1000);
}
