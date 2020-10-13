#include <Arduino.h>
#include <librobus.h>
#include <math.h>

#define PULSE_PER_TURN 3200
#define WHEEL_DIAMETER 76.2 // 3inch in mm
#define DISTANCE_ENTRE_ROUE 184 // 184mm
const float wheelCircumference = WHEEL_DIAMETER * M_PI;
const float pulsePerMM = PULSE_PER_TURN / wheelCircumference;

// Acceleration 
#define START_SPEED 15  // 10 pulses / 10ms
#define TOP_SPEED 60   // 100 pulses / 10ms


// PID shit
float Kp = 0.0002;
float Ki = 0.0001;

uint8_t once = 1;
int32_t distanceAFaire = 1000;




// Prototype de fonction
// float calculP(float currentPulse);
// float calculI(float currentPulse, int errorTotal, int totalLoop);
float calculPID(float wantedSpeed, float currentPulse, int errorTotal, int totalLoop);
float acceleration(uint32_t currentPulseDistance, uint32_t wantedDistanceInPulse);
bool forward(int lenght);
bool turn(bool side, float angle);

void setup() 
{
  BoardInit();
  Serial.begin(9600);
  Serial.println("Program start here");
  ENCODER_Reset(LEFT);
  ENCODER_Reset(RIGHT);
}

void loop() 
{
  static uint8_t step = 1;

  switch(step)
  {
    case(1):
        if(forward(200) == 0)
        {
          step ++;
        }
      break;
    case(2):
        if(turn(LEFT,90) == 0)
        {
          step ++;
        }
      break;
    case(3):
        if(forward(200) == 0)
        {
          step ++;
        }
      break;
    case(4):
        if(turn(LEFT,90) == 0)
        {
          step ++;
        }
      break;
    case(5):
        if(forward(200) == 0)
        {
          step ++;
        }
      break;
      case(6):
        if(turn(LEFT,90) == 0)
        {
          step ++;
        }
      break;
    case(7):
        if(forward(200) == 0)
        {
          step ++;
        }
      break;
    case(8):
        if(turn(LEFT,90) == 0)
        {
          step ++;
        }
      break;
  }
}

/*

  Calcul PID

*/

float calculPID(float wantedSpeed, float currentPulse, int errorTotal, int totalLoop)
{
  float resultatK, resultatI, currentError;

  currentError = wantedSpeed - currentPulse;

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

/*
  Acceleration
  first 25% , ramp up. 
  last 25%. ramp down.
*/
float acceleration(uint32_t currentPulseDistance, uint32_t wantedDistanceInPulse)
{
  float wantedSpeed = 0;
  uint8_t ratio = (((float)currentPulseDistance / (float)wantedDistanceInPulse) * 100); // ratio sur 100%
  uint32_t distanceRamp = wantedDistanceInPulse * 0.40; // Find 25% of the total distance.

  if(ratio >=0 && ratio <= 40) // ramp up.
  {
    wantedSpeed = (((TOP_SPEED - START_SPEED) / (float)distanceRamp) * (float)currentPulseDistance) + START_SPEED;
    // Serial.println("  WantedSpeed = " + String(wantedSpeed));
  }
  else if(ratio >=  60 && ratio <=100) // ramp down
  {
    wantedSpeed = (-(((TOP_SPEED - START_SPEED) / (float)distanceRamp) * (currentPulseDistance - (0.60 * wantedDistanceInPulse))) + TOP_SPEED);
    // Serial.println("  WantedSpeed = " + String(wantedSpeed));

  }
  else
  {
    wantedSpeed = TOP_SPEED;
  }
  return wantedSpeed;
}

/*

  drive forward with a lenght. in mm.

*/
bool forward(int lenght)
{
  static uint32_t lenghtInPulse = 0;
  static uint32_t currentPulseDistance = 0;
  static uint32_t lastMillis = 0;
  static int32_t errorTotalLeft = 0;
  static int32_t errorTotalRight = 0;
  static int32_t totalLoop = 1;
  static float speedLeft = 0.20;
  static float speedRight = 0.20;
  static float wantedSpeed = 30;
  int32_t encoderLeft = 0; 
  int32_t encoderRight = 0;

  lenghtInPulse = lenght * pulsePerMM; // convert lenght (mm) -> lenght in pulse
  if(lenght == 0)
  {
    return 0;
  }
  if(currentPulseDistance <= lenghtInPulse)
  {
    if(millis() - lastMillis >= 10) // Calculate the PID & acceleration each 10ms.
    {
      lastMillis = millis();  // "update" counter for 10ms.
      // Serial.println("moteur"); // semble passer assez souvent
      encoderLeft = ENCODER_ReadReset(LEFT);    // Read encoder for positionning
      encoderRight = ENCODER_ReadReset(RIGHT);  
      currentPulseDistance += encoderLeft;      // Increase for total distance.
      wantedSpeed = acceleration(currentPulseDistance, lenghtInPulse);       // Adjust speed. Ramp up & ramp down.
      // Calculate PID
      errorTotalLeft += wantedSpeed - encoderLeft; // incrementer error total pour I.
      errorTotalRight += wantedSpeed - encoderRight; // incrementer error total pour I
      speedLeft += calculPID(wantedSpeed, encoderLeft, errorTotalLeft, totalLoop);
      speedRight += calculPID(wantedSpeed, encoderRight, errorTotalRight, totalLoop);

      totalLoop ++; // Increase a loop count for PID (I)
      
      // set new speed
      MOTOR_SetSpeed(LEFT,speedLeft);
      MOTOR_SetSpeed(RIGHT,speedRight);
    }
    return 1; // not finish yet.
  }
  else // Reset var for next movement
  {
    MOTOR_SetSpeed(LEFT,0);   // Stop motor.
    MOTOR_SetSpeed(RIGHT,0);  // Stop motor.
    ENCODER_Reset(LEFT);
    ENCODER_Reset(RIGHT);
    totalLoop = 1;
    currentPulseDistance = 0;
    errorTotalLeft = 0;
    errorTotalRight = 0;
  }
  return 0; // Return 0 when the movement is completed.
}


/* 

  Turn

*/
bool turn(bool side, float angle)
{
  // find distance 
  static float wantedSpeed = 0.25; // turn at 0.3 speed.
  static int32_t currentPulse = 0;
  static float distanceArcPulse = 0;
  static bool resetEncoder = 1;
  static bool movementCompleted = 0;

  // reset encoder for better precision.
  if(resetEncoder == 1)
  {
    distanceArcPulse = ((M_PI * DISTANCE_ENTRE_ROUE) / 180) * angle * pulsePerMM;
    Serial.println(distanceArcPulse);
    resetEncoder = 0;
    ENCODER_Reset(LEFT);
    ENCODER_Reset(RIGHT);
    delay(1);
  }
  if(side == LEFT) // turn left. left wheel dont move. Right wheel move.
  {
    // while(ENCODER_Read(RIGHT) <= distanceArcPulse) // marche pas mieux.
    // {
    //   MOTOR_SetSpeed(LEFT,0);
    //   MOTOR_SetSpeed(RIGHT,wantedSpeed);
    // }
    // MOTOR_SetSpeed(RIGHT,0);
    // ENCODER_Reset(RIGHT);
    // return 0;

    if(currentPulse <= distanceArcPulse) // vas surment plus overshoot que under.
    {
      // wantedSpeed = acceleration(currentPulse,distanceArcPulse);
      MOTOR_SetSpeed(LEFT,0);
      MOTOR_SetSpeed(RIGHT,wantedSpeed);
      // if(ENCODER_Read(RIGHT) != 0) // The loop is too fast. If the encoder read more than 0, read_reset the encoder.
      // {
      currentPulse = ENCODER_Read(RIGHT);

      // }
      return 1;
    }
    else
    {
      MOTOR_SetSpeed(RIGHT,0);
      movementCompleted = 1;
    }
  }
  else if(side == RIGHT)
  {
    if(currentPulse <= distanceArcPulse) // -5 . half the error of 10 ? maybe.
    {
      MOTOR_SetSpeed(LEFT,wantedSpeed);
      MOTOR_SetSpeed(RIGHT,0);
      // if(ENCODER_Read(LEFT) != 0) // The loop is too fast. If the encoder read more than 0, read_reset the encoder.
      // {
        currentPulse = ENCODER_Read(LEFT);
      // }
      delay(3);
      return 1;
    }
    else
    {
      movementCompleted = 1;
    }
  }
  if(movementCompleted == 1)
  {
    MOTOR_SetSpeed(LEFT,0); // just in case.
    MOTOR_SetSpeed(RIGHT,0);
    ENCODER_Reset(LEFT);
    ENCODER_Reset(RIGHT);
    Serial.println(currentPulse);
    currentPulse = 0;
    resetEncoder = 1; // for next turn.
    movementCompleted = 0; // reset for the next turn.
    return 0; // return 0 when the movement is completed.
  }
  return 0; // return 0 just in case.
}

