#include<Suiveur_ligne.h>

void Suiveurdeligne()
{
float valeurnoir = 4.3;
float valeurvert = 1.2;
float VoltageValue[8];
const uint8_t LINE_FOLLOWER_PINS[8] = {A4, A5, A6, A7, A8, A9, A10, A11};

  for(int i=0; i<8;i++) // 8 capteurs
  {

    int SensorValue = analogRead(LINE_FOLLOWER_PINS[i]);
  
    VoltageValue[i] = SensorValue/204.6; 
    
    if(VoltageValue[i]<valeurvert) // valeur à laquelle nous voulons qu'un entier différent soit renvoyer
    {
      ReferenceInstruction[i] = 1;
    }
    else if   (VoltageValue[i]>valeurnoir)
    {
      ReferenceInstruction[i] = 1;
    }
    else 
    {
      ReferenceInstruction[i] = 0;
    }
    
   // Serial.println(ReferenceInstruction[i]);
   
  }
}  // renvoi la valeur de 1 si le capteur recoit un voltage supérieur la valeur de réflection