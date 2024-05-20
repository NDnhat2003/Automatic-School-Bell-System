#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ I2C của màn hình LCD. 0x27 là địa chỉ phổ biến.
//thời gian chuông kêu
int bellHour;
int bellMinute;
int bellSecond = 0;
//thời gian hiện tại
int currentHour;
int currentSecond;
int currentMinute;
int dayOfWeek;  //0 là chủ nhật, 1 là thứ 2, ..., 6 là thứ 7
char daysOfTheWeek[7][12] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

int startTime[] = { 7, 0 };  //thời gian bắt đầu ngày học {HH, MM} và MM cũng là phút bắt đầu một tiết học trong ngày (8h00, 14h00,...)
int endTime[] = { 17, 50 };  //thời gian kết thúc ngày học {HH, MM} và MM cũng là phút kết thúc một tiết trong ngày (8h50, 14h50,...)
boolean bellState = true;    //tình trang chuông sẵn sàng kêu hay không

int buzzer = 10;  //Chân pin của chuông
int button = 7;   //chân pin của nút bấm

void setup() {  //khởi tạo I2C, RTC, LCD, thời gian chuông kêu, chân kết nối
  lcd.init();
  lcd.backlight();
  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  lcd.begin(16, 2);
  Wire.begin();
  if (!rtc.begin()) {
    lcd.print("RTC:Not found");
    while (1);
  }
  if (!rtc.isrunning()) {
    lcd.print("RTC:Not running");
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  resetBellTime();  //cài đặt thời gian tiếp theo chuông sẽ kêu dựa vào thời gian hiện tại
}

void loop() {
  DateTime now = rtc.now();
  currentHour = now.hour();
  currentMinute = now.minute();
  currentSecond = now.second();

  if (bellState) {  //nếu thời gian hiện tại đến thời điểm đặt sẵn, chuông sẽ kích hoạt
    if (currentHour == bellHour && currentMinute == bellMinute && currentSecond == bellSecond) {
      activateBell();
    }
  }
  disableBell();  //kiểm tra nút bấm, nếu nút được bấm sẽ vô hiệu hóa chuông và thoát vô hiệu hóa khi bấm lần nữa
  lcd.clear();
  showTime();      //hiển thị thời gian hiện tại và thứ trong tuần trên dòng 1 màn LCD
  showBellTime();  //hiển thị thời gian tiếp theo chuông sẽ kêu, nếu chuông đang bị vô hiệu hóa thì hiện "Bell Disabled"
  delay(1000);      //tần số hiển thị là 1 giây
}

void disableBell() {
  while (digitalRead(button) == LOW) {
    bellState = !bellState;
    resetBellTime();
    break;
  }
}

void resetBellTime() {
  DateTime now = rtc.now();
  if (now.hour() >= startTime[0] && now.hour() <= endTime[0]) {
    if (now.minute() >= endTime[1] && now.hour() == endTime[0]) {
      bellHour = startTime[0];
    }
    if (now.minute() < endTime[1]) {
      if ((now.hour() > 11) && (now.hour() < 13)) {
        bellHour = 13;
        bellMinute = startTime[1];
      } else {
        bellMinute = endTime[1];
        bellHour = now.hour();
      }
    } else {
      if ((now.hour() >= 11) && (now.hour() < 13)) {
        bellHour = 13;
        bellMinute = startTime[1];
      } else {
        bellHour = now.hour() + 1;
        bellMinute = startTime[1];
      }
    }
  } else {
    bellHour = startTime[0];
  }
}

void activateBell() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BELL RING!");
  digitalWrite(buzzer, HIGH);
  delay(5000);
  digitalWrite(buzzer, LOW);
  resetBellTime();
  lcd.clear();
}

void showBellTime() {
  lcd.setCursor(0, 1);
  if (bellState) {
    lcd.print("Bell Time: ");
    PrintTwoDigits(bellHour);
    lcd.print(':');
    PrintTwoDigits(bellMinute);
  } else {
    lcd.print("Bell Disabled");
  }
}

void PrintTwoDigits(int number) {  //hiển thị dạng số 2 đơn vị
  if (number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}

void display(int h, int m, int s) {  //hiện thì thời gian dạng HH:MM:SS
  PrintTwoDigits(h);
  lcd.print(':');
  PrintTwoDigits(m);
  lcd.print(':');
  PrintTwoDigits(s);
}

void showTime() {
  lcd.setCursor(4, 0);
  display(currentHour, currentMinute, currentSecond);
  lcd.print(" ");
  lcd.print(daysOfTheWeek[dayOfWeek]);
}