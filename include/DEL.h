#ifndef DEL_H

#define DEL_H
#include <Arduino.h>
#include <color_sensor.h>

void gererDEL(Color capteurcouleur);
void del_init();
void toggle_led();

// # pin utilisées sur l'arduino
const int pinRouge = 40;
const int pinBleu = 41;
const int pinJaune = 42;

#endif