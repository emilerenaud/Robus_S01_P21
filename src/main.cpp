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
#include <capteur_IR.h>
#include <Sifflet.h>
#include <Suiveur_ligne.h>

/* ****************************************************************************
Variables globales et defines
**************************************************************************** */
#define ON 1
#define OFF 0

const int PulsePerTurn = 3200;
const float Pi = 3.14159f;
const float InchesToMm = 25.4f;
const float WheelDiameter = 3 * InchesToMm;
const float WheelCircumference = WheelDiameter * Pi;
const float pwm = 0.3; // wtf pwm ?

const float R = 187; //mm rayon cerle robot
const float CONVERSION_DEGRE_RAD = 2*Pi/360;
//Vitesse en pulse/s
const int32_t vitesse = 200; // wtf pwm * 1280 ?
const float kp = 0.0001f;
const float ki = 0.00002f;
const float ballPosition = 2450; //Position de la balle sur le parcours (x)
const float bluePosition = 4000;
const float securityRadius = 150; //Rayon de securite pour balle / zone bleue
int direction[2] = {1,1};
float puissance_moteur[2] = {pwm, pwm};
int32_t lastEncodeur[2] = {0,0};
int32_t compteur;
float currentPosition = 0;
int etape = 0;
bool quilleTrouve = false;

//allo
// PROTOTYPES
float erreurProportionel(void);
float erreurIntergral(int32_t p_pulse);
void ponderer_vitesse(uint8_t roue);
int32_t conversion_mmpulse(int32_t mm);
void avancerDistance(int32_t p_pulse, bool alongXAxis);
int32_t avancer(bool onOff);
void tourner(int16_t angle);
void pivot(int16_t angle);
bool detectionSonar(int maxRangeCm);
void mettreAJourDistance();

void setup(){
  Serial.begin(9600);
  suiveurLigne_init();
  BoardInit();
  while (!detectionsifflet()) 
  {
    delay(10);
  }
}


/* ****************************************************************************
Fonctions de boucle infini (loop())
**************************************************************************** */

void loop() 
{
  static uint32_t lastMillis = 0;
  static uint32_t macroDistance = 0;
  static bool security = false;
  static uint8_t timeout = 0;
    
  if(millis() - lastMillis >= 100)
  {
    lastMillis = millis();
    timeout++;

    //Serial.println(SONAR_GetRange(0));
    //Serial.println(suiveurLigne2());
    //Serial.println(currentPosition);
    //Serial.println(analogRead(A5));


    if (etape == 0 && detectionSonar(60) && currentPosition > 200)
    {
      Serial.println(currentPosition);
      if (currentPosition < ballPosition - securityRadius || currentPosition > ballPosition + securityRadius)
        etape = 1;
      else
        etape = -1; //Cas special ou la quille est allignee avec le ballon      
    }

    if (etape == 0 && currentPosition > 4900)
      etape = 1;

    if (etape == 0)
    {      
      currentPosition = macroDistance * 1000 + avancer(ON);
      if (suiveurLigne2() > 4 && !security && (macroDistance != 3 || currentPosition > 3600))
      {
        security = true;
        macroDistance++;
        avancer(OFF);
        avancer(ON);
        Serial.println("Look at me go! I'm so far!");
      }
      else if (suiveurLigne2() == 0 && timeout > 5)
      {
        MOTOR_SetSpeed(RIGHT, 0);
        delay(50);
        MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT]);
        //Ajuster vers la droite
        Serial.print(currentPosition);
        Serial.print(" - ");
        Serial.println("Oops! Gotta go to the right!");
        timeout = 0;
      }
      else if (suiveurLigne2() > 1 && timeout > 5 && (macroDistance != 3 || currentPosition > 3700))
      {
        MOTOR_SetSpeed(LEFT, 0);
        delay(50);
        MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT]);
        //Ajuster vers la gauche
        Serial.print(currentPosition);
        Serial.print(" - ");
        Serial.println("Oops! Don't wanna get out of bounds!");
        timeout = 0;
      }

      if (suiveurLigne2() < 4)
        security = false;
    }

    if (etape == 1)
    {
      avancer(OFF);
      Serial.println(currentPosition);
      pivot(75);
      avancerDistance(conversion_mmpulse(800), false); //A verifier selon le parcours
      if (currentPosition > bluePosition - securityRadius && currentPosition < bluePosition + securityRadius)
      {
        pivot(-78);
        avancerDistance(conversion_mmpulse(600), false);
      }
      etape = 2;
    }

    if (etape == -1)
    {
      avancer(OFF);
      Serial.println(currentPosition);
      pivot(75);
      delay(25000); //A ajuster
      avancerDistance(conversion_mmpulse(800), false);
      etape = 2;
    }  
  
  }
}

/*************** FONCTIONS AVANCER - PID - TOURNER ********************/
/***** PID *****/
// P
float erreurProportionel(){
   return (vitesse-compteur) * kp;
}
// I
float erreurIntergral(int32_t p_pulse){
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

int32_t conversion_pulsemm(int32_t pulse){
  return(pulse*WheelCircumference/PulsePerTurn);
}

// Fonction avancer d'un nombre de pulse.
void avancerDistance(int32_t p_pulse, bool alongXAxis)
{
  Serial.println(p_pulse);
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

int32_t avancer(bool onOff)
{
  static bool initAvancer = 1;
  static int32_t distancePulse = 0;
  if(onOff == ON)
  {
    if(initAvancer == 1)
    {
      initAvancer = 0;
      direction[LEFT] = 1;
      direction[RIGHT] = 1;
      lastEncodeur[0] = lastEncodeur[1] = 0;
      distancePulse = 0;
      ENCODER_ReadReset(0);
      ENCODER_ReadReset(1);
      MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT]);
      MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT]);
    }

    ponderer_vitesse(LEFT);
    ponderer_vitesse(RIGHT);
    distancePulse = (ENCODER_Read(LEFT) + ENCODER_Read(RIGHT)) / 2;
  }
  else
  {
    initAvancer = 1;
    MOTOR_SetSpeed(LEFT,0);
    MOTOR_SetSpeed(RIGHT,0);
  }
  return conversion_pulsemm(distancePulse);
}

// Fonction touner avec un angle. Genre
void tourner(int16_t angle){
  int32_t pulse = conversion_mmpulse(abs(angle) * CONVERSION_DEGRE_RAD * R);
  Serial.print(pulse); 
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
    Serial.println(ENCODER_Read(roue));
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
  // MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT] * direction[LEFT]);
  // MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT] * direction[RIGHT]);
  MOTOR_SetSpeed(LEFT, 0.2 * direction[LEFT]);
  MOTOR_SetSpeed(RIGHT, 0.2 * direction[RIGHT]);

  static uint32_t lastMillis = millis();

  while(pulse > abs(ENCODER_Read(0)) && pulse > abs(ENCODER_Read(1)))
  {
    if(millis() - lastMillis >= 50) // reduit a 50
    {
      lastMillis = millis();

      for (uint8_t i = 0; i < 2; i++) //Pour chaque roue
      {
        if(pulse > abs(ENCODER_Read(i))) // Verifier si le cote a finit de bouger.
        {
          ponderer_vitesse(i);           // Sinon, continuer d'avancer.
        }
        else
        {
          MOTOR_SetSpeed(i,0);           // l'arreter s'il a fini.
        }
      }      
    }
    // delay(100);
  }
  MOTOR_SetSpeed(0,0); // fucking define.
  MOTOR_SetSpeed(1,0);
}

bool detectionSonar(int maxRangeCm) {

    if (SONAR_GetRange(0) < maxRangeCm)
    {
      Serial.println("Haha! I see something!");
      return true;
    }
  
  return false;
}