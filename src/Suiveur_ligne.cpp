#include<Suiveur_ligne.h>

uint16_t baselineSuiveurLigne[8];
const uint8_t LINE_FOLLOWER_PINS[8] = {A5, A6, A7, A8, A9, A10, A11, A12};
uint8_t thresholdLigne = 150;  // baseline sur le plancher environ 400-560.
                              // sur tape electrique environ 865 a 950
                              // sur tape vert environ 38 et 120

void suiveurLigne_init()
{
  for(int i=0; i<8;i++) // 8 capteurs
  {
    baselineSuiveurLigne[i] = analogRead(LINE_FOLLOWER_PINS[i]);
  }
}


void Suiveurdeligne()
{
float valeurnoir = 4.3;
float valeurvert = 1.2;
float VoltageValue[8];

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

uint8_t suiveurLigne2()
{
  uint16_t analogValue[8];
  uint8_t compteurCapteur = 0;

  for(int i=0; i<8;i++) // 8 capteurs
  {
    analogValue[i] = analogRead(LINE_FOLLOWER_PINS[i]);
    if((analogValue[i] + thresholdLigne) < baselineSuiveurLigne[i] || (analogValue[i] - thresholdLigne) > baselineSuiveurLigne[i] ) // voir les commentaires sur thresholdLigne.
    {                     
      compteurCapteur ++;
    }
  }
  return compteurCapteur;
}


uint8_t suiveurLigne3()
{
  uint16_t analogValue[8];
  uint8_t compteurCapteur = 0;
  for(int i=0; i<8;i++) // 8 capteurs
  {
    analogValue[i] = analogRead(LINE_FOLLOWER_PINS[i]);
  }

  if((analogValue[0] + thresholdLigne) < baselineSuiveurLigne[0] || (analogValue[0] - thresholdLigne) > baselineSuiveurLigne[0])
    compteurCapteur += 1;

  if((analogValue[1] + thresholdLigne) < baselineSuiveurLigne[1] || (analogValue[1] - thresholdLigne) > baselineSuiveurLigne[1])
    compteurCapteur += 2;

  if((analogValue[2] + thresholdLigne) < baselineSuiveurLigne[2] || (analogValue[2] - thresholdLigne) > baselineSuiveurLigne[2])
    compteurCapteur += 4;

  if((analogValue[3] + thresholdLigne) < baselineSuiveurLigne[3] || (analogValue[3] - thresholdLigne) > baselineSuiveurLigne[3])
    compteurCapteur += 8;

  if((analogValue[4] + thresholdLigne) < baselineSuiveurLigne[4] || (analogValue[4] - thresholdLigne) > baselineSuiveurLigne[4])
    compteurCapteur += 16;

  if((analogValue[5] + thresholdLigne) < baselineSuiveurLigne[5] || (analogValue[5] - thresholdLigne) > baselineSuiveurLigne[5])
    compteurCapteur += 32;

  if((analogValue[6] + thresholdLigne) < baselineSuiveurLigne[6] || (analogValue[6] - thresholdLigne) > baselineSuiveurLigne[6])
    compteurCapteur += 64;

  if((analogValue[7] + thresholdLigne) < baselineSuiveurLigne[7] || (analogValue[7] - thresholdLigne) > baselineSuiveurLigne[7])
    compteurCapteur += 128;

  return compteurCapteur;
}