#include <Moteur.h>


float wantedSpeed[2] = {30,30};


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


int32_t avancer(bool onOff)
{
  static bool initAvancer = 1;
  static int32_t distancePulse = 0;
  static int32_t errorTotalLeft = 0;
  static int32_t errorTotalRight = 0;
  static float speedLeft = 0.20;
  static float speedRight = 0.20;
  static int32_t totalLoop = 1;
  int32_t encoderLeft = 0; 
  int32_t encoderRight = 0;

  if(onOff == ON)
  {
    if(initAvancer == 1)
    {
        initAvancer = 0;
        distancePulse = 0;
        errorTotalLeft = 0;
        errorTotalRight = 0;
        totalLoop = 1;
        ENCODER_ReadReset(0);
        ENCODER_ReadReset(1);
    }

    encoderLeft = ENCODER_ReadReset(LEFT);
    encoderRight = ENCODER_ReadReset(RIGHT);
    distancePulse += (encoderLeft + encoderRight) / 2; // faire la moyenne des 2 pcq les vitesses vont changer un peu.

    // PID shit.
    errorTotalLeft += wantedSpeed[LEFT] - encoderLeft; // incrementer error total pour I.
    errorTotalRight += wantedSpeed[RIGHT] - encoderRight; // incrementer error total pour I
    speedLeft += calculPID(wantedSpeed[LEFT], encoderLeft, errorTotalLeft, totalLoop);
    speedRight += calculPID(wantedSpeed[RIGHT], encoderRight, errorTotalRight, totalLoop);

    totalLoop ++; // Increase a loop count for PID (I)

    // set new speed
    MOTOR_SetSpeed(LEFT,speedLeft);
    MOTOR_SetSpeed(RIGHT,speedRight);
  }
  else
  {
    initAvancer = 1;
    MOTOR_SetSpeed(LEFT,0);   // Stop motor.
    MOTOR_SetSpeed(RIGHT,0);  // Stop motor.
    ENCODER_Reset(LEFT);
    ENCODER_Reset(RIGHT);
    distancePulse = 0;
  }
  return conversion_pulsemm(distancePulse);
}
