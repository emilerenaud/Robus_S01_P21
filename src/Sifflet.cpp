#include <Sifflet.h>
const int pinAmbiant = A12;
const int pinSifflet = A13;

// Fonction détecteur de sifflet
bool detectionsifflet() // Définir les # de pin utilisées sur l'Arduino
{  
   static uint8_t compteurTempsSifflet = 0;
   if(analogRead(pinSifflet) > analogRead(pinAmbiant))
   {
      compteurTempsSifflet ++;
      if(compteurTempsSifflet > 10) // 50 * 10ms. le sifflet doit durer au moins 500ms. 10 pour les tests.
      {
         compteurTempsSifflet = 0;
         return 1;
      }
      return 0;
  //     Serial.println(analogRead(pinAmbiant));
  //     Serial.println(analogRead(pinSifflet));
   //   return true;
   }
   else
   {
    return false;
   }
}