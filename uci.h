#ifndef UCI_H
#define UCI_H

#include "defs.h"

void initInput();

extern bool Init;
extern bool Searching;
extern bool Infinite;
extern int syzygyProbeLimit;
extern bool dtzPathGiven;

extern bool checkInput();
extern void listenForInput();

#endif
