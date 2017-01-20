/*
 * Camera globals
 * 
 */
#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>  
#define chipSelect 4

// On Uno: camera TX connected to pin 2, camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(2, 3);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

/*
 * PIR sensor tester
 */
 
int ledPin = 13;                // choose the pin for the LED
int inputPin = 6;               // choose the input pin (for PIR sensor)
int powerPin = 7;               // Power Pin
int transistorPin = 9;          // Power pin for transistor
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

void setupCamera() {
  pinMode(4, OUTPUT); // SS on Uno, etc.
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
//  cam.setImageSize(VC0706_640x480);        // biggest
  cam.setImageSize(VC0706_320x240);        // medium
//  cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");
}

void takePicture() {
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Create an image with the name IMAGExx.JPG
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
//      Serial.print('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); Serial.println(" ms elapsed");
  if(cam.reset()) {
    Serial.println("Reset camera");
  } else {
    Serial.println("Camera reset failed");
  }
}

void setup() {
  pinMode(powerPin, HIGH);
  digitalWrite(powerPin, HIGH);
  pinMode(transistorPin, OUTPUT);
  digitalWrite(transistorPin, LOW);
  
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
 
  Serial.begin(9600);
  setupCamera();
}

void loop(){
  val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    // we have just turned on

    // Turn on power to transistor below:\
    digitalWrite(transistorPin, HIGH);
    delay(100);
    
    Serial.println("Taking picture\n\nBe impressed!");
    takePicture();

    delay(100);
    digitalWrite(transistorPin, LOW);
  }
}
