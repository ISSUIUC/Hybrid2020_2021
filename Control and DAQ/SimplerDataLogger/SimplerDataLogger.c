'''Although originally I was going to adapt the previous script and remove
the different threads and stuff, I found an Arduino guide built exactly for 
data from 3 analog pins and writing it onto the SD card. The only difference might be
with how the Teensy connects to the SD. --Anshuk
Here is the Arduino reference page:
https://www.arduino.cc/en/Tutorial/Datalogger
''' 

#include <SD.h>

// SD file definitions.
const uint8_t sdChipSelect = BUILTIN_SDCARD; //This might be different based on how the Teensy SD is.

int c = 0; //Counter for batches of data to be sent
size = 100; //Size of each batch of data
// make a string for assembling the data to log:
String dataString = "";

void setup() {
    //Open serial communication and wait for port to open
    Serial.begin(9600);
    // wait for Serial Monitor to connect. Needed for native USB port boards only:
    while (!Serial);

    Serial.print("Initializing SD card...");
    //If the SD card doesn't get initialized:
    if (!SD.begin(sdChipSelect)) {
    Serial.println(F("SD begin failed."));
    while (true);
    }

    Serial.println("initialization done."); //SD card connected
}

void loop() {

    // read three sensors and append to the string. Change based on the analogPins used:
    for (int analogPin = 0; analogPin < 3; analogPin++) { 
        int sensor = analogRead(analogPin);
        dataString += String(sensor);
        dataString += ",";
        c++;
    }

    if (c == size) {
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        File dataFile = SD.open("datalog.csv", FILE_WRITE);

        // if the file is available, write to it:
        if (dataFile) {
            dataFile.println(dataString);
            dataFile.close();
            // print to the serial port too:
            Serial.println(dataString);
        }
        // if the file isn't open, pop up an error:
        else {
            Serial.println("error opening datalog.csv");
        }
        c = 0; //For the next batch
        dataString = ""; //For the next batch

        //Add a delay somewhere to change the read-rate?
    }
}   