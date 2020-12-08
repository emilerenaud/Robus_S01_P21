#ifndef Strip_DEL_H

#define Strip_DEL_H
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel strip;
// 1 couleur sur la longeur de la strip
void colorWipe(uint32_t color, int wait);

// Flash de lumière qui se déplace vers la gauche
void theaterChase(uint32_t color, int wait);

// Arc-en-ciel - temps en millisecondes pour changer de pixel
void rainbow(int wait);

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait);

#endif