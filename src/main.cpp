/*
Projet: Le nom du script
Equipe: Votre numero d'equipe
Auteurs: Les membres auteurs du script
Description: Breve description du script
Date: Derniere date de modification
*/
/* ****************************************************************************
Inclure les librairies de functions que vous voulez utiliser
**************************************************************************** */

#include <LibRobus.h> // Essentielle pour utiliser RobUS
#include <Arduino.h> 
#include <Moteur.h>
#include <Sifflet.h>
#include <Suiveur_ligne.h>
#include <capteur_IR.h>
#include <DEL.h>
#include <color_sensor.h>
#include <Musique.h>


/* ****************************************************************************
Variables globales et defines
**************************************************************************** */
#define ON 1
#define OFF 0
#define FORWARD 0
#define BACKWARD 1

#define TEMP_BOUCLE 10



int etape = 0;
int ligne = 0;                     // Calcul le nombre de lignes franchies depuis le début du parcour

// PROTOTYPES


void setup()
{
  Serial.begin(9600);
  BoardInit(); 

  // suiveurLigne_init();
  moteur_init();
  delay(2000);
  // bougerDistance(100,FORWARD);
  // playMusique();


}

/* ****************************************************************************
Fonctions de boucle infini (loop())
**************************************************************************** */

void loop() 
{
  static uint32_t lastMillis = 0;
  static int step = 1;
  uint8_t ligne = 0;
  static bool run = 0;
  if(ROBUS_IsBumper(3) == 1) // bumper arriere.
  {
    run = 1;
  }

  if(run)
  {
    if(millis() - lastMillis >= TEMP_BOUCLE) // Faire la grosse boucle au 10ms pour pas allez trop vite.
    {
      lastMillis = millis();
      if(step == 1)
      {
        parcourTable();
        delay(2000);
      }
    } // end timed loop (10ms)
  }
} // end loop.

/*************** FONCTIONS AVANCER - PID - TOURNER ********************/
/***** PID *****/
// P
// float erreurProportionel(int32_t pulseMoteur){
//    return float((wantedSpeed-pulseMoteur) * kp);
// }
// // I
// float erreurIntergral(int32_t p_pulse)
// {
//     static uint32_t nbCycle = 0;
//     nbCycle++;
//     // Serial.print(vitesse * nbCycle);
//     Serial.print(" ~ ");
//     Serial.println(p_pulse);
//     return (wantedSpeed * nbCycle/2 - p_pulse) * ki;
// }

// // Ajuster vitesse
// void ponderer_vitesse(uint8_t roue)
// {
//   int32_t nowEndodeur = abs(ENCODER_Read(roue));
//   compteur = nowEndodeur - lastEncodeur[roue]; // Normalement pu utiliser
//   float correction = erreurProportionel(nowEndodeur - lastEncodeur[roue]);
//   puissance_moteur[roue] += correction;
//   // puissance_moteur[roue] += erreurProportionel();// + erreurIntergral(Nombre de pulse);
//   lastEncodeur[roue] = nowEndodeur;
//   // if(puissance_moteur[roue] < 0.2)
//   // {
//   //   puissance_moteur[roue] = 0.2;
//   // }
//   // else if(puissance_moteur[roue] > 1)
//   // {
//   //   puissance_moteur[roue] = 1;
//   // }
//   if(roue == LEFT)
//   {
//     MOTOR_SetSpeed(roue, (puissance_moteur[roue]) * direction[roue]);
//     // Serial.print("Gauche = " + String(puissance_moteur[roue]));
//     // Serial.print("   Compteur L = " + String(compteur));
//     // Serial.print("  Correction = " + String(correction));
//   }
//   else
//   {
//     MOTOR_SetSpeed(roue, puissance_moteur[roue]* direction[roue]);
//     // Serial.print("  Droite = " + String(puissance_moteur[roue]));
//     // Serial.print("  Compteur R = " + String(compteur));
//     // Serial.println("  Correction = " + String(correction));
//   }
  
  
// }

// // Convertion mm en pulse.
// int32_t conversion_mmpulse(int32_t mm){
//   return(mm*PulsePerTurn/WheelCircumference);
// }

// int32_t conversion_pulsemm(int32_t pulse){
//   return(pulse*WheelCircumference/PulsePerTurn);
// }

// // Fonction avancer d'un nombre de pulse.
// void avancerDistance(int32_t p_pulse)
// {
//   //Serial.println(p_pulse);
//   direction[LEFT] = 1;
//   direction[RIGHT] = 1;
//   lastEncodeur[0] = lastEncodeur[1] = 0;
//   puissance_moteur[LEFT] = 0.23;
//   puissance_moteur[RIGHT] = 0.20;
//   ENCODER_ReadReset(0);
//   ENCODER_ReadReset(1);
//   MOTOR_SetSpeed(LEFT, 0.25);
//   delayMicroseconds(100);
//   // delay(5);
//   MOTOR_SetSpeed(RIGHT, 0.20);

//   static uint32_t lastMillis1 = millis();
//   while(p_pulse > ENCODER_Read(0))
//   {
//     if(millis() - lastMillis1 >= TEMP_BOUCLE)
//     {
//       lastMillis1 = millis();
//       ponderer_vitesse(LEFT);
//       ponderer_vitesse(RIGHT);
//     }
//     // delay(100);
//   }
//   delay(500);
//   MOTOR_SetSpeed(0,0);
//   MOTOR_SetSpeed(1,0);
// }

// int32_t avancer(bool onOff)
// {
//   static bool initAvancer = 1;
//   static int32_t distancePulse = 0;
//   if(onOff == ON)
//   {
//     if(initAvancer == 1)
//     {
//       initAvancer = 0;
//       direction[LEFT] = 1;
//       direction[RIGHT] = 1;
//       lastEncodeur[0] = lastEncodeur[1] = 0;
//       distancePulse = 0;
//       ENCODER_ReadReset(0);
//       ENCODER_ReadReset(1);

//       puissance_moteur[LEFT] = 0.23;
//       puissance_moteur[RIGHT] = 0.20;
//       MOTOR_SetSpeed(LEFT, 0.25);
//       delayMicroseconds(100);
//       // delay(5);
//       MOTOR_SetSpeed(RIGHT, 0.20);
//       delay(10); // le temps qu'il avance un peu.
//     }

//     ponderer_vitesse(LEFT);
//     ponderer_vitesse(RIGHT);
//     distancePulse = ENCODER_Read(RIGHT);
//   }
//   else
//   {
//     initAvancer = 1;
//     MOTOR_SetSpeed(LEFT,0);
//     MOTOR_SetSpeed(RIGHT,0);
//     lastEncodeur[0] = lastEncodeur[1] = 0;
//     distancePulse = 0;
//     ENCODER_ReadReset(0);
//     ENCODER_ReadReset(1);

//     puissance_moteur[LEFT] = 0.23;
//     puissance_moteur[RIGHT] = 0.20;
//   }
//   return conversion_pulsemm(distancePulse);
// }

// // Fonction touner avec un angle. Genre
// void tourner(int16_t angle){
//   int32_t pulse = conversion_mmpulse(abs(angle) * CONVERSION_DEGRE_RAD * R);
//   //Serial.print(pulse); 
//   int8_t roue;
//   if (angle > 0){
//    roue = RIGHT;
//   }
//   else{
//     roue = LEFT;
//   }
//   direction[roue] = 1;
//   direction[1-roue] = 0;
//   ENCODER_ReadReset(roue);
//   MOTOR_SetSpeed(roue, 0.2);
//   lastEncodeur[0] = lastEncodeur[1] = 0;
//   while(pulse > ENCODER_Read(roue)){
//     // ponderer_vitesse(roue);
//     //Serial.println(ENCODER_Read(roue));
//     delay(2);
//   }
//   MOTOR_SetSpeed(roue,0);
// }

// void pivot(int16_t angle){
//  int32_t pulse = conversion_mmpulse(abs(angle /2) * CONVERSION_DEGRE_RAD * R);
//   if (angle > 0)
//   {
//     direction[LEFT] = -1;
//     direction[RIGHT] = 1;
//   }
//     else
//     {
//       direction[LEFT] = 1;
//       direction[RIGHT] = -1;
//     }
//   lastEncodeur[0] = lastEncodeur[1] = 0;
//   ENCODER_ReadReset(0);
//   ENCODER_ReadReset(1);
//   // MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT] * direction[LEFT]);
//   // MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT] * direction[RIGHT]);
//   MOTOR_SetSpeed(LEFT, 0.26 * direction[LEFT]);
//   MOTOR_SetSpeed(RIGHT, 0.25 * direction[RIGHT]);

//   static uint32_t lastMillis2 = millis();

//   int32_t encoderLeft = abs(ENCODER_Read(LEFT));
//   int32_t encoderRight = abs(ENCODER_Read(RIGHT));

//   while(pulse >= encoderLeft || pulse >= encoderRight)
//   {
//     if(millis() - lastMillis2 >= TEMP_BOUCLE/5) // live a 10ms
//     {
//       lastMillis2 = millis();
//       // Update Encoder each 10ms.
//       encoderLeft = abs(ENCODER_Read(LEFT));
//       encoderRight = abs(ENCODER_Read(RIGHT));

//       if(pulse >= encoderLeft) // Verifier si le coter gauche a finir de bouger.
//       {
//         // ponderer_vitesse(LEFT);           // le faire continuer d'avancer sinon.
//       }
//       else
//       {
//         MOTOR_SetSpeed(LEFT,0);           // l'arreter s'il a fini.
//       }
  
//       if(pulse >= encoderRight)  // same shit for right side.
//       {
//         // ponderer_vitesse(RIGHT);
//       }
//       else
//       {
//         MOTOR_SetSpeed(RIGHT,0);
//       }
      
      
//     }
//     // delay(100);
//   }
//   // delay(500);
//   MOTOR_SetSpeed(0,0); // fucking define plz.
//   MOTOR_SetSpeed(1,0);
//   delay(500);
// }

// void FonctionServo(uint8_t servo, uint8_t angle)
// {
//   SERVO_Enable(servo);
//   SERVO_SetAngle(servo, angle);
// }
