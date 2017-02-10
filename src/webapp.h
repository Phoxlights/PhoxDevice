#ifndef WEBAPP_H
#define WEBAPP_H

#include <Arduino.h>

int webAppBegin();
int webAppOnReady(int (*onReady)());
int webAppOnColor(int (*onColor)(byte rgb[3]));

#endif
