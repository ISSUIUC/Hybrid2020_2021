#include <arduino.h>


const int Pin_button = 1;  
const int Pin_PT = 22; 
const int Count = 50;

const float a = 0.26115772421664074;
const float b = -395.4737740445223;

int system_state = 0;
int i = 0;

int buttonState = 0;

float pressure = -999.9;
float voltage = -999.9;

void setup(){

  Serial.begin(9600);

  while(!Serial){} // Wait for Serial initialization

  pinMode(Pin_button,INPUT); // Pin Modesetup
  pinMode(Pin_PT, INPUT); // Pressure pin

  analogReadResolution(13);
  

  // Comment:
  // It seems safer to connect Pin_button with Ground first when starting up.
  // USB connections seem to provide Ground, but without USB its hard to say...
  // DON'T TOUCH ON THE CONNECTION (Sometimes I am charged)!

  Serial.printf("Setup Completed\n");
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
    
    // Read the vol
    voltage = analogRead(Pin_PT);
    pressure = a*voltage + b;

    if(pressure<0) pressure = 0.0;

    Serial.printf("100\t%f\t0\t0\t\n",pressure);
    // Serial.printf("100\t100\t100\t100\t\n");
    // Delay for sometime.
    delay(100);
}

