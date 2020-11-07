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
#include <Sifflet.h>
#include <Suiveur_ligne.h>
#include <capteur_IR.h>
#include <DEL.h>
#include <color_sensor.h>

/* ****************************************************************************
Variables globales et defines
**************************************************************************** */
#define ON 1
#define OFF 0

int ReferenceInstruction[8]; 
const int PulsePerTurn = 3200;
const float Pi = 3.14159f;
const float InchesToMm = 25.4f;
const float WheelDiameter = 3 * InchesToMm;
const float WheelCircumference = WheelDiameter * Pi;
const float pwm = 0.3; // wtf pwm ?

const float R = 187; //mm rayon cerle robot
const float CONVERSION_DEGRE_RAD = 2*Pi/360;
//Vitesse en pulse/s
const int32_t vitesse = pwm * 1280; // wtf 1280 ?
const float kp = 0.0001f;
const float ki = 0.00002f;
int direction[2] = {1,1};
float puissance_moteur[2] = {pwm, pwm};
int32_t lastEncodeur[2] = {0,0};
int32_t compteur;
int etape = 0;
int ligne = 0;                     // Calcul le nombre de lignes franchies depuis le début du parcour
uint16_t rRef, gRef, bRef, cRef;   // Valeur de référence capteur de couleur

// PROTOTYPES
float erreurProportionel(void);
float erreurIntergral(int32_t p_pulse);
void ponderer_vitesse(uint8_t roue);
int32_t conversion_mmpulse(int32_t mm);
void avancerDistance(int32_t p_pulse);
void avancer(bool onOff);
void tourner(int16_t angle);
void pivot(int16_t angle);

void setup(){

  Serial.begin(9600);  
  BoardInit(); 
  ColorSensorInit(&rRef, &gRef, &bRef, &cRef);
  del_init();
  MOTOR_SetSpeed(0,0);
  MOTOR_SetSpeed(1,0); 
  
}

/* ****************************************************************************
Fonctions de boucle infini (loop())
**************************************************************************** */

void loop() 
{
  if (etape == 0)          // Étape detection sifflet
  {    
    if(detectionsifflet() == true){
      Serial.println("Détection sifflet");
      etape++;
    }
  }
  else if (etape == 1)     // Étape tourner
  {
    pivot(90);
    etape++;
  }
  else if (etape == 2)       // Étape centrage 
  {
    avancer(ON);
    Suiveurdeligne();

    int somme = 0;
    for (int i = 0; i<8; i++)
    {
      somme += ReferenceInstruction[i]; 
    }
    if(somme >6)
    {
      avancer(OFF);
      avancerDistance(conversion_mmpulse(30)); 
      etape++;
    }
  }
  else if (etape == 3)   // Étape tourner
  {
    pivot(-90);;
    etape++;
  }
  else if (etape == 4)    // Étape attendre départ autre robot
  {
    delay(1000);
    etape++;
  }
  else if (etape == 5)     // Étape pour avancer jusqu'à la pastille de couleur
  {
    avancer(ON);

    int somme2 = 0;

    for (int i = 0; i<8; i++)
    {
      somme2 += ReferenceInstruction[i]; 
    }
    if(somme2 >6)
    {
      ligne++;
    }
    if(ligne == 2)
    {
      avancer(OFF);
      avancerDistance(conversion_mmpulse(200));
      etape++;
    }
  } 
  else if (etape == 6)           //Lire couleur
  {
    Color c = getTrueColor();
    gererDEL(c);
    etape++;
  //if(c == RED)
  //{
  //}
  }
}


/*************** FONCTIONS AVANCER - PID - TOURNER ********************/
/***** PID *****/
// P
float erreurProportionel(){
   return (vitesse-compteur) * kp;
}
// I
float erreurIntergral(int32_t p_pulse)
{
    static uint32_t nbCycle = 0;
    nbCycle++;
    Serial.print(vitesse * nbCycle);
    Serial.print(" ~ ");
    Serial.println(p_pulse);
    return (vitesse * nbCycle/2 - p_pulse) * ki;
}

// Ajuster vitesse
void ponderer_vitesse(uint8_t roue)
{
  int32_t nowEndodeur = abs(ENCODER_Read(roue));
  compteur= nowEndodeur - lastEncodeur[roue];
  lastEncodeur[roue] = nowEndodeur;
  puissance_moteur[roue] += erreurProportionel();// + erreurIntergral(nowEndodeur);
  if(puissance_moteur[roue] < 0.2)
  {
    puissance_moteur[roue] = 0.2;
  }
  else if(puissance_moteur[roue] > 1)
  {
    puissance_moteur[roue] = 1;
  }
  MOTOR_SetSpeed(roue, puissance_moteur[roue]* direction[roue]);
}

// Convertion mm en pulse.
int32_t conversion_mmpulse(int32_t mm){
  return(mm*PulsePerTurn/WheelCircumference);
}

// Fonction avancer d'un nombre de pulse.
void avancerDistance(int32_t p_pulse)
{
  //Serial.println(p_pulse);
  direction[LEFT] = 1;
  direction[RIGHT] = 1;
  lastEncodeur[0] = lastEncodeur[1] = 0;
  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);
  MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT]);
  MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT]);

  static uint32_t lastMillis = millis();
  while(p_pulse > ENCODER_Read(0)){
    if(millis() - lastMillis >= 100)
    {
      lastMillis = millis();
      ponderer_vitesse(LEFT);
      ponderer_vitesse(RIGHT);
    }
    // delay(100);
  }
  MOTOR_SetSpeed(0,0);
  MOTOR_SetSpeed(1,0);
}

void avancer(bool onOff)
{
  static bool initAvancer = 1;
  if(onOff == ON)
  {
    if(initAvancer == 1)
    {
      initAvancer = 0;
      direction[LEFT] = 1;
      direction[RIGHT] = 1;
      lastEncodeur[0] = lastEncodeur[1] = 0;
      ENCODER_ReadReset(0);
      ENCODER_ReadReset(1);
      MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT]);
      MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT]);
    }

    ponderer_vitesse(LEFT);
    ponderer_vitesse(RIGHT);
  }
  else
  {
    initAvancer = 1;
    MOTOR_SetSpeed(LEFT,0);
    MOTOR_SetSpeed(RIGHT,0);
  }
}

// Fonction touner avec un angle. Genre
void tourner(int16_t angle){
  int32_t pulse = conversion_mmpulse(abs(angle) * CONVERSION_DEGRE_RAD * R);
  //Serial.print(pulse); 
  int8_t roue;
  if (angle > 0){
   roue = RIGHT;
  }
  else{
    roue = LEFT;
  }
  direction[roue] = 1;
  direction[1-roue] = 0;
  ENCODER_ReadReset(roue);
  MOTOR_SetSpeed(roue, puissance_moteur[roue]);
  lastEncodeur[0] = lastEncodeur[1] = 0;
  while(pulse > ENCODER_Read(roue)){
    // ponderer_vitesse(roue);
    //Serial.println(ENCODER_Read(roue));
    delay(2);
  }
  MOTOR_SetSpeed(roue,0);
}

void pivot(int16_t angle){
 int32_t pulse = conversion_mmpulse(abs(angle /2) * CONVERSION_DEGRE_RAD * R);
  if (angle > 0)
  {
    direction[LEFT] = -1;
    direction[RIGHT] = 1;
  }
    else
    {
      direction[LEFT] = 1;
      direction[RIGHT] = -1;
    }
  lastEncodeur[0] = lastEncodeur[1] = 0;
     ENCODER_ReadReset(0);
     ENCODER_ReadReset(1);
      MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT] * direction[LEFT]);
      MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT] * direction[RIGHT]);
  
  static uint32_t lastMillis = millis();
  while(pulse > abs(ENCODER_Read(0))){
    if(millis() - lastMillis >= 100)
    {
      lastMillis = millis();
      ponderer_vitesse(LEFT);
      ponderer_vitesse(RIGHT);
    }
    // delay(100);
  }
  MOTOR_SetSpeed(0,0);
  MOTOR_SetSpeed(1,0);
}


