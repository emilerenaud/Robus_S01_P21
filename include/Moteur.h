#ifndef MOTEUR_H
#define MOTEUR_H

#include <Arduino.h> 
#include <LibRobus.h>

#define ON 1
#define OFF 0

const float Kp = 0.0001f;
const float Ki = 0.00002f;

const int PulsePerTurn = 3200;
const float Pi = 3.14159f;
const float InchesToMm = 25.4f;
const float WheelDiameter = 3 * InchesToMm;
const float WheelCircumference = WheelDiameter * Pi;
const float radiusRobot = 187; //mm rayon cerle robot
const float CONVERSION_DEGRE_RAD = 2*Pi/360;

void moteur_init(void);
bool avancerDistance(int distanceMM);
void set_wantedSpeed(bool side, float vitesse);
float calculPID(float speed, float currentPulse, int errorTotal, int totalLoop);
int32_t avancer(bool onOff);
int32_t conversion_mmpulse(int32_t mm);
int32_t conversion_pulsemm(int32_t pulse);



#endif