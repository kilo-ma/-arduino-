//稳定版v4
//新增：1:优化UI
//     2:电量监控

#include <U8g2lib.h>
#include <U8x8lib.h>
#include <MsTimer2.h>
#include <Arduino.h>
#include <Wire.h>
#include <String.h>


U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0);


#define  TrigPin 11                   //vcc = 5v, distance = 2-450cm ,
#define  EchoPin 10
#define  KEY 3
#define  BUTTON_PIN_1 4
#define  VoltReadPin A2

#define  menuY 42
#define  headY 10

volatile float time_a = 0;
volatile float cm = 0;
volatile boolean state_1 = 0;
volatile float sum5 = 0;
volatile float temp = 0;
volatile int  power = 100;
volatile int modeNum = 0;
volatile byte shortTouch = 0;
volatile float result = 0;
volatile float a, b = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(EchoPin, INPUT);
  pinMode(TrigPin, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);

  pinMode(KEY, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KEY), state_a, RISING);

  u8g2.begin();
  display_function_1();
  delay(1000);

  MsTimer2::set(1000, PowerState);
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly
  if (state_1 == 1) {
    if ( modeNum == 0 ) distance();
    else ave_cm();               //5次测量平均值---》cm
    reset_1();
  }

  GUI_flash();

  judgeButton();
  shortTouch = 0;
}


void distance() {             //单次测距；
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(60);
  digitalWrite(TrigPin, LOW);

  time_a = float(pulseIn(EchoPin, HIGH));
  cm = (time_a * 17) / 1000;
}

void print_distance() {       //串口打印；
  Serial.print(cm);
  Serial.println("cm");
}

void state_a() {              //按键检测；
  state_1 = 1;
}

void reset_1() {              //重置
  state_1 = 0;
}

void display_function_1() {     //oled打印 开机画面；
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso20_tr);
    u8g2.drawStr(10, menuY, "Power On");
  } while ( u8g2.nextPage() );
}


void ave_cm() {               //5次测量取平均提高精度
  for (int i = 0; i < 5; i++) {
    distance();
    sum5 = sum5 + cm;
    delay(60);
  }
  cm = sum5 / 5;
  sum5 = 0;
}



void PowerState() {                 //计算剩余电量
  int now_power;
  int val = analogRead(VoltReadPin);
  val = constrain(val, 758, 854);
  now_power = map(val, 758, 854, 0, 100);
  if ( abs(now_power - power) > 1 )
    power = now_power;
  Serial.println(power);
}





void  judgeButton() {
  int k0 = 1; int tt = 0;
  k0 = digitalRead(BUTTON_PIN_1);
  while ( k0 == 0 ) {
    tt = tt + 1;
    delay(10);
    k0 = digitalRead(BUTTON_PIN_1);
  }
  if (tt > 50) {                //长按检测
    changeMode();
  }
  else {
    if (tt > 0 && tt < 30 ) {
      shortTouch = 1;
      Serial.println("0");
    }
  }       //短按检测
}

void  changeMode() {            //mode循环
  if (modeNum < 3) modeNum = modeNum + 1;
  else  modeNum = 0;
}

void area() {

  if (a == 0) {
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.setCursor(5, 64);
    u8g2.print("Area: ");
    u8g2.setCursor(45, 64);
    u8g2.print("ReadLength");
    judgeButton();
    if (shortTouch == 1) {
      a = cm;
      shortTouch = 0;
    }
  }

  if ((a != 0) && (b == 0)) {
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.setCursor(5, 64);
    u8g2.print("Area: ");
    u8g2.setCursor(45, 64);
    u8g2.print("ReadWidth");
    judgeButton();
    if (shortTouch == 1) {
      b = cm;
      shortTouch = 0;
    }
  }

  if ((a != 0) && (b != 0)) {
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.setCursor(5, 64);
    u8g2.print("Area: ");
    u8g2.setCursor(43, 64);
    float sum = a * b;
    if ((sum / 10000) > 1) {
      sum = sum / 10000;
      u8g2.print(sum);
      u8g2.setCursor(100, 64);
      u8g2.print("M^2");
    }
    else {
      u8g2.print(int(sum));
      u8g2.setCursor(100, 64);
      u8g2.print("Cm^2");
    }
    judgeButton();
    if (shortTouch == 1) {
      a = 0;
      b = 0;
      shortTouch = 0;
    }

  }
}


void GUI_flash() {                //oled打印总界面
  u8g2.firstPage();
  do {
    head_display();
    body_display();
    tail_display();
  } while ( u8g2.nextPage() );
  delay(10);
}

void head_display() {
  Power();
  Mode();
}

void body_display() {
  if (power < 5 ) {
    display1();
  }
  else  if ((cm > 450)) {
    display2();
  }
  else display0();

}

void tail_display() {
  switch (modeNum)
  {
    case 0: Simple();
      break;
    case 1: Average();
      break;
    case 2: area();
      break;
    case 3: ;
      break;
  }

}


void Power() {
  u8g2.drawBox(3, 1, 4, 4);
  u8g2.drawFrame(1, 4, 9, 11);
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.setCursor(16, headY);
  u8g2.print(power);
  u8g2.drawStr(38, headY, "%");
}

void Mode() {
  u8g2.setCursor(80, headY);
  u8g2.print("Mode: ");
  u8g2.setCursor(116, headY);
  u8g2.print(modeNum);
}

void Simple() {
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.setCursor(5, 64);
  u8g2.print("SimpleMode");
}

void Average() {
  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.setCursor(5, 64);
  u8g2.print("AveMode");
}

void display0() {
  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.drawStr(0, menuY, "cm:");
  u8g2.setCursor(45, menuY);
  u8g2.print(cm);
}


void display1() {
  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.drawStr(10, menuY, "LOW BATTERT");
}


void display2() {
  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.drawStr(30, menuY, "ERROR");
}
