#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <AccelStepper.h>

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
int x = 0;
int y = 0;
int z = 0;
// variables to hold averaged values
int xAverage = 0;
int yAverage = 0;
int zAverage = 0;

// array to hold parsed data
int cmd_str[] = {x, y, z};

boolean newData = false; // switched when string terminated by end marker

#define PIN 1 // left stepper - step
#define PIN 2 // left stepper - dir
#define PIN 5 // ring 4
#define PIN 6 // ring 3
#define PIN 10 // ring 2
#define PIN 11 // ring 1
#define PIN 12 // right stepper - step
#define PIN 13 // right stepper - dir

// set up rings
Adafruit_NeoPixel stripA = Adafruit_NeoPixel(16, 5, NEO_GRB); 
Adafruit_NeoPixel stripB = Adafruit_NeoPixel(23, 6, NEO_GRB);
Adafruit_NeoPixel stripC = Adafruit_NeoPixel(31, 10, NEO_GRB);
Adafruit_NeoPixel stripD = Adafruit_NeoPixel(38, 11, NEO_GRB); 

// set up steppers
//AccelStepper stepLeft(1, 1, 2); 
//AccelStepper stepRight(1, 12, 13);

const int num_pins = 8;
const int pin_list[] = {2, 3, 5, 6, 10, 11, 12, 13};

void setup()
{
  Serial.begin(9600);  // start serial port
  delay(100);

  // setup pins from PIN_LIST as outputs
  for (int i = 0; i < num_pins; i++) {
    pinMode(pin_list[i], OUTPUT);
  }

  // force initialize LEDs on startup
  for (int i = 0; i < 11; i++) {
    stripA.setPixelColor(i, 0, 0, 0);
    stripA.show();
  }
  for (int i = 0; i < 23; i++) {
    stripB.setPixelColor(i, 0, 0, 0);
    stripB.show();
  }
  for (int i = 0; i < 31; i++) {
    stripC.setPixelColor(i, 0, 0, 0);
    stripC.show();
  }
  for (int i = 0; i < 38; i++) {
    stripD.setPixelColor(i, 0, 0, 0);
    stripD.show();
  }
}

void loop() {
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    //showParsedData();
    lightLeds();    
    stepLeft.run(); // accelstepper requires run function to be called repeatedly
    stepRight.run();
    newData = false; // return to default newData state
  }
}

  // ==============
  // removed stepper motor code as was causing minor serial data conflicts, and drivers not working
  // ==============

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) { // while serial available, discover start/end markers and process string
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {     // split the data into its parts and write to variables
  char * strtokIndx;

  strtokIndx = strtok(tempChars, ","); // discover data separated by commas
  x = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  y = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  z = atoi(strtokIndx);
}

void showParsedData() { // not possible to view when serial port is open

  Serial.print("X ");
  Serial.println(x);
  Serial.print("Y ");
  Serial.println(y);
  Serial.print("Z ");
  Serial.println(z);
  
}

void lightLeds() { // huge amount of customisation possibilities here

  // update array with new variable values
  cmd_str[0] = x; 
  xAverage = (xAverage * 9 + x ) / 10; // smooth out value changes and major LED flicker by finding running average
  cmd_str[1] = y;
  yAverage = (yAverage * 9 + y ) / 10;
  cmd_str[2] = z;
  zAverage = (zAverage * 9 + z ) / 10;

  for (int i = 0; i < 16; i++) {
    stripA.setPixelColor(i, cmd_str[0], cmd_str[1], cmd_str[2]); // map LED rings based on updated cmd_str array
    stripA.show();
  }
  for (int i = 0; i < 23; i++) {
    stripB.setPixelColor(i, cmd_str[0], cmd_str[1], cmd_str[2]);
    stripB.show();
  }
  for (int i = 0; i < 31; i++) {
    stripC.setPixelColor(i, cmd_str[2], cmd_str[1], cmd_str[0]);
    stripC.show();
  }
  for (int i = 0; i < 38; i++) {
    stripD.setPixelColor(i, cmd_str[2], cmd_str[1], cmd_str[0]);
    stripD.show();
  }
  delay(10); // allow colour to settle before update - reduces flicker
}

