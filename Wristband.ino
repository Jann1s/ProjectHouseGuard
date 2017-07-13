// TMRpcm - Version: Latest 
#include <TMRpcm.h>
#include <pcmConfig.h>
#include <pcmRF.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#define SD_ChipSelectPin 4  //CS Pin of SDCard Module
#define BUTTON_PRESSED LOW   
#define BUTTON_PRESS_TIME 1000      //~5 seconds at 5V

SoftwareSerial BTserial(5,6); //10,11 TX | RX

//-----------
//GLOBAL VARS
//-----------
bool sleepMode;
TMRpcm tmrpcm;


//ANALOG PINS

//DIGITAL PINS
const int pinButton = 3;

char tempChar;
char alarmCode;
char roomNumber;
//indicate whether still waiting for room msg
bool waitRoom;

bool buttonPressed;
int buttonPressCount;
int songQueue;
//if button is not pressed after a certain amount
bool notResponding;

//when the alarms begin 
unsigned long millisStart;
//current time 
unsigned long millisNow;
//interval time between alarms starting and current time 
unsigned long interval;

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT); //Setting up internal led as sleep mode indicator
  digitalWrite(LED_BUILTIN, LOW);
  waitRoom = false;
  notResponding = false;
  interval = 10000;
  
  pinMode(pinButton, INPUT);
  digitalWrite(pinButton, HIGH);
  buttonPressCount = -1;
  songQueue = -1;
  
  sleepMode = false;
  Serial.begin(9600);
  
  Serial.println("Started!");
  setupSDCard();
  setupSpeaker();
  setupBluetooth();   
}

void loop() {
	
  //cheeck if button was pressed
  checkButton(); 
    
  //wait for bluetooth to receige msgs
  if(BTserial.available()){   
    
    //read bluetooth messages
    tempChar = BTserial.read();
    
    //if true, set the room number and proceed
    if (waitRoom) {
      roomNumber = tempChar;
      songQueue = 0;
      waitRoom = false;
      millisStart = millis();
    }
    
    //depending on code, set the alarm code
    if (((tempChar == 'f' || tempChar == 'p') && !sleepMode) || (sleepMode && tempChar == 'f')) {
      alarmCode = tempChar;
      waitRoom = true;
    }
  }
  
  //if a sound is being played now and user not responding to alarm
  //send back sms code to base station
  if (songQueue >= 0 && !notResponding) {
    millisNow = millis();
    
    if ((millisNow - millisStart) > interval) {
      BTserial.write("b");
      notResponding = true;
    }
  }
  
  //depending on the message and room, play the appropriate sound
  if (!tmrpcm.isPlaying() && songQueue >= 0) {  
    
    //if first audio file is playing
	  //depending on the message received
	  //play the next room sound accordingly
    if (songQueue == 0) {
      
      if (alarmCode == 'f') {
        tmrpcm.play("f.wav");
        songQueue = 1;
      }
      else if (alarmCode == 'p') {
        tmrpcm.play("p.wav");
      }
    }
    else if (songQueue == 1) {
      if (roomNumber == '1') {
        tmrpcm.play("1.wav");
      }
      else if (roomNumber == '2') {
        tmrpcm.play("2.wav");
      }
      else if (roomNumber == '3') {
        tmrpcm.play("3.wav");
      }

      songQueue = 0;
    }
  }
}

/*
* SETUP METHODS
*/
//Method for setting up Bluetooth
void setupBluetooth() 
{
  BTserial.begin(38400);
}

//Method for setting up Audio
void setupSpeaker() {
  tmrpcm.speakerPin = 9;
  tmrpcm.setVolume(5);
  tmrpcm.quality(1);
  tmrpcm.loop(0);
  tmrpcm.play("f.wav");
}

//Method for setting up SDCard
void setupSDCard() 
{
  if (!SD.begin(SD_ChipSelectPin)) 
  {
    Serial.println("SD fail");
    return;
  }
}

//Start vibration
void setVibration(bool start) 
{
  //The vibe board unfortunatly never arrived.
}

//if button is pressed once, stop the alarm
//if button is pressed twice, enter sleep mode
void checkButton()
{
  int buttonState = digitalRead(pinButton);
  
  if (buttonState == LOW) {
    buttonPressed = true;
  }
  else {
    buttonPressed = false;
    if (buttonPressCount == -1) {
      buttonPressCount = -1;
    }
    else if (buttonPressCount == -2 && buttonState == HIGH) {
      buttonPressCount = -1;
    }
  }
  
  //if button is pressed once
  if (!buttonPressed && buttonPressCount >= 0 && buttonPressCount < BUTTON_PRESS_TIME) {
    BTserial.write("k");
    tmrpcm.disable();
    songQueue = -1;
    buttonPressCount = -2;
  }  
  
  //if button is pressed twice
  if (buttonState == LOW && buttonPressed && buttonPressCount >= -1) {
    buttonPressCount++;
    Serial.println(buttonPressCount);
    
    //Activate / Deactivate sleep mode
    if (buttonPressCount > BUTTON_PRESS_TIME) {
      sleepMode = !sleepMode;
      
      //if sleepmode is on, disable sounds and notify user 
      if (sleepMode) 
      {
        if (tmrpcm.isPlaying()) 
        {
          tmrpcm.disable();
        }
        
        tmrpcm.play("sleepon.wav");
      }
      //enable alarms and disable sleep mode
      else 
      {
        if (tmrpcm.isPlaying()) 
        {
          tmrpcm.disable();
        }
        
        tmrpcm.play("sleepoff.wav");
      }
      
      //reset button press count
      buttonPressCount = -2;
    }
  }  
}