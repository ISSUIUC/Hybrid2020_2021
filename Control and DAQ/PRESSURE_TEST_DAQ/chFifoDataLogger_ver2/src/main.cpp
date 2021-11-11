#include <arduino.h>
#include <SD.h>

const int Pin_button = 1;  
const int Pin_PT = 22; 
const int Count = 50;
const int FlushPeriod = 100;

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

  analogReadResolution(13);

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
}



void loop(){



    buttonState = digitalRead(Pin_button); // Acquire pin in every loop

    // Serial.printf("%d\n",buttonState); // Used to keep track of the pin status

    if (buttonState == HIGH && i<=Count){

      // Add a count, accumulate to certain value before activate the state.
      i+=1; 

    }
    else if (buttonState == HIGH && i>Count)
    {

      // Activate system if count is enough, and keep it.
      system_state = 1; 
      // Serial.printf("Active!\n");

    }
    else{

      // Reset in other cases.
      system_state = 0;
      i = 0;

    }

    if(system_state) digitalWrite(LED2, HIGH);
    
    // Read the volt and convert to pressure
    float voltage = analogRead(Pin_PT);
    float pressure = a*voltage + b;

    if(pressure<0) pressure = 0.0;

    // Get timestamp
    int timestamp = micros();

    Serial.printf("%d\t%f\t0\t0\t\n",timestamp,pressure);

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
  


    // Serial.printf("100\t100\t100\t100\t\n");
    // Delay for sometime.
    delay(100);
}

