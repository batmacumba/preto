
/*
  "pretoOSC.h"
  OSC functions and variables

  Dependencies: https://github.com/CNMAT/OSC
*/

#ifndef pretoOSC_H_
#define pretoOSC_H_

#define UNUSED(x) (void)(x) // Avoids unused variable warning

/* Dependencies */
#include <Ethernet.h> 
#include <EthernetUdp.h> 
#include <OSCBundle.h> 
#include <avr/wdt.h>
/* Includes */ 
#include "pretoServo.h" 

/* Net_settings struct holds all the data we'll need to store and recall network settings */
struct Net_Settings {
  byte mac[6];                          // MAC address
  byte shutter_ip[4];                   // Shutter IP address
  byte master_ip[4];                    // Master IP address
  unsigned int shutter_port;            // Port which will be used to listen to incoming messages
  unsigned int master_port;             // Port which will be used to send messages to the master
  unsigned int mem_addr;                // This struct's own address on the EEPROM
};

/* Global variables */
EthernetUDP Udp;
Net_Settings Net;

void initOSC(int settings_addr) {
  /* def: reads network settings data from the EEPROM and starts the ethernet based on that information */
  Serial.println("initOSC()...");
  /* reads the data from EEPROM */
  EEPROM.get(settings_addr, Net);
  IPAddress master_ip(Net.master_ip);

  /* starts the ethernet */
  Ethernet.begin(Net.mac, Net.shutter_ip);
  Udp.begin(Net.shutter_port);

  /* reports its IP via serial */
  Serial.println("Ok");
  Serial.print("Shutter IP Address:");
  Serial.println(Ethernet.localIP());
  Serial.print("Shutter Port:");
  Serial.println(Net.shutter_port);
  Serial.print("Master IP Address:");
  Serial.println(master_ip);
  Serial.print("Master Port:");
  Serial.println(Net.master_port);
}

void rebootShutter(OSCMessage &msg, int addrOffset) {
  /* def: reboots the board */
  UNUSED(addrOffset);
  msg.empty();
  /* reporting via serial */
  Serial.println("Rebooting in 1s...");
  /* reboots */
  wdt_enable(WDTO_1S);
  while (1) {}
}

void ShutterIP (OSCMessage &msg, int addrOffset) {
  /* def: sets the shutter IP and reboots */
  UNUSED(addrOffset);
  /* validation of message */
  if (msg.isInt(0) && msg.getInt(0) >= 0 && msg.getInt(0) <= 255) {
    /* updates the Net_settings struct and saves it on the EEPROM */
    Net.shutter_ip[3] = msg.getInt(0);
    EEPROM.put(Net.mem_addr, Net);
    /* serial report */
    Serial.println("Shutter IP Address set to 192.168.0." + Net.shutter_ip[3]);
    /* reboots */
    Serial.println("Rebooting...");
    rebootShutter(msg, addrOffset);
  }
}

void MasterIP (OSCMessage &msg, int addrOffset) {
  /* def: sets the master IP */
  UNUSED(addrOffset);
  
  /* validation of message */
  if (msg.isInt(0) && msg.getInt(0) >= 0 && msg.getInt(0) <= 255) {
    /* updates the Net_settings struct and saves it on the EEPROM */
    Net.master_ip[3] = msg.getInt(0);
    EEPROM.put(Net.mem_addr, Net);
    /* serial report */
    Serial.print("Master IP Address set to 192.168.0.");
    Serial.println(Net.master_ip[3]);
    /* clears the msg buffer */
    msg.empty();
  }
}

void ShutterPort (OSCMessage &msg, int addrOffset) {
  /* def: sets the shutter listening port and reboots */
  UNUSED(addrOffset);
  /* validation of message */
  if (msg.isInt(0) && msg.getInt(0) >= 0 && msg.getInt(0) <= 65535) {
    /* updates the Net_settings struct and saves it on the EEPROM */
    Net.shutter_port = msg.getInt(0);
    EEPROM.put(Net.mem_addr, Net);
    /* serial report */
    Serial.println("Shutter port set to" + Net.shutter_port);
    /* reboots */
    Serial.println("Rebooting...");
    rebootShutter(msg, addrOffset);
  }
}

void MasterPort (OSCMessage &msg, int addrOffset) {
  /* def: sets the master's listening port */
  UNUSED(addrOffset);
  /* validation of message */
  if (msg.isInt(0) && msg.getInt(0) >= 0 && msg.getInt(0) <= 65535) {
    /* updates the Net_settings struct and saves it on the EEPROM */
    Net.master_port = msg.getInt(0);
    EEPROM.put(Net.mem_addr, Net);
    /* serial report */
    Serial.println("Master port set to" + Net.master_port);
    /* clears the msg buffer */
    msg.empty();
  }
}

void getPositionShutter(OSCMessage &msg, int addrOffset) {
  /* def: sends the master the current servo angle */
  UNUSED(addrOffset);

  /* prepares the OSC packet */
  msg.empty();
  msg.add("/position");
  msg.add(positionShutter);
  /* reports via serial */
  Serial.print("/position ");
  Serial.println(positionShutter);
  /* sends the packet */
  Udp.beginPacket(Net.master_ip, Net.master_port); 
  msg.send(Udp); 
  Udp.endPacket(); 
  msg.empty(); 
}

void Pong(OSCMessage &msg, int addrOffset) {
  /* def: sends the master a pong reply */
  UNUSED(addrOffset);

  /* prepares the OSC packet */
  msg.empty();
  msg.add("/pong");
  /* reports via serial */
  Serial.print("/pong ");
  /* sends the packet */
  Udp.beginPacket(Net.master_ip, Net.master_port); 
  msg.send(Udp); 
  Udp.endPacket(); 
  msg.empty(); 
}

void resetConfig(OSCMessage &msg, int addrOffset) {
  /* def: resets the configuration to the default values */
  UNUSED(addrOffset);
  Net_Settings net_setup;
  Servo_Settings serv_setup;
  
  Serial.println("resetConfig()...");
  /* Marks the config byte as not configured */
  EEPROM.write(CONFIG_BYTE, 0);
  
  /* reboots */
  Serial.println("Rebooting...");
  rebootShutter(msg, addrOffset);
}

void setupShutter() {
  /* def: configures the board to be used as a shutter the first time you run it */
  Net_Settings net_setup;
  Servo_Settings serv_setup;
  
  Serial.println("setupShutter()...");
  /* Clears EEPROM memory */
  for (int i = 0 ; i < EEPROM.length() ; i++) EEPROM.write(i, 0);
  Serial.println("EEPROM cleared");
  /* Generates "random" MAC address */
  net_setup.mac[0] = 0xDE; net_setup.mac[1] = 0xAD; net_setup.mac[5] = 0x00;
  randomSeed(analogRead(0));
  for (int i = 2; i < 5; i++) net_setup.mac[i] = random(0, 255);
  Serial.println("Random MAC address generated");
  
  /* sets the default IP values */
  net_setup.shutter_ip[0] = 192; net_setup.shutter_ip[1] = 168; net_setup.shutter_ip[2] = 0; net_setup.shutter_ip[3] = 99;
  net_setup.master_ip[0] = 192; net_setup.master_ip[1] = 168; net_setup.master_ip[2] = 0; net_setup.master_ip[3] = 40;
  /* sets the default port values */
  net_setup.shutter_port = 8000;
  net_setup.master_port = 9000;
  net_setup.mem_addr = 1;
  /* stores everything in the EEPROM for recalling after reboot */
  EEPROM.put(1, net_setup);
  Serial.println("Network settings stored");

  /* sets the CLOSED, OPEN and ServoPin default values and stores it in the EEPROM */
  serv_setup.ShutterClosed = 45;
  serv_setup.ShutterOpen = 135;
  serv_setup.ServoPin = 9;
  serv_setup.mem_addr = sizeof(Net_Settings) + 1;
  EEPROM.put(sizeof(Net_Settings) + 1, serv_setup);
  Serial.println("Servo settings stored");
  
  /* Marks the config byte as "configured" */
  EEPROM.write(CONFIG_BYTE, MARKED);
  Serial.println("Board configured!");
  /* reboots */
  Serial.println("Rebooting in 1s...");
  wdt_enable(WDTO_1S);
  while (1) {}
}

void receiveOSC() {
  /* def: general OSC routing function */
  OSCMessage msgIN;
  int size;
  if ((size = Udp.parsePacket()) > 0) {
    while (size--)
      msgIN.fill(Udp.read());
    if (!msgIN.hasError()) {
      msgIN.route("/close", closeShutter);
      msgIN.route("/open", openShutter);
      msgIN.route("/position", getPositionShutter);
      msgIN.route("/reboot", rebootShutter);
      msgIN.route("/move", moveShutter);
      msgIN.route("/servo_pin", ServoPin);
      msgIN.route("/test", testServo);
      msgIN.route("/shutter_ip", ShutterIP);
      msgIN.route("/master_ip", MasterIP);
      msgIN.route("/shutter_port", ShutterPort);
      msgIN.route("/master_port", MasterPort);
      msgIN.route("/set_closed", setClosed);
      msgIN.route("/set_open", setOpen);
      msgIN.route("/reset_config", resetConfig);
      msgIN.route("/ping", Pong);
    }
  }
}

#endif /* pretoOSC_H_ */
