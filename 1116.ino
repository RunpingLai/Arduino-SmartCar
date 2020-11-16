//↓转弯基于检测到全白
#define S1   3
#define S2   A0
#define S3   A1
#define S4   A2
#define S5   A3
#define S6   2
#define ENA  10
#define ENB  9
#define l1   7
#define l2   6
#define r1   5
#define r2   4
#define kp   1
#define ki   0
#define kd   5
#define trs  80

//1 全局变量
int minimal = 1100;
int v1, v2, v3, v4, v5, v6;
int speedSet = 95;
//用于Set,初始速度
int threshold = 55;
int cornerLeftValue = 0;
int cornerRightValue = 0;
int corner = 0;
unsigned long cornerbegin = -1,whitebegin=-1;
float cornerMoment;
float ave, left, right;
float toleft, toright;
float previous_left = 0, previous_right = 0;
float pl, pr, il = 0, ir = 0, dl, dr;

struct {
  float toleft, toright;
} pidParas;

//2 Setup设置
void setup()
{
  // put your setup code here, to run once:
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(l1, OUTPUT);
  pinMode(l2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);
  pinMode(S6, INPUT);

  //下降沿触发，触发中断0，调用turnRight函数
  //  attachInterrupt(0, turnRight, RISING);
  Serial.begin(9600);
  Read();
  Set(70, 70);
}

// 3 Set设速
inline void Set(int sa, int sb)
{
  int abssa = (sa > 0 ? sa : -sa), abssb = (sb > 0 ? sb : -sb);
  if (abssa < 50 && abssb < 50)
  {
    abssa += 60;
    abssb += 60;
  }
  analogWrite(ENA, abssa);
  analogWrite(ENB, abssb);
  digitalWrite(l2, sa > 0);
  digitalWrite(l1, sa <= 0);
  digitalWrite(r2, sb > 0);
  digitalWrite(r1, sb <= 0);
  Serial.print(sa);
  Serial.print(",,,");
  Serial.println(sb);


}

// 4 Read读取传感器数值
inline void Read()
{
  v1 = digitalRead(S1);
  v2 = analogRead(S2);
  v3 = analogRead(S3);
  v4 = analogRead(S4);
  v5 = analogRead(S5);
  v6 = digitalRead(S6);
  if (v2 > 350)v2 = 350;
  if (v3 > 350)v4 = 350;
  if (v4 > 350)v4 = 350;
  if (v5 > 350)v5 = 350;

  if (v2 < minimal)minimal = v2;
  if (v3 < minimal)minimal = v3;
  if (v4 < minimal)minimal = v4;
  if (v5 < minimal)minimal = v5;
  minimal += 10;

}

// 5 pid循线
void pidControl()
{

  Read();
  ave = minimal - 10;
  //pid均速设置
  left = (v2 + v3) / 2 - ave;
  right = (v4 + v5) / 2 - ave;

  pl = left;
  pr = right;
  il = il + left;
  ir = ir + right;
  if (ir > 100)ir = 100;
  if (ir < -100)ir = -100;
  dl = left - previous_left;
  dr = right - previous_right;

  previous_left = left;
  previous_right = right;

  pidParas.toleft = (kp * pl) + (ki * il) + (kd * dl);
  pidParas.toright = (kp * pr) + (ki * ir) + (kd * dr);
  Set(speedSet - pidParas.toleft, speedSet - pidParas.toright);
}

// 6 Print
void Print()
{
  Serial.print(v1);
  Serial.print(',');
  Serial.print(v2);
  Serial.print(',');
  Serial.print(v3);
  Serial.print(',');
  Serial.print(v4);
  Serial.print(',');
  Serial.print(v5);
  Serial.print(',');
  Serial.println(v6);
}

void loop()
{
  Read();
  //Print();
  cornerJudge();
  whiteJudge();
  //if ((millis() - cornerbegin > 80) && cornerbegin != -1) {
  //  Print();
  //  cornerReact();
  //  cornerbegin = -1;
  //}

  //if ((v2 < minimal) && (v3 < minimal) && (v4 < minimal) && (v5 < minimal)) {
  //  //全白
  //  Set(10, 10);
  //  //把这句放到下面if里面怎么样
  //  if (cornerbegin == -1)
  //    cornerbegin = millis();
  //}
  //非全白
  if (cornerbegin == -1) {
    pidControl();
    delay(5);
  }

}
//思路：if全白 到下一个loop里进行cornerReact，同时消除whitebegin印记
//if not 全白，在本loop里进行pidControl，也消除whitebegin印记
//设计初衷：为防止遇到“白色误差”而总是左右转（有时在直线也会出现全白情况），设计了millis()-whitebegin>80才cornerReact
//问题：都清零怎么会满足>80的条件呢
//哦哦懂了，仔细看第一个ifif，如果上个loop辨别到了whitebegin，则本loop仅进行Set(0,0)

void cornerJudge()
{
  if (v1 || v6)
  {
    //    Print();
    if (cornerbegin == -1) {
      if (v1)
        corner = 1;
      else if (v6)
        corner = 6;
      cornerbegin = millis();
      Set(170, 170);
    }
    else if (millis() - cornerbegin < 150 && cornerbegin != -1) {
      //      Print();
      if ((corner == 1 && v6) || (corner == 6 && v1)) {
        corner = 0;
        //        Print();
      }
    }
  }
  if (millis() - cornerbegin >= 150 && cornerbegin != -1) {
    //      Print();
    if (corner != 0)
      cornerReact2();
    corner = 0;
    cornerbegin = -1;
  }
}
//
//void cornerReact()
//{
////  Serial.print("1111\n");
//
//  int tempminimal = minimal;
////  Set(1,1);
////  delay(90);
//  //  if (corner ！= 0)
//  //  {
//  //    corner = ((cornerLeftValue>cornerRightValue)?1:6);
//  //  }
//  //
////  delay(20);
//  Set(-100,-100);
//  delay(300);
//  if (corner == 6) {
//    Set(100, -50);
//  }
//  else if (corner == 1) {
//    Set(-50, 100);
//  }
//  cornerMoment = millis();
//  do {
//    Read();
//    if (millis() - cornerMoment > 2000)
//      break;
//  } while ((v3 < tempminimal && v4 < tempminimal || (v2>tempminimal || v5>tempminimal)) || (millis() - cornerMoment < 350));
//  Set(100,100);
//  delay(400);
//}

void whiteJudge() {
  if ((!v1 && v2 < minimal && v3 < minimal && v4 < minimal && v5 < minimal && !v6)) {//全白
    if(whitebegin==-1)
      whitebegin = millis();
  }
  else {//非全白
    whitebegin = -1;
  }
  if (millis() - whitebegin > 150 && whitebegin != -1) {
    whitebegin = -1;
    turnBack();
  }
}
void turnBack() {
  int tempminimal = minimal;
  Set(-100, -100);
  delay(300);
  Set(-100, 100);
  int whiteMoment=millis();
  do {
    Read();
    if (millis() - whiteMoment > 2000)
      break;
  } while (!(v1) || !(millis() - whiteMoment > 700));
  delay(100);
  do {
    Read();
    if (millis() - whiteMoment > 500)
      break;
  } while (!(v2 < tempminimal && v5 < tempminimal && (v3 > 100 || v4 > 100) && !v1 && !v6) || !((millis() - whiteMoment > 100)));
}
void cornerReact2() {
  int tempminimal = minimal;
  Set(-100, -100);
  delay(300);
  if (corner == 6) {
    Set(100, -80);
  }
  else if (corner == 1) {
    Set(-80, 100);
  }
  cornerMoment = millis();
  do {
    Read();
    if (millis() - cornerMoment > 800)
      break;
  } while (!(corner == 1 ? v1 : v6) || !(millis() - cornerMoment > 100));
  delay(200);
  do {
    Read();
    if (millis() - cornerMoment > 500)
      break;
  } while (!(v2 < tempminimal && v5 < tempminimal && (v3 > 100 || v4 > 100) && !v1 && !v6) || !((millis() - cornerMoment > 100)));
  Set(150, 150);
  delay(100);
}
//void vagueDirectionRecord()
//{
//  int i;
//  cornerLeftValue = 0;
//  cornerRightValue = 0;
//  for (i = 0; i < 5; i++)
//  {
//    Read();
//    cornerLeftValue += v2;
//    cornerRightValue += v5;
//  }
//}

//梳理：
//可调项：
//pid均速设置
//speedSet
//kp kd


//问题：
//遇到拐弯 但直接冲出去了
//直线过程中可能会调头
//速度不够就不会转弯 而是会无限卡顿
