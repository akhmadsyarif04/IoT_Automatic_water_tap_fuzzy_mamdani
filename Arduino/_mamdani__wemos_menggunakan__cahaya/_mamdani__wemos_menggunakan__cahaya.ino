#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
Servo motorServo;

const char* ssid = "dragneel"; 
const char* password = "Fire Dragneel"; 
const char* host = "192.168.1.11"; //edit the host adress, ip address etc. 
 
#define pinSensor A0
int sensorValue = 0;

float sensorSuhu;
float sensorKelembaban;
float sensorCahaya;

//Rule Base
float suhu [2];
float kelembaban [2];
float cahaya [2];
float rule [2][2][2];
float rule000, rule001, rule010, rule011, rule100, rule101, rule110, rule111 ;
float defuz, pwm, defuzzy;
float temp, minsuhuKelem;
float ruleMin [8];

void setup() {
  lcd.begin(); // initializing the lcd 16x2
//  lcd.setBacklightPin(3, POSITIVE); // enable or turn on the backlight
//  lcd.setBacklight(HIGHT);

//  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  
  Serial.begin(115200);
  delay(5000);
  
  motorServo.attach(D4); //D4
  motorServo.write(20); // set jadi 0 pertama kali

  //cahaya
  pinMode(A0, INPUT);

//  Serial.print("{\"humidity\": ");
//  Serial.print(kelembaban);
//  Serial.print(", \"temp\": ");
//  Serial.print(suhu);
//  Serial.print("}\n");


 // =========================================================
 // wifi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop() {
//  sensorKelembaban = dht.readHumidity();
//  sensorSuhu = dht.readTemperature();

  //sensor ldr
  int nilai = analogRead(A0);   // Baca nilai sensor
  float Vout = nilai*0.0048828125;
//  sensorCahaya = 500/(10*((5-Vout)/Vout)); // ini yang benar conversi lux

//  float lux2=(2500/Vout-500)/10;
//  Serial.println(sensorCahaya);
//  Serial.println(lux2);

 sensorSuhu = 23.00;
 sensorKelembaban = 83.00 ;
 sensorCahaya = 0.35;

//  fuzzifikasi
   FuzzySuhu(sensorSuhu);
   FuzzyKelembaban(sensorKelembaban);
   FuzzyCahaya(sensorCahaya);

// defuzzifikasi
   Defuzzy();
   
// servo
   motorServo.write(defuzzy);

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
  lcd.print("C: ");
  lcd.setCursor(2,1);
  lcd.print(sensorCahaya);
  lcd.setCursor(8,1);
  lcd.print("F: ");
  lcd.setCursor(10,1);
  lcd.print(defuzzy);
// ==============================================================
  // koneksi wifi
  sensorValue = analogRead(pinSensor);
 
  Serial.print("connecting to ");
  Serial.println(host);
 
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
 
  // We now create a URI for the request
  String url = "/walet/add.php?";
  url += "suhu=";
  url += sensorSuhu;
  url += "&kelembaban=";
  url += sensorKelembaban;
  url += "&cahaya=";
  url += sensorCahaya;
  url += "&keran=";
  url += defuzzy;
  
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
 
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
 
    if (line.indexOf("sukses gaes") != -1) {
      Serial.println();
      Serial.println("Yes, data masuk");
    } else if (line.indexOf("gagal gaes") != -1) {
      Serial.println();
      Serial.println("Maaf, data gagal masuk");
      //digitalWrite(alarmPin, HIGH);
    }
  }
 
//  Serial.println();
//  Serial.println("closing connection");
  delay(10000); // delay 10detik
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
  { suhu [1] = 4;}

  Serial.print("suhu 0: ");
  Serial.println(suhu[0]);
    Serial.print("suhu 1: ");
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
//  Serial.print("kelembaban functions: ");
//  Serial.println(sensorKelembaban);
}

void FuzzyCahaya(float sensorCahaya){
  // untuk hampir panas
  if (sensorCahaya <= 0.35)
  { cahaya [0] = 1;}
  else if (sensorCahaya > 0.35 && sensorCahaya <= 1.00)
  {  cahaya [0] = (1.00 - sensorCahaya)/(1.00 - 0.35); }
  else
  { cahaya [0] = 0;}

  // untuk panas
  if (sensorCahaya <= 0.35)
  { cahaya [1] = 0;}
  else if (sensorCahaya > 0.35 && sensorCahaya <= 1.00)
  { cahaya [1] = (sensorCahaya-0.35)/(1.00 - 0.35);}
  else
  { cahaya [1] = 1;}
  Serial.print("cahaya: ");
  Serial.println(sensorCahaya);
  Serial.print("cahaya functions 0: ");
  Serial.println(cahaya[0]);
  Serial.print("cahaya functions 1: ");
  Serial.println(cahaya[1]);
}

// rule dan defuzzifikasi belum dirubah
void RuleEva (){
 int i, j, c;
 for ( i=0; i<=1; i=i+1)
 {
   for ( j=0; j<=1; j=j+1)
   {
      for ( c=0; c<=1; c=c+1)
      {
         minsuhuKelem = min(suhu[i], kelembaban[j]);
         temp = min(minsuhuKelem, cahaya[c]); 
         rule [i][j][c] = temp;
      }
   } 
 } 
 rule000 = rule [0][0][0]; // (hampir panas,hampir panas, hampir panas = )
 rule001 = rule [0][0][1]; // (hampir panas,hampir panas, panas = )
 
 rule010 = rule [0][1][0]; // (hampir panas,panas, hampir panas = )
 rule011 = rule [0][1][1]; // (hampir panas,panas, panas = )
 
 rule100 = rule [1][0][0]; // (panas,hampir panas, hampir panas = )
 rule101 = rule [1][0][1]; // (panas,hampir panas, panas = )

 rule110 = rule [1][1][0]; // (panas, panas, hampir panas = )
 rule111 = rule [1][1][1]; // (panas,panas, panas = )
 
}

void Defuzzy () {
  // metode mamdani (weighted average)
  int y, z, c;
  int x = 1;
  for ( y=0; y<=1; y=y+1)
  {
    for ( z=0; z<=1; z=z+1)
    {
      for ( c=0; c<=1; c=c+1)
      {
        float suhuKelembaban = min(suhu[y],kelembaban[z]);
        ruleMin[x] = min(suhuKelembaban,cahaya[c]);
        x++;
      }
    } 
  } 
//  ruleMin[1] = min(suhu[0],kelembaban[0],cahaya[0]);
//  ruleMin[2] = min(suhu[0],kelembaban[0],cahaya[1]);
//  ruleMin[3] = min(suhu[0],kelembaban[1],cahaya[0]);
//  ruleMin[4] = min(suhu[0],kelembaban[1],cahaya[1]);
//  ruleMin[5] = min(suhu[1],kelembaban[0],cahaya[0]);
//  ruleMin[6] = min(suhu[1],kelembaban[0],cahaya[1]);
//  ruleMin[7] = min(suhu[1],kelembaban[1],cahaya[0]);
//  ruleMin[8] = min(suhu[1],kelembaban[1],cahaya[1]);
  
  Serial.print("HHH : ");
  Serial.println(ruleMin[1]);
  Serial.print("HHP : ");
  Serial.println(ruleMin[2]);
  Serial.print("HPH : ");
  Serial.println(ruleMin[3]);
  Serial.print("HPP : ");
  Serial.println(ruleMin[4]);
  Serial.print("PHH : ");
  Serial.println(ruleMin[5]);
  Serial.print("PHP : ");
  Serial.println(ruleMin[6]);
  Serial.print("PPH : ");
  Serial.println(ruleMin[7]);
  Serial.print("PPP : ");
  Serial.println(ruleMin[8]);

   int i, j;
   float RuleMin = 2;
   for ( i=1; i<=8; i=i+1)
   {
       if(ruleMin[i] > 0){
         RuleMin = min(RuleMin, ruleMin[i]);
         Serial.println(ruleMin[i]);
       }
   }
//   Serial.print("rule min for : ");
//   Serial.println(RuleMinFor);

   int m, n;
   float RuleMax = 0;
   for ( m=1; m<=8; m=m+1)
   {
       if(ruleMin[m] > 0){
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

  float a1, a2;
//  a1 = (RuleMin*(90-45))+45;
//  a2 = (RuleMax*(90-45))+45;
      
  if(sensorSuhu <= 27.00 && sensorKelembaban >= 75.00 && sensorCahaya <= 0.35  ){
    Serial.println("normal normal");
      a1 = ((RuleMin*(90-45))-90)*(-1);
      a2 = (RuleMax*(90-45))+45;
  }else if( sensorSuhu >= 30.00 && sensorKelembaban <= 65.00  && sensorCahaya >= 1 ){
    Serial.println("panas panas");  
      a1 = (RuleMin*(90-45))+45;
      a2 = ((RuleMax*(90-45))-90)*(-1);
  }else if( sensorSuhu < 30.00 && sensorSuhu > 27.00 && sensorKelembaban > 65.00 && sensorKelembaban < 75.00 && sensorCahaya > 0.35 && sensorCahaya < 1){
      Serial.println("hampir panas hampir panas");
      a1 = ((RuleMin*(90-45))-90)*(-1);
      a2 = (RuleMax*(90-45))+45;
  }else{
      Serial.println("panas hampir panas");
      a1 = ((RuleMin*(90-45))+45);
      a2 = ((RuleMax*(90-45))-90)*(-1);
  }



  Serial.print("a1 : ");
  Serial.println(a1);
  Serial.print("a2 : ");
  Serial.println(a2);

  float m1a = RuleMin/2;
//  float m1 = pow((m1a*a1),2)-pow((m1a*0),2);
  float m1b = 1;
  // perpangkatan
  for(int a=1;a<=2;a++){
        m1b=m1b*a1;
  }
 float m1c = 1;
  for(int a=1;a<=2;a++){
        m1c=m1c*0;
  }
  float m1 = (m1a*m1b)-(m1a*m1c);
  Serial.print("m1 : ");
  Serial.println(m1);

  double tahap1_m2a = 0.022;
  float tahap1_m2b = 45/45;
//  double tahap2_m2a = tahap1_m2a/3; // disini ragu apakah 45/3 atau 3/45
  double tahap2_m2a = 0.00740;
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
  for(int a=1;a<=2;a++){
        tahap3_pangkat3 = tahap3_pangkat3*a1;
  }
  float tahap3_pangkat4 = 1;
  for(int a=1;a<=2;a++){
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
  float luasArea_A2 = ((RuleMin+RuleMax)*(a2-a1))/2;
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
