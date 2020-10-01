#include <Arduino.h>
#include <librobus.h>
// #include <math.h>

// Prototypes de fonction
float speedLeft = 0.2;
float speedRight = 0.2;
int32_t encoderLeft = 0; // changer pour int32 normal
int32_t encoderRight = 0;
// PID shit
int16_t consigne = 800; // Je veux 800 pulses environs. = 1/4 de tour
uint16_t cycle = 0;
float Kp = 0.0001;
float Ki = 0.0001;

uint8_t once = 1;

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
  if(once)
  {
    MOTOR_SetSpeed(LEFT,speedLeft);
    MOTOR_SetSpeed(RIGHT,speedRight);
    delay(200);
    encoderLeft = ENCODER_ReadReset(LEFT);
    encoderRight = ENCODER_ReadReset(RIGHT);
    // MOTOR_SetSpeed(LEFT,0); 
    // MOTOR_SetSpeed(RIGHT,0);
    speedLeft += ((consigne - encoderLeft) * Kp); // Faire attention quand consigne est plus petit que encodeur. C'est bizzare.
    speedRight += ((consigne - encoderRight) * Kp);

    Serial.println("Encodeur Left : " + String(encoderLeft) + " |  Encodeur Right : " + String(encoderRight));
    Serial.println("Error Left : " + String(consigne - encoderLeft) + " |  Error Right : " + String(consigne - encoderRight));
    Serial.println("Speed Left : " + String(speedLeft) + " |  Speed Right : " + String(speedRight));
    //  once = 0;
  }
}

