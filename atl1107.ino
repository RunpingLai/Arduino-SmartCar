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
#define kp   0.6
#define ki   0
#define kd   15
#define trs  80

//1 全局变量
int minimal = 1100;
int v1, v2, v3, v4, v5, v6;
int speedSet = 65;
//用于Set,初始速度
int threshold = 55;
int cornerLeftValue = 0;
int cornerRightValue = 0;
int corner = 0;
unsigned long whitebegin = -1;
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
  analogWrite(ENA, speedSet + (sa > 0 ? sa : -sa));
  analogWrite(ENB, speedSet + (sb > 0 ? sb : -sb));
  digitalWrite(l2, sa > 0);
  digitalWrite(l1, sa <= 0);
  digitalWrite(r2, sb > 0);
  digitalWrite(r1, sb <= 0);
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
  if ((millis() - whitebegin > 80) && whitebegin != -1) {
    Print();
    cornerReact();
    whitebegin = -1;
  }

  if ((v2 < minimal) && (v3 < minimal) && (v4 < minimal) && (v5 < minimal)) {
    //全白
    Set(0, 0);
    //把这句放到下面if里面怎么样
    if (whitebegin == -1)
      whitebegin = millis();
  }
  else {
    //非全白
    whitebegin = -1;
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

  //  vagueDirectionRecord();
  if (v1 || v6)
  {
    if (v1)
      corner = 1;
    if (v6)
      corner = 6;
    //      Print();
  }
}

void cornerReact()
{
  //  Set(-100, -100);
  //  delay(50);
  cornerMoment = millis();
  int tempminimal = minimal;
  //  if (corner ！= 0)
  //  {
  //    corner = ((cornerLeftValue>cornerRightValue)?1:6);
  //  }
  //
  if (corner == 6) {
    Set(20, -20);
  }
  else if (corner == 1) {
    Set(-20, 20);
  }
  do {
    Read();
    if (millis() - cornerMoment > 2000)
      break;
  } while ((v3 < tempminimal && v4 < tempminimal) || (millis() - cornerMoment < 600));
  corner = 0;
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
