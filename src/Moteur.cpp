#include <Moteur.h>


float wantedSpeed[2] = {30,30};
int32_t distanceParcouruPulse = 0;
int32_t distanceAFaire = 0;

void moteur_init()
{
    MOTOR_SetSpeed(0,0);
    MOTOR_SetSpeed(1,0);
    ENCODER_Reset(LEFT);
    ENCODER_Reset(RIGHT);
}

// // Convertion mm en pulse.
int32_t conversion_mmpulse(int32_t mm)
{
  return(mm*PulsePerTurn/WheelCircumference);
}

int32_t conversion_pulsemm(int32_t pulse)
{
  return(pulse*WheelCircumference/PulsePerTurn);
}


void set_wantedSpeed(bool side, float vitesse)
{
    wantedSpeed[side] = vitesse;
}

int32_t get_distanceParcouru()
{
  return conversion_pulsemm(distanceParcouruPulse);
}

void set_distanceParcouru(uint32_t distanceMM)
{
  distanceParcouruPulse = conversion_mmpulse(distanceMM);
}

void set_distanceAFaire(uint32_t distanceMM)
{
  distanceAFaire = conversion_mmpulse(distanceMM);
}

float calculPID(float speed, float currentPulse, int errorTotal, int totalLoop)
{
  float resultatK, resultatI, currentError;

  currentError = speed - currentPulse;

  // Calcul K
  resultatK =  currentError * Kp;

  // Calcul I
  // errorTotal += currentError;
  if(totalLoop != -1)
  {
    resultatI = (errorTotal / totalLoop) * Ki;
  }
  else
  {
    resultatI = 0;
  }

  return (resultatK + resultatI);
}


void bouger(bool onOff,bool direction)
{
  static bool initAvancer = 1;
  static int32_t errorTotalLeft = 0;
  static int32_t errorTotalRight = 0;
  static float speedLeft = 0.20;
  static float speedRight = 0.20;
  static int32_t totalLoop = 1;
  int32_t encoderLeft = 0; 
  int32_t encoderRight = 0;
  static uint8_t directionRoue = 0;

  if(onOff == ON)
  {
    if(initAvancer == 1)
    {
      initAvancer = 0;
      errorTotalLeft = 0;
      errorTotalRight = 0;
      totalLoop = 1;
      ENCODER_Reset(0);
      ENCODER_Reset(1);
      
    }

    if(direction == FORWARD)
    {
      directionRoue = 1;
    }
    else if(direction == BACKWARD)
    {
      directionRoue = -1;
    }

    encoderLeft = abs(ENCODER_ReadReset(LEFT));
    encoderRight = abs(ENCODER_ReadReset(RIGHT));
    distanceParcouruPulse += (encoderLeft + encoderRight) / 2; // faire la moyenne des 2 pcq les vitesses vont changer un peu.

    // PID shit.
    errorTotalLeft += wantedSpeed[LEFT] - encoderLeft; // incrementer error total pour I.
    errorTotalRight += wantedSpeed[RIGHT] - encoderRight; // incrementer error total pour I
    speedLeft += calculPID(wantedSpeed[LEFT], encoderLeft, errorTotalLeft, totalLoop);
    speedRight += calculPID(wantedSpeed[RIGHT], encoderRight, errorTotalRight, totalLoop);

    totalLoop ++; // Increase a loop count for PID (I)

    // set new speed
    MOTOR_SetSpeed(LEFT,speedLeft * directionRoue);
    MOTOR_SetSpeed(RIGHT,speedRight * directionRoue);
  } // end On
  else
  {
    initAvancer = 1;
    MOTOR_SetSpeed(LEFT,0);   // Stop motor.
    MOTOR_SetSpeed(RIGHT,0);  // Stop motor.
    ENCODER_Reset(LEFT);
    ENCODER_Reset(RIGHT);
  } // end Off

}

void bougerDistance(int32_t distanceMM, bool direction)
{
  int32_t lastMillis = 0;
  set_distanceParcouru(0);
  while(get_distanceParcouru() <= distanceMM)
  {
    if(millis() - lastMillis >= 10)
    {
      lastMillis = millis();
      bouger(ON,direction);
    }
  }
  bouger(OFF,direction);
}


void tourner(bool direction, float angle)
{
  int32_t pulse = conversion_mmpulse(abs(angle /2) * CONVERSION_DEGRE_RAD * radiusRobot);
  int32_t lastMillis2 = 0;
  int8_t directionLeft = 0;
  int8_t directionRight = 0;
  
  if(direction == LEFT)
  {
    directionLeft = -1;
    directionRight = 1;
  }
  else
  {
    directionLeft = 1;
    directionRight = -1;
  }
  ENCODER_ReadReset(LEFT);
  ENCODER_ReadReset(RIGHT);
  int32_t encoderLeft = abs(ENCODER_Read(LEFT));
  int32_t encoderRight = abs(ENCODER_Read(RIGHT));

  MOTOR_SetSpeed(LEFT,0.25 * directionLeft);
  MOTOR_SetSpeed(RIGHT,0.25 * directionRight);
  while(pulse >= encoderLeft && pulse >= encoderRight)
  {
    if(millis() - lastMillis2 >= 10) // live a 10ms
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
  }
  MOTOR_SetSpeed(LEFT,0);
  MOTOR_SetSpeed(RIGHT,0);
  ENCODER_Reset(LEFT);
  ENCODER_Reset(RIGHT);
}
