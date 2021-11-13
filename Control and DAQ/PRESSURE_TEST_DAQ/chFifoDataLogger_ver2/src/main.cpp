#include <arduino.h>
#include <SD.h>
#include <servo.h>

const int Pin_button = 1;  
const int Pin_PT = 22; //PT3 is 22, PT2 is 21, PT1 is 20
const int Count = 50;
const int FlushPeriod = 100;

Servo myservo1;
Servo myservo2;
const int BallServo1 = 5;
const int BallServo2 = 4;

const int LED1=6,LED2=7,LED3=8,LED4=9;

int FlushCounter = 0;

const float a = 0.26115772421664074;
const float b = -395.4737740445223;

int system_state = 0;
int i = 0;

int buttonState = 0;

File file; // For saving data into SD card file.

// SD file definitions.
const uint8_t sdChipSelect = BUILTIN_SDCARD;

void setup(){

  Serial.begin(9600);

  while(!Serial){} // Wait for Serial initialization

  pinMode(Pin_button,INPUT); // Pin Modesetup
  pinMode(Pin_PT, INPUT); // Pressure pin

  pinMode(LED_BUILTIN, OUTPUT); // LED PIN // 6 7 8 9 
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  analogReadResolution(12);

  // SD card initialization
  
  // Open file.
  if (!SD.begin(sdChipSelect)) {
    Serial.println(F("SD begin failed."));
    while (true) {}
  }
  file = SD.open("0218B.CSV", O_CREAT | O_WRITE | O_TRUNC);
  if (!file) {
    
    Serial.println(F("file open failed."));
    while (true) {}
  }
  

  // Comment:
  // It seems safer to connect Pin_button with Ground first when starting up.
  // USB connections seem to provide Ground, but without USB its hard to say...
  // DON'T TOUCH ON THE CONNECTION (Sometimes I am charged)!

  Serial.printf("Setup Completed\n");
  digitalWrite(LED_BUILTIN, HIGH);

  //Attach myservo to correct pin
  myservo1.attach(BallServo1);
  myservo2.attach(BallServo2);
  myservo1.write(0);
  myservo2.write(0);
}



void loop(){
    // Read the volt and convert to pressure
    float voltage = analogRead(Pin_PT);
    float pressure = a*voltage*2 + b;

    //if(pressure<0) pressure = 0.0;

    // Get timestamp
    int timestamp = micros();

    Serial.print(timestamp);
    Serial.print("\t");
    Serial.print(pressure);
    Serial.print("\t");
    Serial.print("0");
    Serial.print("\t");
    Serial.print("0");
    Serial.print("\t\n");
    


    file.print(timestamp);
    file.write(',');
    file.print(pressure);
    file.println();

    if(FlushCounter >= FlushPeriod){
      file.flush();
      FlushCounter = 0;
      digitalWrite(LED1, HIGH);
    }
    else{
      FlushCounter += 1;
      digitalWrite(LED1, LOW);
    }


    if(Serial.available()){
      char check = Serial.read();
      if(check == 'a'){
        myservo1.write(105);
        myservo2.write(105);
        //Serial.print(check);
        digitalWrite(LED3, HIGH);

      }
      if(check == 'b'){
        myservo1.write(0);
        myservo2.write(0);
        //Serial.print(check);
        digitalWrite(LED3, LOW);
      }
    }
  

    // Serial.printf("100\t100\t100\t100\t\n");
    // Delay for sometime
    delay(100);
}

