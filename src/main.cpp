#include <Arduino.h>
#include <librobus.h>
#include <math.h>

#define PULSE_PER_TURN 3200
#define WHEEL_DIAMETER 76.2 // 3inch in mm
const int wheelCircumference = WHEEL_DIAMETER * M_PI;
const float pulsePerMM = PULSE_PER_TURN / wheelCircumference;

// const float Pi = 3.14159f;
// const float WheelCircumference = WheelDiameter * Pi;


float speedLeft = 0.20;
float speedRight = 0.20;
int32_t encoderLeft = 0; // changer pour int32 normal
int32_t encoderRight = 0;

// PID shit
float Kp = 0.0002;
float Ki = 0.0001;

uint8_t once = 1;
uint32_t distanceVoulu = 2000;

// Acceleration
float startSpeed = 10;  // 20 pulse / 10ms
float wantedSpeed = 50;
float topSpeed = 100;   // 100 pulse / 10ms    

// Prototype de fonction
float calculP(float currentPulse);
float calculI(float currentPulse, int errorTotal, int totalLoop);
void acceleration(uint32_t);

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
  static int32_t errorTotalLeft = 0;
  static int32_t errorTotalRight = 0;
  static int32_t totalLoop = 1;
  static int32_t lastMillis = 0;
  static int32_t currentPulseDistance = 0;

  if(currentPulseDistance < (distanceVoulu * pulsePerMM))
  {
    if(millis() - lastMillis >= 10)
    {
      lastMillis = millis();

      MOTOR_SetSpeed(LEFT,speedLeft);
      MOTOR_SetSpeed(RIGHT,speedRight);
      delay(10);
      encoderLeft = ENCODER_ReadReset(LEFT);
      encoderRight = ENCODER_ReadReset(RIGHT);
      currentPulseDistance += encoderLeft;
      acceleration(currentPulseDistance);
      // // MOTOR_SetSpeed(LEFT,0); 
      // // MOTOR_SetSpeed(RIGHT,0);
      // // Calcul I
      // // Calcul P
      speedLeft += calculP(encoderLeft);
      speedRight += calculP(encoderRight);
      speedLeft += calculI(encoderLeft,errorTotalLeft,totalLoop);
      speedRight += calculI(encoderRight,errorTotalRight,totalLoop);
      totalLoop ++;
      // speedLeft += ((consignePulse - encoderLeft) * Kp); 
      // speedRight += ((consignePulse - encoderRight) * Kp);

      // Serial.println("Encodeur Left : " + String(encoderLeft) + " |  Encodeur Right : " + String(encoderRight));
      // Serial.println("Error Left : " + String(consignePulse - encoderLeft) + " |  Error Right : " + String(consignePulse - encoderRight));
      // Serial.println("Speed Left : " + String(speedLeft) + " |  Speed Right : " + String(speedRight));
      //  once = 0;
    }
  }
  else
  {
    MOTOR_SetSpeed(LEFT,0);
    MOTOR_SetSpeed(RIGHT,0);
  }
  
  
  
}

/*
  Calcul P pour le PID
*/
float calculP(float currentPulse)
{
 return (wantedSpeed - currentPulse) * Kp;
}

/*
  Calcul I pour le PID
*/
float calculI(float currentPulse, int errorTotal, int totalLoop)
{
  float error = wantedSpeed - currentPulse;
  errorTotal += error;
  return (errorTotal / totalLoop) * Ki;
}

/*
  Acceleration
  first 25% , ramp up. 
  last 25%. ramp down.
*/
void acceleration(uint32_t currentPulseDistance)
{
  uint8_t ratio = (currentPulseDistance / (distanceVoulu * pulsePerMM)) * 100; // ratio sur 100%
  uint32_t distanceRamp = (distanceVoulu * pulsePerMM) * 0.25;
  if(ratio >=0 && ratio <= 25) // ramp
  {
    wantedSpeed = (((topSpeed - startSpeed) / distanceRamp) * currentPulseDistance) + startSpeed;
    // Serial.print("Ratio : " + String(ratio) + "  |  " + String(currentPulseDistance) + "  | ");
    // Serial.println((((topSpeed - startSpeed) / distanceRamp) * currentPulseDistance) + startSpeed);
  }
  else if(ratio >= 75 && ratio <=100) // ramp down
  {
    wantedSpeed = (-(((topSpeed - startSpeed) / distanceRamp) * (currentPulseDistance - (0.75 *distanceVoulu * pulsePerMM))) + topSpeed);
    // Serial.println(-(((topSpeed - startSpeed) / distanceRamp) * (currentPulseDistance - (0.75 *distanceVoulu * pulsePerMM))) + topSpeed);
  }
  else
  {
    wantedSpeed = topSpeed;
  }
  
}

