#include <Sifflet.h>
const int pinAmbiant = A12;
const int pinSifflet = A13;

// Fonction détecteur de sifflet
bool detectionsifflet(){  // Définir les # de pin utilisées sur l'Arduino
//Serial.println(analogRead(pinSifflet));
   if(analogRead(pinSifflet) > 450)
   {
  //     Serial.println(analogRead(pinAmbiant));
  //     Serial.println(analogRead(pinSifflet));
     return true;
  }else{
    return false;
   }
}