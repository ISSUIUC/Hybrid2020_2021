/*
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Arduino pin 6 -> HX711 CLK
 Arduino pin 5 -> HX711 DOUT
 Arduino pin 5V -> HX711 VCC
 Arduino pin GND -> HX711 GND 
*/

#include "HX711.h"
#include <Servo.h>

Servo s1, s2;

HX711 scale(4, 5);

const float a = 0.26115772421664074;
const float b = -395.4737740445223;
constexpr int OPEN_DEGREES = 140;
constexpr int CLOSE_DEGREES = 0;

float calibration_factor = -2.381; // this calibration factor is adjusted according to my load cell
float units;
float ounces;

void setup() {
  Serial.begin(9600);
  
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  s1.attach(7);
  s2.attach(8);
  s1.write(CLOSE_DEGREES);
  s2.write(CLOSE_DEGREES);
//  Serial.println("HX711 calibration sketch");
//  Serial.println("Remove all weight from scale");
//  Serial.println("After readings begin, place known weight on scale");
//  Serial.println("Press + or a to increase calibration factor");
//  Serial.println("Press - or z to decrease calibration factor");

  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
//  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
//  Serial.println(zero_factor);
}

void loop() {
  delay(10);
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

//  Serial.print("Reading: ");
  units = scale.get_units(), 10;
  if (units < 0)
  {
    units = 0.00;
  } 
  ounces = units * 0.035274;

  float voltage1 = analogRead(A0);
  float pressure1 = voltage1*8*a + b;
  float voltage2 = analogRead(A1);
  float pressure2 = voltage2*8*a + b;
  float voltage3 = analogRead(A2);
  float pressure3 = voltage3*8*a + b;
  Serial.print(millis());
  Serial.print('\t');
  Serial.print(units);
  Serial.print('\t');
  Serial.print(pressure1);
  Serial.print('\t');
  Serial.print(pressure2);
//  Serial.print('\t');
//  Serial.print(pressure3);
  Serial.println();
//  Serial.print(" grams"); 
//  Serial.print(" calibration_factor: ");
//  Serial.print(calibration_factor);
 

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == 'a'){
      s1.write(CLOSE_DEGREES);
      s2.write(CLOSE_DEGREES);
    } else if(temp == 'b'){
      s1.write(OPEN_DEGREES);
      s2.write(OPEN_DEGREES);
    }
  }
}