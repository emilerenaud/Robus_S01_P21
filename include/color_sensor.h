#ifndef color_sensor_h

#define color_sensor_h
#include <Arduino.h>
#include "Adafruit_TCS34725.h"

extern uint16_t rRef, gRef, bRef, cRef;

enum Color {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
Color getTrueColor();
void printColor(uint16_t red, uint16_t green, uint16_t blue, uint16_t clear);
void ColorSensorInit(uint16_t* rRef, uint16_t* gRef, uint16_t* bRef, uint16_t* cRef);

#endif