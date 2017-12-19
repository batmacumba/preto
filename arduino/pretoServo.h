
/*
  "pretoServo.h"
  Servo functions and variables
*/

#ifndef pretoServo_H_
#define pretoServo_H_

#include <Servo.h>

#define UNUSED(x) (void)(x)                 // Avoids unused variable warning

/* Servo_Settings will hold all the servo related variables that must be stored */
struct Servo_Settings {
  unsigned int ShutterClosed;                 // CLOSED position
  unsigned int ShutterOpen;                   // OPEN position
  unsigned int ServoPin;                      // Connect the servo's orange cable to this pin
  unsigned int mem_addr;                      // This struct's own address on the EEPROM
};

/* Global variables */
Servo_Settings Serv;
Servo shutterServomoteur;                     // servo object        
unsigned int positionShutter;                 // holds the current position           

void testServo() {
  /* def: test movement */
  Serial.println("testServo()");
  shutterServomoteur.write(Serv.ShutterOpen);
  delay(500);
  shutterServomoteur.write(Serv.ShutterClosed);
  delay(2000);
  shutterServomoteur.write(Serv.ShutterOpen);
  /* updates its current position */
  positionShutter = Serv.ShutterOpen;
  Serial.println("Ok");
}

void initServo(int settings_addr) {
  /* def: servo initialization and test. You should see the arm swing once at boot */
  /* reads the closed and open values from EEPROM */
  Serial.println("initServo()");
  EEPROM.get(settings_addr, Serv);
  Serial.println("ShutterClosed = " + Serv.ShutterClosed);
  Serial.println("ShutterOpen = " + Serv.ShutterOpen);
  Serial.println("ServoPin = " + Serv.ServoPin);
  Serial.println("mem_addr = " + Serv.mem_addr);
  
  /* attach to the pin defined above */
  shutterServomoteur.attach(Serv.ServoPin);
  Serial.println("Ok");
  
  testServo();
}

void openShutter(OSCMessage &msg, int addrOffset) {
  /* def: opens the shutter */
  msg.empty();
  UNUSED(addrOffset);

  /* checks if the shutter is not open already */
  if (positionShutter != Serv.ShutterOpen) {
    /* opens it and updates its current position */
    shutterServomoteur.write(Serv.ShutterOpen);
    positionShutter = Serv.ShutterOpen;
    Serial.println("Shutter open");
  }
}

void closeShutter(OSCMessage &msg, int addrOffset) {
  /* def: closes the shutter */
  msg.empty();
  UNUSED(addrOffset);

  /* checks if the shutter is not closed already */
  if (positionShutter != Serv.ShutterClosed) {
    /* closes it and updates its current position */
    shutterServomoteur.write(Serv.ShutterClosed);
    positionShutter = Serv.ShutterClosed;
    Serial.println("Shutter closed");
  }
}

void moveShutter(OSCMessage &msg, int addrOffset) {
  /* def: moves the shutter to the argument given in the OSC message */
  UNUSED(addrOffset);

  /* message validation */
  /* float processing for compatibility with touchOSC */
  if (msg.isFloat(0)) {
    float f = msg.getFloat(0);
    unsigned int angle = (int) f;
    msg.empty();
    msg.add(angle);
  }
  if (msg.isInt(0) && msg.getInt(0) >= 0 && msg.getInt(0) <= 180) {
    positionShutter = msg.getInt(0);
    shutterServomoteur.write(positionShutter);
  }
  /* reports back its position to the serial */
  Serial.println(positionShutter); 
  /* clears the msg buffer */
  msg.empty();
}

void setClosed (OSCMessage &msg, int addrOffset) {
  /* def: sets the current servo position as the CLOSED position */
  UNUSED(addrOffset);

  /* updates the ShutterClosed variable */
  Serv.ShutterClosed = positionShutter;
  /* saves it to the EEPROM so it will be remembered after reboot */
  EEPROM.put(Serv.mem_addr, Serv);
  Serial.println(Serv.ShutterClosed);
  /* clears the msg buffer */
  msg.empty();
}

void setOpen (OSCMessage &msg, int addrOffset) {
  /* def: sets the current servo position as the OPEN position */
  UNUSED(addrOffset);

  /* updates the ShutterOpen variable */
  Serv.ShutterOpen = positionShutter;
  /* saves it to the EEPROM so it will be remembered after reboot */
  EEPROM.put(Serv.mem_addr, Serv);
  Serial.println(Serv.ShutterOpen);
  /* clears the msg buffer */
  msg.empty();
}

void ServoPin(OSCMessage &msg, int addrOffset) {
  /* def: sets the digital pin which will be used for the servo */
  UNUSED(addrOffset);

  /* message validation */
  if (msg.isInt(0) && msg.getInt(0) >= 2 && msg.getInt(0) <= 26) {
    /* updates the ServoPin variable */
    Serv.ServoPin = msg.getInt(0);
    /* saves it to the EEPROM so it will be remembered after reboot */
    EEPROM.put(Serv.mem_addr, Serv);
  }
  /* reports via serial */
  Serial.println("ServoPin = " + Serv.ServoPin); 
  /* clears the msg buffer */
  msg.empty();
}

#endif /* pretoServo_H_ */
