
/*
  "preto.h"
  Interface for the commandMode enum and findCommandMode() function. This will also include DMX and Manual interfaces in the future.
 */

#ifndef PRETO_H_
#define PRETO_H_

#include "pretoOSC.h" // OSC library for this project

/* commandMode enum holds the possible command modes for the shutter */
enum commandMode {
  osc,
  manual,
  dmx
};

/* findCommandMode() will be the function which returns the currently chosen command mode */
commandMode findCommandMode() {
  return osc;
}

#endif /* PRETO_H_ */
