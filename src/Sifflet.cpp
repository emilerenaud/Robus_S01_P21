#include <Sifflet.h>
const int pinAmbiant = 12;
const int pinSifflet = 13;
// Fonction détecteur de sifflet
bool detectionsifflet(int pinAmbiant, int pinSifflet){  // Définir les # de pin utilisées sur l'Arduino

  if(analogRead(pinAmbiant) < analogRead(pinSifflet)){
    return true;
  }else{
    return false;
   }
}