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
#include <Musique.h>

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
// const int32_t vitesse = pwm * 150; // wtf 1280 ?
#define TEMP_BOUCLE 10
int32_t wantedSpeed = 45; // 40 pulses / 10ms. // normalement pu utiliser
const float kp = 0.0001f;
const float ki = 0.00002f;
int direction[2] = {1,1};
float puissance_moteur[2] = {pwm, pwm};
int32_t lastEncodeur[2] = {0,0};
int32_t compteur;
int etape = 0;
int ligne = 0;                     // Calcul le nombre de lignes franchies depuis le début du parcour
uint16_t rRef, gRef, bRef, cRef;   // Valeur de référence capteur de couleur
Color couleur; // couleur detecter par le capteur

// PROTOTYPES
float erreurProportionel(int32_t pulseMoteur);
float erreurIntergral(int32_t p_pulse);
void ponderer_vitesse(uint8_t roue);
int32_t conversion_mmpulse(int32_t mm);
void avancerDistance(int32_t p_pulse);
int32_t avancer(bool onOff);
void tourner(int16_t angle);
void pivot(int16_t angle);
void FonctionServo(uint8_t servo, uint8_t angle);


void setup()
{
  Serial.begin(9600);
  Serial.println("allo");  
  BoardInit(); 
  FonctionServo(0,1);
  ColorSensorInit(&rRef, &gRef, &bRef, &cRef);
  del_init();
  suiveurLigne_init();
  MOTOR_SetSpeed(0,0);
  MOTOR_SetSpeed(1,0);
  ENCODER_Reset(LEFT);
  ENCODER_Reset(RIGHT);
  // pivot(90); // tourner a gauche  test
  // playMusique();
  // etape = -1; // pas faire le parcour. Mode debug.
  // delay(1000);
}

/* ****************************************************************************
Fonctions de boucle infini (loop())
**************************************************************************** */

void loop() 
{
  static uint32_t lastMillis = 0;
  static uint32_t compteurDelay = 0;
  if(millis() - lastMillis >= TEMP_BOUCLE) // Faire la grosse boucle au 10ms pour pas allez trop vite.
  {
    lastMillis = millis();
    // Mode debug
    if(etape == -1)
    {
      if(avancer(ON) > 1000)
      {
        avancer(OFF);
        etape = -2;
      }
      // if(forward(1000) == 0)
      // {
      //   etape = -2;
      // }
      // avancerDistance(conversion_mmpulse(1000));
      // avancerDistance(conversion_mmpulse(1000));
      // pivot(180);
      // // avancerDistance(conversion_mmpulse(1000));
      // pivot(-180);

      // // avancerDistance(conversion_mmpulse(1000));
      // pivot(180);
      // // avancerDistance(conversion_mmpulse(1000));
      // pivot(-180);

      // // avancerDistance(conversion_mmpulse(1000));
      // pivot(180);
      // // avancerDistance(conversion_mmpulse(1000));
      // pivot(-180);
      // etape = -2;
    }
    else if(etape == -2)
    {
      // pivot(180);
      tourner(181);
      delay(500);
      etape = -3;
    }
    else if(etape == -3)
    {
      if(avancer(ON) > 1000)
      {
        avancer(OFF);
        etape = -4;
      }
    }
    else if(etape == -4)
    {
      // pivot(180);
      tourner(181);
      delay(500);
      etape = -5;
    }

  // ****************** PARCOUR *********************** //
    if (etape == 0)          // Étape detection sifflet
    {    
      if(detectionsifflet() == true)
      {
        Serial.println("Détection sifflet");
        etape++;
      }
      delay(2000);  // faire starter le robot tu seul
      etape++;      // faire starter le robot tu seul
      etape++;      // allez a l'etape 2
    }
    else if (etape == 1)     // Étape tourner à droite
    {
      pivot(-90); 
      delay(500);
      etape++;
    }
    else if (etape == 2)       // Étape centrage du robot
    {
      avancer(ON);
      if(suiveurLigne2() > 6)
      {
        // Serial.println("Ligne 1 trouver!");
        avancer(OFF);
        // toggle_led();      // Allumer DEL quand il voit une ligne
        // avancerDistance(conversion_mmpulse(20));
        etape ++;
      }
    }
    else if (etape == 3)   // Étape tourner à gauche
    {
      pivot(90); 
      // tourner(92);       
      delay(500);
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
      if(suiveurLigne2() > 6)
      {
        // toggle_led();
        Serial.println("Ligne 2 trouver!");
        // delay(1000);  // pas de delay. Faire non bloquant. sinon le PID bug.
        etape ++;
      }
    }
    else if (etape == 6)       // Étape pour avancer pour clearer le capteur de couleur 
    {
      avancer(ON); // recall la fonction pour faire le PID.
      if(compteurDelay < 10) // delay non bloquant de 1sec ish.
      {
        compteurDelay ++;
      }
      else if(suiveurLigne2() > 6)
      {
        // toggle_led();
        // Serial.println("Ligne 3 trouver!");
        avancer(OFF);
        avancerDistance(conversion_mmpulse(150)); // Avancer pour que le capteur de couleur capte 250
        etape ++;
      }
        // avancerDistance(conversion_mmpulse(1150));
        // etape ++;
    }
    
    else if (etape == 7)           // Étape pour lire couleur
    {
      couleur = getTrueColor();
      Serial.println("Couleur = " + String(couleur));
      gererDEL(couleur);
      etape++;
    }
    else if (etape == 8)      // Étape avncer jusqu'à l'autre ligne 
    {
      avancer(ON);
      if(suiveurLigne2() > 6)
      {
        Serial.println("Ligne 4 trouver!");
        avancer(OFF);
        // toggle_led();
        etape ++;
      }
    }
    else if (etape == 9)       // Étape monter le servo (étape 9 à 11)
    {
      FonctionServo(0,45);
      avancerDistance(conversion_mmpulse(200));
      // FonctionServo(0,2);
      etape ++;
      
    }
     else if (etape == 10)
    {
      FonctionServo(0,30);
      avancerDistance(conversion_mmpulse(200));
      etape ++;
      
    }
    else if (etape == 11)
    {
      FonctionServo(0,2);
      etape ++;
      
    }
    else if (etape == 12)  // Étape si couleur = rouge
    {
      if (couleur == RED)
      {

      }
      else
      {
        etape++;
      }
    }
    else if (etape == 13)       // Étape si couleur = jaune
    {
      if (couleur == YELLOW)
      {
        
      }
      else
      {
        etape++;
      }
      
    }
    else if (etape == 14)         // Étape si couleur = cyan
    {
      if (couleur == CYAN)
      {
        static int s_etape = 0;
        if(s_etape == 0)
        {
          avancerDistance(conversion_mmpulse(240)); // why the fuck  avancer de 240 ?
          Serial.println("fin avancer distance");
          s_etape++;
        }
        else if (s_etape == 1)
        {
          
          avancer(ON);
          if(suiveurLigne2() > 6)
          {
            Serial.println("ligne 1");
            // toggle_led();
            s_etape++;
          }
        }
        else if (s_etape == 2)
        {
          avancer(ON);
          if(suiveurLigne2() > 6)
          {
            Serial.println("ligne 2");
            // toggle_led();
            avancer(OFF);
            s_etape++;
          }
        }
        else if (s_etape == 3)
        {
          
          //avancerDistance(conversion_mmpulse(150));
          tourner(90);
          s_etape++;
        }
        else if (s_etape == 4)
        {
          avancer(ON);
          if (suiveurLigne2() > 6)
          {
            Serial.println("ligne 3 ou stop");
            avancer(OFF);
            playMusique();
            // toggle_led();
            s_etape ++;
            etape ++;
          }
        }
      }
      else
      {
        etape++; // Si couleur n'est pas le CYAN
      }
    }
  }
}

/*************** FONCTIONS AVANCER - PID - TOURNER ********************/
/***** PID *****/
// P
float erreurProportionel(int32_t pulseMoteur){
   return float((wantedSpeed-pulseMoteur) * kp);
}
// I
float erreurIntergral(int32_t p_pulse)
{
    static uint32_t nbCycle = 0;
    nbCycle++;
    // Serial.print(vitesse * nbCycle);
    Serial.print(" ~ ");
    Serial.println(p_pulse);
    return (wantedSpeed * nbCycle/2 - p_pulse) * ki;
}

// Ajuster vitesse
void ponderer_vitesse(uint8_t roue)
{
  int32_t nowEndodeur = abs(ENCODER_Read(roue));
  compteur = nowEndodeur - lastEncodeur[roue]; // Normalement pu utiliser
  float correction = erreurProportionel(nowEndodeur - lastEncodeur[roue]);
  puissance_moteur[roue] += correction;
  // puissance_moteur[roue] += erreurProportionel();// + erreurIntergral(Nombre de pulse);
  lastEncodeur[roue] = nowEndodeur;
  // if(puissance_moteur[roue] < 0.2)
  // {
  //   puissance_moteur[roue] = 0.2;
  // }
  // else if(puissance_moteur[roue] > 1)
  // {
  //   puissance_moteur[roue] = 1;
  // }
  if(roue == LEFT)
  {
    MOTOR_SetSpeed(roue, (puissance_moteur[roue]) * direction[roue]);
    // Serial.print("Gauche = " + String(puissance_moteur[roue]));
    // Serial.print("   Compteur L = " + String(compteur));
    // Serial.print("  Correction = " + String(correction));
  }
  else
  {
    MOTOR_SetSpeed(roue, puissance_moteur[roue]* direction[roue]);
    // Serial.print("  Droite = " + String(puissance_moteur[roue]));
    // Serial.print("  Compteur R = " + String(compteur));
    // Serial.println("  Correction = " + String(correction));
  }
  
  
}

// Convertion mm en pulse.
int32_t conversion_mmpulse(int32_t mm){
  return(mm*PulsePerTurn/WheelCircumference);
}

int32_t conversion_pulsemm(int32_t pulse){
  return(pulse*WheelCircumference/PulsePerTurn);
}

// Fonction avancer d'un nombre de pulse.
void avancerDistance(int32_t p_pulse)
{
  //Serial.println(p_pulse);
  direction[LEFT] = 1;
  direction[RIGHT] = 1;
  lastEncodeur[0] = lastEncodeur[1] = 0;
  puissance_moteur[LEFT] = 0.23;
  puissance_moteur[RIGHT] = 0.20;
  ENCODER_ReadReset(0);
  ENCODER_ReadReset(1);
  MOTOR_SetSpeed(LEFT, 0.25);
  delayMicroseconds(100);
  // delay(5);
  MOTOR_SetSpeed(RIGHT, 0.20);

  static uint32_t lastMillis1 = millis();
  while(p_pulse > ENCODER_Read(0))
  {
    if(millis() - lastMillis1 >= TEMP_BOUCLE)
    {
      lastMillis1 = millis();
      ponderer_vitesse(LEFT);
      ponderer_vitesse(RIGHT);
    }
    // delay(100);
  }
  delay(500);
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

      puissance_moteur[LEFT] = 0.23;
      puissance_moteur[RIGHT] = 0.20;
      MOTOR_SetSpeed(LEFT, 0.25);
      delayMicroseconds(100);
      // delay(5);
      MOTOR_SetSpeed(RIGHT, 0.20);
      delay(10); // le temps qu'il avance un peu.
    }

    ponderer_vitesse(LEFT);
    ponderer_vitesse(RIGHT);
    distancePulse = ENCODER_Read(RIGHT);
  }
  else
  {
    initAvancer = 1;
    MOTOR_SetSpeed(LEFT,0);
    MOTOR_SetSpeed(RIGHT,0);
    lastEncodeur[0] = lastEncodeur[1] = 0;
    distancePulse = 0;
    ENCODER_ReadReset(0);
    ENCODER_ReadReset(1);

    puissance_moteur[LEFT] = 0.23;
    puissance_moteur[RIGHT] = 0.20;
  }
  return conversion_pulsemm(distancePulse);
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
  MOTOR_SetSpeed(roue, 0.2);
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
  // MOTOR_SetSpeed(LEFT, puissance_moteur[LEFT] * direction[LEFT]);
  // MOTOR_SetSpeed(RIGHT, puissance_moteur[RIGHT] * direction[RIGHT]);
  MOTOR_SetSpeed(LEFT, 0.26 * direction[LEFT]);
  MOTOR_SetSpeed(RIGHT, 0.25 * direction[RIGHT]);

  static uint32_t lastMillis2 = millis();

  int32_t encoderLeft = abs(ENCODER_Read(LEFT));
  int32_t encoderRight = abs(ENCODER_Read(RIGHT));

  while(pulse >= encoderLeft || pulse >= encoderRight)
  {
    if(millis() - lastMillis2 >= TEMP_BOUCLE/5) // live a 10ms
    {
      lastMillis2 = millis();
      // Update Encoder each 10ms.
      encoderLeft = abs(ENCODER_Read(LEFT));
      encoderRight = abs(ENCODER_Read(RIGHT));

      if(pulse >= encoderLeft) // Verifier si le coter gauche a finir de bouger.
      {
        // ponderer_vitesse(LEFT);           // le faire continuer d'avancer sinon.
      }
      else
      {
        MOTOR_SetSpeed(LEFT,0);           // l'arreter s'il a fini.
      }
  
      if(pulse >= encoderRight)  // same shit for right side.
      {
        // ponderer_vitesse(RIGHT);
      }
      else
      {
        MOTOR_SetSpeed(RIGHT,0);
      }
      
      
    }
    // delay(100);
  }
  // delay(500);
  MOTOR_SetSpeed(0,0); // fucking define plz.
  MOTOR_SetSpeed(1,0);
  delay(500);
}

void FonctionServo(uint8_t servo, uint8_t angle)
{
  SERVO_Enable(servo);
  SERVO_SetAngle(servo, angle);
}
