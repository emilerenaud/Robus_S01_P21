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
#include <Bouton.h>
#include <Adafruit_NeoPixel.h>


/* ****************************************************************************
Variables globales et defines
**************************************************************************** */
#define ON 1
#define OFF 0
#define FORWARD 0
#define BACKWARD 1

#define TEMP_BOUCLE 10


#define POMPE_A_HIGH 11
#define POMPE_A_LOW 10
#define POMPE_A_PWM 9
#define POMPE_B_HIGH 41
#define POMPE_B_LOW 42
#define POMPE_B_PWM 8

int etape = 0;
int ligne = 0;                     // Calcul le nombre de lignes franchies depuis le début du parcour

// Object 
#define LED_PIN 7
#define LED_COUNT 24
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Constantes
const uint8_t LONGEUR_STRIPE = 24;
const uint8_t BOND_STRIPE = 8;
const uint8_t BOND_DECOMPTE = 1;
const uint8_t NB_COCKTAIL = LONGEUR_STRIPE / BOND_STRIPE;

uint8_t comptrCocktail = 0; 
int8_t comptrConfirmation = 0; //pt depart strip

uint8_t etat = 0;
uint8_t couleurCocktail[NB_COCKTAIL][3] = 
{
 // {RED, GREEN, BLUE}    
    {255,   0,  0}, // Vert
    {100, 100,  0}, // Jaune
    {0, 255,   0} // Rouge
};

// prototype
void melange_1(void);
void melange_2(void);
void melange_3(void);
int Pourcentage(uint8_t valeur);

// FonctionServo
void FonctionServo(uint8_t servo, uint8_t angle)
{
  SERVO_Enable(servo);
  SERVO_SetAngle(servo, angle);
}

void setup()
{
  Serial.begin(9600);
  BoardInit(); 

  // Init pompe
  pinMode(POMPE_A_HIGH,OUTPUT);
  pinMode(POMPE_A_LOW,OUTPUT);
  pinMode(POMPE_B_HIGH,OUTPUT);
  pinMode(POMPE_B_LOW,OUTPUT);
  digitalWrite(POMPE_A_HIGH,HIGH);
  digitalWrite(POMPE_B_HIGH,HIGH);
  digitalWrite(POMPE_A_LOW,LOW);
  digitalWrite(POMPE_B_LOW,LOW);


  InitBouton();

  strip.begin();            // Initialise la strip de DEL (obligatoire)
  strip.show();             // Étaint les DEL
  strip.setBrightness(50);  // Initialise la luminosité des DEL à 1/5 de leur max -> max = 255 
  
  FonctionServo(0, 160);
  FonctionServo(0, 158);
  moteur_init();

  // delay(2000);
    //  Serial.println("d");
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
    Serial.println("allo");
  }
  if(run == 0)
  {
    GererBouton();
    if(FrontMontant(1) == HIGH)
    {
      run = 1;
    }
  }

  if(run)
  {
      // Inputs en parallele
      GererBouton();

      static uint8_t lastComptrB = -1;
      static uint32_t lastDecompteConf = 0;
      switch (etat)
      {
        case 0: // Avancer le robot selon emplacement defini
          lastDecompteConf  = 0;
          delay(500);
          parcourTable();
          delay(500);
          etat = 1;
          break;


        case 1: // Decompte de confirmation
        { 
          if (FrontMontant(0) == 1 )
          {
              etat = 2;
              Serial.println("Choisir le cocktail");
          }
    
          if(millis() - lastDecompteConf >= 500)
          {
            if(lastDecompteConf  == 0)
            {
                for(uint8_t i = 0; i < LONGEUR_STRIPE; i++)
                {
                    strip.setPixelColor(i, strip.Color(255, 0, 0));
                    Serial.print(" O ");
                }

                comptrConfirmation = (LONGEUR_STRIPE - 1);
            }
            else
            { 
                for(int8_t i = comptrConfirmation; i > (comptrConfirmation - BOND_DECOMPTE); i--)
                {
                    strip.setPixelColor(i, strip.Color(0, 0, 0));
                    // Serial.print(" A "); // DEGUB
                }
                comptrConfirmation -= BOND_DECOMPTE;

                if(comptrConfirmation < 0)
                {
                  etat = 0;
                }
            }

            strip.show();

            Serial.print(" | Compteur = ");
            Serial.println(comptrConfirmation);
            
            lastDecompteConf = millis();
          }
        }  
          break;


        case 2: // Choix du cocktail

        // Curseur du cocktail
        if (FrontMontant(1) == HIGH)
        {  
          if(comptrCocktail < NB_COCKTAIL - 1) 
            comptrCocktail++;
        } 
        if (FrontMontant(2) == HIGH)
        {  
          if(comptrCocktail > 0)
            comptrCocktail--;
        }

        // Indicateur du choix du cocktail
        if(comptrCocktail != lastComptrB)
        {
          //Serial.println(comptrCocktail);
          lastComptrB = comptrCocktail;
          
          for(uint8_t i = 0; i < LONGEUR_STRIPE; i++)
          {
            // Serial.println((uint8_t)(ii==comptrCocktail)*255);
            // if(i == comptrCocktail)
            if((i/ BOND_STRIPE) == comptrCocktail)
            {
              strip.setPixelColor(i, strip.Color(couleurCocktail[comptrCocktail][0], couleurCocktail[comptrCocktail][1], couleurCocktail[comptrCocktail][2]));
              Serial.print(" O ");
            }
            else
            {
              strip.setPixelColor(i, strip.Color(0, 0, 0));
              Serial.print(" - ");
            }
          }

          strip.show();

          Serial.print(" | Compteur = ");
          Serial.println(comptrCocktail);  
        } 

        if (EtatBTN(1) == HIGH && EtatBTN(2) == HIGH)
        {
          etat = 0;
          lastComptrB = 1;
          comptrCocktail = 0;
        }

        if (FrontMontant(0) == 1 )
        {
          //Sortir plateau
          FonctionServo(0, 20);
          etat = 3;
          for (uint8_t j = 0; j < LONGEUR_STRIPE; j++)
          {
            strip.setPixelColor(j, strip.Color(0, 0, 0));
          }
          strip.show();
          lastDecompteConf  = millis();
          comptrConfirmation = 0; 
          // Serial.println("Transition 2 -> 3"); // DEBUG
        }
          break;


        case 3: // Preparation du cocktail
          // Effet visuel preparation cocktail (avec DEL)
          if(millis() - lastDecompteConf >= 50)
          {
            uint32_t chrono = micros();
            lastDecompteConf  = millis();
            if(comptrConfirmation < LONGEUR_STRIPE)
            {
              strip.setPixelColor(comptrConfirmation, strip.Color(couleurCocktail[comptrCocktail][0], couleurCocktail[comptrCocktail][1], couleurCocktail[comptrCocktail][2]));
            }
            else
            {
              strip.setPixelColor(comptrConfirmation%LONGEUR_STRIPE, strip.Color(0,0,0));
            }
            strip.show();    

            comptrConfirmation = ++comptrConfirmation%(LONGEUR_STRIPE*2);
            Serial.println(micros() - chrono);
          }

          
        
          delay(100);
          // Verser le cocktail
          if(comptrConfirmation >= 40)
          {
            FonctionServo(0,22);
            switch(comptrCocktail)
            {
              case 0:
              Serial.println("Cocktail 1 fucker"); // rouge 
                melange_1();
                break;
              case 1:
              Serial.println("Cocktail 2 fucker");
                melange_2();
                break;
              case 2:
              Serial.println("Cocktail 3 fucker");
                melange_3();
                break;
            }
            comptrCocktail = 0;
            lastComptrB = 1;
            playMusique();
            FonctionServo(0,160);
            FonctionServo(0,158);

            etat=0;
          }
          break;


        case 4: // Etat du reservoir
          // Si vide ...

          // Continuer la run
          Serial.println("Boom");
          etat = 10;
        default:
          break;
      
    
      // Outputs en parallele
      // GererMusique();
    }    
  }
} // end loop.

int Pourcentage(uint8_t valeur)
{
  int x = map(valeur,0,100,0,255);
  return x;
}

void melange_2() // Vert
{
  // primer les pompes au debut 500ms.
  analogWrite(POMPE_A_PWM,Pourcentage(100)); // lautre
  delay(100);
  analogWrite(POMPE_B_PWM,Pourcentage(80)); // yan
  delay(500);
  // starter le melange
  analogWrite(POMPE_A_PWM,Pourcentage(70)); // lautre
  analogWrite(POMPE_B_PWM,Pourcentage(40)); // yan
  delay(2000);
  analogWrite(POMPE_A_PWM,0);
  analogWrite(POMPE_B_PWM,0);
}

void melange_1()
{
  // primer les pompes au debut 500ms.
  analogWrite(POMPE_A_PWM,Pourcentage(100)); // lautre
  delay(100);
  analogWrite(POMPE_B_PWM,Pourcentage(80)); // yan
  delay(500);
  // starter le melange
  analogWrite(POMPE_A_PWM,Pourcentage(40)); // lautre
  analogWrite(POMPE_B_PWM,Pourcentage(40)); // yan
  delay(1000);
  analogWrite(POMPE_A_PWM,0);
  delay(1000);
  analogWrite(POMPE_B_PWM,0);
}

void melange_3()
{
  // primer les pompes au debut 500ms.
  analogWrite(POMPE_A_PWM,Pourcentage(100)); // lautre
  delay(100);
  analogWrite(POMPE_B_PWM,Pourcentage(80)); // yan
  delay(500);
  // starter le melange
  analogWrite(POMPE_A_PWM,Pourcentage(100)); // lautre
  analogWrite(POMPE_B_PWM,Pourcentage(40)); // yan
  delay(1000);
  analogWrite(POMPE_B_PWM,0);
  delay(1500);
  analogWrite(POMPE_A_PWM,0);
}
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
