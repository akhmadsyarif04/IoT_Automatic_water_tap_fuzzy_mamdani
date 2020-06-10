#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul
#include <DHT.h>
DHT dht(2, DHT11); //Pin, Jenis DHT
#include <Servo.h>
Servo motorServo; 

//sensor ldr
const int pin_ldr = A0;
int powerPin = 3;    // untuk pengganti VCC/5vOLT
#define r1 8

//Rule Base
float suhu [2];
float kelembaban [2];
float cahaya [2];
float rule [2][2];
float rule00, rule01;
float rule10, rule11;
float defuz, pwm, defuzzy;
float temp;
float sensorKelembaban;
 float sensorSuhu;
 float ruleMin [4];
// int sensorCahaya;



void setup(){
  
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight(); 
  // jadikan pin power sebagai output
  pinMode(powerPin, OUTPUT);
  // default bernilai LOW
  digitalWrite(powerPin, LOW);
 Serial.begin(9600);
 dht.begin();
 pinMode(r1, OUTPUT);
 pinMode(r1, OUTPUT);
 motorServo.attach(10);
}
 
void loop(){
 digitalWrite(powerPin, HIGH);

float sensorSuhu = dht.readTemperature();
float sensorKelembaban = dht.readHumidity();

// sensorSuhu = 27.00;
// sensorKelembaban = 65.00 ;
// sensorCahaya = 2;
// float sensorCahaya = dht.readTemperature();

//  fuzzifikasi
 FuzzySuhu(sensorSuhu);
 FuzzyKelembaban(sensorKelembaban);

// defuzzifikasi
 Defuzzy();

// Serial.print("k: ");
// Serial.print(sensorKelembaban);
// Serial.print(" ");
// Serial.print("s: ");
// Serial.println(sensorSuhu);
//  Serial.print("c: ");
// Serial.println(sensorCahaya);
// Serial.print("fuzzy: ");
// Serial.println(defuzzy);
 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("K: ");
  lcd.setCursor(2,0);
  lcd.print(sensorKelembaban);
  lcd.setCursor(8,0);
  lcd.print("S: ");
  lcd.setCursor(10,0);
  lcd.print(sensorSuhu);

  lcd.setCursor(0,1);
//  lcd.print("C: ");
//  lcd.setCursor(2,1);
//  lcd.print(sensorCahaya);
  lcd.setCursor(8,1);
  lcd.print("F: ");
  lcd.setCursor(10,1);
  lcd.print(defuzzy);

  delay(5000);

//  servo
   motorServo.write(defuzzy);  // Turn Servo 

}


void FuzzySuhu(float sensorSuhu){
  // untuk suhu hampir panas
  if (sensorSuhu <= 27.00)
  { suhu [0] = 1;}
  else if (sensorSuhu > 27.00 && sensorSuhu <= 30.00)
  {  suhu [0] = (30.00 - sensorSuhu)/(30.00 - 27.00); }
  else
  { suhu [0] = 0;}
  // untuk suhu panas
  if (sensorSuhu <= 27.00)
  { suhu [1] = 0;}
  else if (sensorSuhu > 27.00 && sensorSuhu <= 30.00)
  { suhu [1] = (sensorSuhu-27.00)/(30.00-27.00);}
  else
  { suhu [1] = 1;}
  Serial.print("suhu hampir panas 0: ");
  Serial.println(suhu[0]);
  Serial.print("suhu panas 1: ");
  Serial.println(suhu[1]);
  
}

void FuzzyKelembaban(float sensorKelembaban){
  // untuk panas
  if (sensorKelembaban <= 65.00)
  { kelembaban [0] = 1;}
  else if (sensorKelembaban > 65.00 && sensorKelembaban <= 70.00)
  {  kelembaban [0] = (70.00 - sensorKelembaban)/(70.00 - 65.00); }
  else
  { kelembaban [0] = 0;}

  // untuk hampir panas
  if (sensorKelembaban <= 65.00)
  { kelembaban [1] = 0;}
  else if (sensorKelembaban > 65.00 && sensorKelembaban <= 70.00)
  { kelembaban [1] = (sensorKelembaban-65.00)/(70.00 - 65.00);}
  else
  { kelembaban [1] = 1;}
  Serial.print("kelembaban hampir panas 0: ");
  Serial.println(kelembaban[0]);
  Serial.print("kelembaban panas 1: ");
  Serial.println(kelembaban[1]);
}


// rule dan defuzzifikasi belum dirubah
void RuleEva (){
 int i, j;
 for ( i=0; i<=1; i=i+1)
 {
   for ( j=0; j<=1; j=j+1)
   {
     temp = min(suhu[i], kelembaban[j]);
     rule [i][j] = temp;
   } 
 } 
 rule00 = rule [0][0]; // (hampir panas,hampir panas = )
 rule01 = rule [0][1]; // (hampir panas,panas = )
 
 rule10 = rule [1][0]; // (panas,hampir panas = )
 rule11 = rule [1][1]; // (panas,panas = )
 
}

void Defuzzy () {
  // metode mamdani (weighted average)
  
  ruleMin[1] = min(suhu[0],kelembaban[0]);
  ruleMin[2] = min(suhu[0],kelembaban[1]);
  ruleMin[3] = min(suhu[1],kelembaban[0]);
  ruleMin[4] = min(suhu[1],kelembaban[1]);
  Serial.print("HH : ");
  Serial.println(ruleMin[1]);
  Serial.print("HP : ");
  Serial.println(ruleMin[2]);
  Serial.print("PH : ");
  Serial.println(ruleMin[3]);
  Serial.print("PP : ");
  Serial.println(ruleMin[4]);

   int i, j;
   float RuleMin = 1;
   for ( i=1; i<=4; i=i+1)
   {
       if(rule[i] == 0){
         RuleMin = min(RuleMin, ruleMin[i]);
       }
   }
//   Serial.print("rule min for : ");
//   Serial.println(RuleMinFor);

   int m, n;
   float RuleMax = 1;
   for ( m=1; m<=4; m=m+1)
   {
       if(rule[m] == 0){
         RuleMax = max(RuleMax, ruleMin[m]);
       }
   }

//  float RuleMin1 = min(ruleMin[1], ruleMin[2]);
//  float RuleMin2 = min(ruleMin[3], RuleMin1);
//  float RuleMin = min(ruleMin[4], RuleMin2);
//
//  float RuleMax1 = max(ruleMin[1], ruleMin[2]);
//  float RuleMax2 = max(ruleMin[3], RuleMax1);
//  float RuleMax = max(ruleMin[4], RuleMax2);
//  
  Serial.print("rule min : ");
  Serial.println(RuleMin);
  Serial.print("rule max : ");
  Serial.println(RuleMax);

  float a1 = (RuleMin*(45-90))+45;
  float a2 = (RuleMax*(45-90))+45;

  Serial.print("a1 : ");
  Serial.println(a1);
  Serial.print("a2 : ");
  Serial.println(a2);

  float m1a = RuleMin/2;
  float m1 = pow((m1a*a1),2)-pow((m1a*0),2);
  Serial.print("m1 : ");
  Serial.println(m1);

  float tahap1_m2a = 1/45;
  float tahap1_m2b = 45/45;
  float tahap2_m2a = 45/3; // disini ragu apakah 45/3 atau 3/45
  float tahap2_m2b = tahap1_m2b/2;
  float tahap3_pangkat1 = 1;
  for(int a=1;a<=3;a++){
        tahap3_pangkat1 = tahap3_pangkat1*a1;
  }
  float tahap3_pangkat2 = 1;
  for(int a=1;a<=3;a++){
        tahap3_pangkat2 = tahap3_pangkat2*a2;
  }
  float tahap3_m2a = (tahap2_m2a*tahap3_pangkat1)-(tahap2_m2a*tahap3_pangkat2);
  float tahap3_pangkat3 = 1;
  for(int a=1;a<=3;a++){
        tahap3_pangkat3 = tahap3_pangkat3*a1;
  }
  float tahap3_pangkat4 = 1;
  for(int a=1;a<=3;a++){
        tahap3_pangkat4 = tahap3_pangkat4*a2;
  }
  float tahap3_m2b = (tahap2_m2b*tahap3_pangkat3)-(tahap2_m2b*tahap3_pangkat4);

  float m2 = tahap3_m2a - tahap3_m2b;
  Serial.print("m2 : ");
  Serial.println(m2);

  float m3a = RuleMax/2;
  float m3b = 1;
  for(int a=1;a<=2;a++){
        m3b=m3b*90;
  }
  float m3c = 1;
  for(int a=1;a<=2;a++){
        m3c=m3c*a2;
  }
  float m3 = (m3a*m3b)-(m3a*m3c);
  Serial.print("m3 : ");
  Serial.println(m3);

  float luasArea_A1 = a1*RuleMin;
  float luasArea_A2 = ((RuleMin+RuleMax)*(a1-a2))/2;
  float luasArea_A3 = (90-a2)*RuleMax;
  Serial.print("luasArea_A1 : ");
  Serial.println(luasArea_A1);
  Serial.print("luasArea_A2 : ");
  Serial.println(luasArea_A2);
  Serial.print("luasArea_A3 : ");
  Serial.println(luasArea_A3);

  defuzzy = (m1+m2+m3)/(luasArea_A1+luasArea_A2+luasArea_A3);
  Serial.print("defuzzy : ");
  Serial.println(defuzzy);
}
