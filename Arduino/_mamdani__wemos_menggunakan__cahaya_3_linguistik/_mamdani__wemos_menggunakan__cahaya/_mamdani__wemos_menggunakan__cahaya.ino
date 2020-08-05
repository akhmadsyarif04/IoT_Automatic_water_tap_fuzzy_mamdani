#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // GANTI 0x3F Ke 0x27 kalau LCD ga muncul

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
Servo motorServo;

const char* ssid = "361Carwash"; 
const char* password = "361Carwash"; 
const char* host = "192.168.100.12"; //edit the host adress, ip address etc. 
 
#define pinSensor A0
int sensorValue = 0;

float sensorSuhu;
float sensorKelembaban;
float sensorCahaya;

//Rule Base
float suhu [3];
float kelembaban [3];
float cahaya [3];
float rule [3][3][3];
float rule000, rule001, rule002;
float rule010, rule011, rule012;
float rule020, rule021, rule022;

float rule100, rule101, rule102;
float rule110, rule111, rule112;
float rule120, rule121, rule122;

float rule200, rule201, rule202;
float rule210, rule211, rule212;
float rule220, rule221, rule222;

float defuz, pwm, defuzzy, keran;
float temp, minsuhuKelem;
float ruleMin [28];

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
  sensorKelembaban = dht.readHumidity();
  sensorSuhu = dht.readTemperature();

  //sensor ldr
  int nilai = analogRead(A0);   // Baca nilai sensor
  float Vout = nilai*0.0048828125;
  sensorCahaya = 500/(10*((5-Vout)/Vout)); // ini yang benar conversi lux


// sensorSuhu = 32.00;
// sensorKelembaban = 63.00 ;
// sensorCahaya = 0.88;
 
//  fuzzifikasi
   FuzzySuhu(sensorSuhu);
   FuzzyKelembaban(sensorKelembaban);
   FuzzyCahaya(sensorCahaya);

// defuzzifikasi
   Defuzzy();
   
// servo
   motorServo.write(keran);

//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("K: ");
//  lcd.setCursor(2,0);
//  lcd.print(sensorKelembaban);
//  lcd.setCursor(8,0);
//  lcd.print("S: ");
//  lcd.setCursor(10,0);
//  lcd.print(sensorSuhu);
//
//  lcd.setCursor(0,1);
//  lcd.print("C: ");
//  lcd.setCursor(2,1);
//  lcd.print(sensorCahaya);
//  lcd.setCursor(8,1);
//  lcd.print("F: ");
//  lcd.setCursor(10,1);
//  lcd.print(keran);
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
  String url = "/IoT_walet/add.php?";
  url += "suhu=";
  url += sensorSuhu;
  url += "&kelembaban=";
  url += sensorKelembaban;
  url += "&cahaya=";
  url += sensorCahaya;
  url += "&keran=";
  url += keran;
   
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
  // untuk suhu Normal
  if (sensorSuhu <= 25.00)
  { suhu [0] = 1;}
  else if (25.00 < sensorSuhu && sensorSuhu < 27.00)
  {  suhu [0] = (27.00 - sensorSuhu)/(27.00 - 25.00); }
  else if (sensorSuhu >= 27.00)
  { suhu [0] = 0;}
//  Serial.print("suhu functions: ");
//  Serial.println(sensorSuhu);

  // untuk suhu hampir panas / Sedang
  if (sensorSuhu <= 25.00 || sensorSuhu >= 29.00)
  { suhu [1] = 0;}
  else if (25.00 < sensorSuhu && sensorSuhu <= 27.00)
  { suhu [1] = (sensorSuhu-25.00)/(27.00-25.00);}
  else if (27.00 <= sensorSuhu && sensorSuhu < 29.00)
  { suhu [1] = (29.00 - sensorSuhu)/(29.00-27.00);}

  // untuk suhu panas / Tinggi
  if (sensorSuhu <= 27.00)
  { suhu [2] = 0;}
  else if (27.00 < sensorSuhu && sensorSuhu < 29.00)
  { suhu [2] = (sensorSuhu-27.00)/(29.00-27.00);}
  else if (sensorSuhu >= 29.00)
  { suhu [2] = 1;}
  Serial.println("suhu functions: ");
  Serial.println(suhu [0]);
  Serial.println(suhu [1]);
  Serial.println(suhu [2]);
}

void FuzzyKelembaban(float sensorKelembaban){
  // untuk kelembaban normal
  if (sensorKelembaban <= 70.00)
  { kelembaban [0] = 0;}
  else if (70.00 < sensorKelembaban && sensorKelembaban < 95.00)
  {  kelembaban [0] = (sensorKelembaban - 70.00)/(95.00 - 70.00); }
  else if (sensorKelembaban >= 95.00)
  { kelembaban [0] = 1;}

  // untuk kelembaban Sedang
  if (sensorKelembaban <= 65.00 || sensorKelembaban >= 95.00)
  { kelembaban [1] = 0;}
  else if (65.00 < sensorKelembaban && sensorKelembaban <= 70.00)
  { kelembaban [1] = (sensorKelembaban-65.00)/(70.00-65.00);}
  else if (70.00 <= sensorKelembaban && sensorKelembaban < 95.00)
  { kelembaban [1] = (95.00 - sensorKelembaban)/(95.00-70.00);}

  // untuk kelembaban tinggi
  if (sensorKelembaban <= 65.00)
  { kelembaban [2] = 1;}
  else if (65.00 < sensorKelembaban && sensorKelembaban < 70.00)
  { 
    kelembaban [2] = (70.00 - sensorKelembaban)/(70.00 - 65.00);
   }
  else if (sensorKelembaban >= 70.00)
  { kelembaban [2] = 0;}
  Serial.println("kelembaban functions: ");
  Serial.println(kelembaban[0]);
  Serial.println(kelembaban[1]);
  Serial.println(kelembaban[2]);
}

void FuzzyCahaya(float sensorCahaya){
  // untuk cahaya normal
  if (sensorCahaya <= 0.00)
  { cahaya [0] = 1;}
  else if (0 < sensorCahaya && sensorCahaya < 0.36)
  {  cahaya [0] = (0.36 - sensorCahaya)/(0.36 - 0.00); }
  else if (sensorCahaya >= 0.36)
  { cahaya [0] = 0;}

  // untuk cahaya Sedang
  if (sensorCahaya <= 0.00 || sensorCahaya >= 1.00)
  { 
    cahaya [1] = 0;
  }
  else if (0 < sensorCahaya && sensorCahaya <= 0.36)
  { 
    cahaya [1] = (sensorCahaya-0.00)/(0.36-0.00);
   }
  else if (0.36 <= sensorCahaya && sensorCahaya < 1.00)
  { 
    cahaya [1] = (1.00 - sensorCahaya)/(1.00-0.36);
  }

  // untuk cahaya tinggi
  if (sensorCahaya <= 0.36)
  { cahaya [2] = 0;}
  else if (0.36 < sensorCahaya && sensorCahaya < 1.00)
  { cahaya [2] = (sensorCahaya-0.36)/(1.00 - 0.36);}
  else if (sensorCahaya >= 1.00)
  { cahaya [2] = 1;}
  Serial.println("cahaya functions 0: ");
  Serial.println(sensorCahaya);
  Serial.println(cahaya[0]);
  Serial.println(cahaya[1]);
  Serial.println(cahaya[2]);
}


// rule dan defuzzifikasi belum dirubah
void RuleEva (){
 int i, j, c;
 for ( i=0; i<=2; i=i+1)
 {
   for ( j=0; j<=2; j=j+1)
   {
      for ( c=0; c<=2; c=c+1)
      {
         minsuhuKelem = min(suhu[i], kelembaban[j]);
         temp = min(minsuhuKelem, cahaya[c]); 
         rule [i][j][c] = temp;
      }
   } 
 } 
 rule000 = rule [0][0][0]; // (normal,normal, normal = )
 rule001 = rule [0][0][1]; // (normal,normal, sedang = )
 rule002 = rule [0][0][2]; // (normal,normal, tinggi = )

 rule010 = rule [0][1][0]; // (normal,sedang, normal = )
 rule011 = rule [0][1][1]; // (normal,sedang, sedang = )
 rule012 = rule [0][1][2]; // (normal,sedang, tinggi = )

 rule020 = rule [0][2][0]; // (normal,tinggi, normal = )
 rule021 = rule [0][2][1]; // (normal,tinggi, sedang = )
 rule022 = rule [0][2][2]; // (normal,tinggi, tinggi = )

 rule100 = rule [1][0][0]; // (sedang,normal, normal = )
 rule101 = rule [1][0][1]; // (sedang,normal, sedang = )
 rule102 = rule [1][0][2]; // (sedang,normal, tinggi = )

 rule110 = rule [1][1][0]; // (sedang,sedang, normal = )
 rule111 = rule [1][1][1]; // (sedang,sedang, sedang = )
 rule112 = rule [1][1][2]; // (sedang,sedang, tinggi = )

 rule120 = rule [1][2][0]; // (sedang,tinggi, normal = )
 rule121 = rule [1][2][1]; // (sedang,tinggi, sedang = )
 rule122 = rule [1][2][2]; // (sedang,tinggi, tinggi = )

 rule200 = rule [2][0][0]; // (tinggi,normal, normal = )
 rule201 = rule [2][0][1]; // (tinggi,normal, sedang = )
 rule202 = rule [2][0][2]; // (tinggi,normal, tinggi = )

 rule210 = rule [2][1][0]; // (tinggi,sedang, normal = )
 rule211 = rule [2][1][1]; // (tinggi,sedang, sedang = )
 rule212 = rule [2][1][2]; // (tinggi,sedang, tinggi = )

 rule220 = rule [2][2][0]; // (tinggi,tinggi, normal = )
 rule221 = rule [2][2][1]; // (tinggi,tinggi, sedang = )
 rule222 = rule [2][2][2]; // (tinggi,tinggi, tinggi = )
 
}

void Defuzzy () {
  
  // metode mamdani (weighted average)
  int y, z, c;
  int o = 1;
  for ( y=0; y<=2; y=y+1)
  {
    for ( z=0; z<=2; z=z+1)
    {
      for ( c=0; c<=2; c=c+1)
      {
        float suhuKelembaban = min(suhu[y],kelembaban[z]);
        ruleMin[o] = min(suhuKelembaban,cahaya[c]);
        o++;
      }
    } 
  }
  
  
//  ruleMin[1] = min(suhu[0],kelembaban[0],cahaya[0]);
//  ruleMin[2] = min(suhu[0],kelembaban[0],cahaya[1]);
//  ruleMin[3] = min(suhu[0],kelembaban[0],cahaya[2]);

//  ruleMin[4] = min(suhu[0],kelembaban[1],cahaya[0]);
//  ruleMin[5] = min(suhu[0],kelembaban[1],cahaya[1]);
//  ruleMin[6] = min(suhu[0],kelembaban[1],cahaya[2]);

//  ruleMin[7] = min(suhu[0],kelembaban[2],cahaya[0]);
//  ruleMin[8] = min(suhu[0],kelembaban[2],cahaya[1]);
//  ruleMin[9] = min(suhu[0],kelembaban[2],cahaya[2]);

//  ruleMin[10] = min(suhu[1],kelembaban[0],cahaya[0]);
//  ruleMin[11] = min(suhu[1],kelembaban[0],cahaya[1]);
//  ruleMin[12] = min(suhu[1],kelembaban[0],cahaya[2]);

//  ruleMin[13] = min(suhu[1],kelembaban[1],cahaya[0]);
//  ruleMin[14] = min(suhu[1],kelembaban[1],cahaya[1]);
//  ruleMin[15] = min(suhu[1],kelembaban[1],cahaya[2]);

//  ruleMin[16] = min(suhu[1],kelembaban[2],cahaya[0]);
//  ruleMin[17] = min(suhu[1],kelembaban[2],cahaya[1]);
//  ruleMin[18] = min(suhu[1],kelembaban[2],cahaya[2]);

//  ruleMin[19] = min(suhu[2],kelembaban[0],cahaya[0]);
//  ruleMin[20] = min(suhu[2],kelembaban[0],cahaya[1]);
//  ruleMin[21] = min(suhu[2],kelembaban[0],cahaya[2]);

//  ruleMin[22] = min(suhu[2],kelembaban[1],cahaya[0]);
//  ruleMin[23] = min(suhu[2],kelembaban[1],cahaya[1]);
//  ruleMin[24] = min(suhu[2],kelembaban[1],cahaya[2]);

//  ruleMin[25] = min(suhu[2],kelembaban[2],cahaya[0]);
//  ruleMin[26] = min(suhu[2],kelembaban[2],cahaya[1]);
//  ruleMin[27] = min(suhu[2],kelembaban[2],cahaya[2]);
  
  Serial.print("NNN : ");
  Serial.println(ruleMin[1]);
  Serial.print("NNS : ");
  Serial.println(ruleMin[2]);
  Serial.print("NNT : ");
  Serial.println(ruleMin[3]);
  Serial.print("NSN : ");
  Serial.println(ruleMin[4]);
  Serial.print("NSS : ");
  Serial.println(ruleMin[5]);
  Serial.print("NST : ");
  Serial.println(ruleMin[6]);
  Serial.print("NTN : ");
  Serial.println(ruleMin[7]);
  Serial.print("NTS : ");
  Serial.println(ruleMin[8]);
  Serial.print("NTT : ");
  Serial.println(ruleMin[9]);

  Serial.print("SNN : ");
  Serial.println(ruleMin[10]);
  Serial.print("SNS : ");
  Serial.println(ruleMin[11]);
  Serial.print("SNT : ");
  Serial.println(ruleMin[12]);
  Serial.print("SSN : ");
  Serial.println(ruleMin[13]);
  Serial.print("SSS : ");
  Serial.println(ruleMin[14]);
  Serial.print("SST : ");
  Serial.println(ruleMin[15]);
  Serial.print("STN : ");
  Serial.println(ruleMin[16]);
  Serial.print("STS : ");
  Serial.println(ruleMin[17]);
  Serial.print("STT : ");
  Serial.println(ruleMin[18]);

  Serial.print("TNN : ");
  Serial.println(ruleMin[19]);
  Serial.print("TNS : ");
  Serial.println(ruleMin[20]);
  Serial.print("TNT : ");
  Serial.println(ruleMin[21]);
  Serial.print("TSN : ");
  Serial.println(ruleMin[22]);
  Serial.print("TSS : ");
  Serial.println(ruleMin[23]);
  Serial.print("TST : ");
  Serial.println(ruleMin[24]);
  Serial.print("TTN : ");
  Serial.println(ruleMin[25]);
  Serial.print("TTS : ");
  Serial.println(ruleMin[26]);
  Serial.print("TTT : ");
  Serial.println(ruleMin[27]);
  
  Serial.println(sensorSuhu);
  Serial.println(sensorKelembaban);
  Serial.println(sensorCahaya);
  
   int i, j;
   float RuleMin = 2;
   char* kondisi1;
   for ( i=1; i<=27; i=i+1)
   {
       if(ruleMin[i] > 0){ // hanya yang lebih dari 0 yang diambil
        if(RuleMin > ruleMin[i]){
           RuleMin = min(RuleMin, ruleMin[i]);
         if(i == 1){
//            kondisi = "normal normal";
            kondisi1 = "stop";
         }
         else  if(i >= 2 && i < 9){
//            kondisi = "normal sedang";
            kondisi1 = "kecil";
         }
         else  if(i >= 10 && i < 15){
//            kondisi = "normal tinggi";
            kondisi1 = "kecil";
         }
         else  if(i == 16 || i == 19 || i == 20 || i == 22){
//            kondisi = "sedang normal";
            kondisi1 = "kecil";
         }
         else  if(i == 9){
//            kondisi = "sedang sedang";
            kondisi1 = "besar";
         }
         else  if(i == 15){
//            kondisi = "sedang tinggi";
            kondisi1 = "besar";
         }
         else  if(i == 17){
//            kondisi = "tinggi normal";
            kondisi1 = "besar";
         }
         else  if(i == 18){
//            kondisi = "tinggi sedang";
            kondisi1 = "besar";
         }
         else  if(i == 21){
//            kondisi = "tinggi tinggi";
            kondisi1 = "besar";
         }
         else  if(i >= 23 && i <= 27){
//            kondisi = "tinggi tinggi";
            kondisi1 = "besar";
         }
       }
          
     }
   }


   int m, n;
   float RuleMax = 0;
   char* kondisi2;
   for ( m=1; m<=27; m=m+1)
   {
       if(ruleMin[m] > 0.00 ){
         if(RuleMax < ruleMin[m]){
          RuleMax = max(RuleMax, ruleMin[m]);;
           if(m == 1){
  //          kondisi = "normal normal";
              kondisi2 = "stop";
           }
           else  if(m >= 2 && m < 9){
  //            kondisi = "normal sedang";
              kondisi2 = "kecil";
           }
           else  if(m >= 10 && m < 15){
  //            kondisi = "normal tinggi";
              kondisi2 = "kecil";
           }
           else  if(m == 16 || m == 19 || m == 20 || m == 22){
  //            kondisi = "sedang normal";
              kondisi2 = "kecil";
           }
           else  if(m == 9){
  //            kondisi = "sedang sedang";
              kondisi2 = "besar";
           }
           else  if(m == 15){
  //            kondisi = "sedang tinggi";
              kondisi2 = "besar";
           }
           else  if(m == 17){
  //            kondisi = "tinggi normal";
              kondisi2 = "besar";
           }
           else  if(m == 18){
  //            kondisi = "tinggi sedang";
              kondisi2 = "besar";
           }
           else  if(m == 21){
  //            kondisi = "tinggi tinggi";
              kondisi2 = "besar";
           }
           else  if(m >= 23 && m <= 27){
  //            kondisi = "tinggi tinggi";
              kondisi2 = "besar";
           }
         }
       }
   }

   Serial.println(kondisi1);
   Serial.println(kondisi2);
  Serial.print("rule min : ");
  Serial.println(RuleMin);
  Serial.print("rule max : ");
  Serial.println(RuleMax);

  float a1, a2;
//  a1 = (RuleMin*(90-45))+45;
//  a2 = (RuleMax*(90-45))+45;
  Serial.println(sensorSuhu);
  Serial.println(sensorKelembaban);
  Serial.println(sensorCahaya); 
  if(sensorSuhu <= 25.00 && sensorKelembaban >= 95.00 && sensorCahaya <= 0.00  ){
    Serial.println("normal normal");
      a1 = ((RuleMin*(90-45))-90)*(-1);
      a2 = (RuleMax*(90-45))+45;
  }else if( sensorSuhu >= 29.00 && sensorKelembaban <= 65.00 && sensorCahaya > 1.00){
    Serial.println("tinggi tinggi");  
      a1 = (RuleMin*(90-45))+45;
      a2 = ((RuleMax*(90-45))-90)*(-1);
  }else{
    if ( kondisi1 == "kecil" && kondisi2 == "kecil" ){
      //      =((S129*(45-0))+0)
      //      =((S129*(45-0))+0)
      Serial.println("kecil kecil");
      a1 = (RuleMin*(45-0))+0;
      a2 = (RuleMax*(45-0))+0;
    }
    else if(kondisi1 == "kecil" && kondisi2 == "besar" ){
      // maka gunakan rumus 45-90
      //      =((S129*(90-45))+45)
      //      =((S129*(90-45))+45)
       Serial.println("kecil besar"); 
      a1 = (RuleMin*(90-45))+45;
      a2 = (RuleMin*(90-45))+45;
    }
    else if(kondisi1 == "besar" && kondisi2 == "kecil" ){
      // maka gunakan rumus 0-45
      //      =((S129*(45-0))-45)*-1
      //      =((S129*(45-0))-45)*-1
      Serial.println("besar kecil"); 
      a1 = ((RuleMax*(45-0))-45)*(-1);
      a2 = ((RuleMax*(45-0))-45)*(-1);
    }
    else if(kondisi1 == "besar" && kondisi2 == "besar" ){
      // maka gunakan rumus 45-90
      //      =((S129*(90-45))-90)*-1
      //      =((S129*(90-45))-90)*-1
      Serial.println("besar besar"); 
      a1 = ((RuleMax*(90-45))-90)*(-1);
      a2 = ((RuleMax*(90-45))-90)*(-1);
    }
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
//  defuzzy = (m1+m2+m3)/(luasArea_A1+luasArea_A2+luasArea_A3);
  if (defuzzy <= 0) {
      keran = 0;
  }else if(defuzzy > 90){
      keran = 90;
  }else{
    keran = defuzzy;
  }
  keran = 0;
  Serial.print("defuzzy : ");
  Serial.println(keran);
}
