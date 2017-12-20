
/*
  "Preto"
  Arduino shutter for video projectors controllable via OSC

  Default settings:
    Shutter IP:   192.168.0.99
    Master IP:    192.168.0.40
    MAC address:  XX:XX:XX:XX:XX:XX (random)
    Closed angle: 45
    Open angle:   135
    Servo pin:    9

  OSC commands: 
    /close            Closes the shutter
    /open             Opens the shutter
    /position         Sends a OSC message to the master with the servo's current angle
    /reboot           Reboots the shutter
    /move %d          If the argument is an integer between 0 and 180, moves the servo to that position
    /servo_pin %d     Sets the servo pin
    /test             Tests the servo
    /shutter_ip %d    Sets its own IP and reboots
    /master_ip %d     Sets the master's IP
    /shutter_port %d  Sets the shutter's listening port
    /master_port %d   Sets the master's listening port
    /set_closed       Assigns the current servo angle to the CLOSE command
    /set_open         Assigns the current servo angle to the OPEN command
    /reset_config     Resets the board to the default configuration in case anything goes wrong
    /ping             Sends the master a /pong reply   
  
 */
#define CONFIG_BYTE 0     // EEPROM position which will store the "mark"
#define MARKED 69         // Are you experienced?

/* Dependencies */
#include <Ethernet.h>      
#include <OSCBundle.h>     
#include <EthernetUdp.h>   
#include <Servo.h>         
#include <EEPROM.h>
/* Includes */
#include "preto.h"

/* Global variable */
commandMode selectedMode;

void setup() {
  Serial.begin(9600);

  /* Returns the current command mode. Currently only OSC is available */
  selectedMode = findCommandMode();

  /* If this board is not marked as "configured", runs the setup function */
  if (EEPROM.read(CONFIG_BYTE) != MARKED) {
    setupShutter();
  }
  /* If this board has been previously configured, starts operation */
  else if (EEPROM.read(CONFIG_BYTE) == MARKED) {
    initOSC(1);
    initServo(sizeof(Net_Settings) + 1);
  }
}

void loop() {
  /* Only OSC is available for now */
  switch (selectedMode) {
    case osc:
      receiveOSC();
      break;
    case dmx :
      break;
    case manual :
      break;
  }
}
