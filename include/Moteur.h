#ifndef MOTEUR_H
#define MOTEUR_H

#include <Arduino.h> 
#include <LibRobus.h>

#define ON 1
#define OFF 0
#define FORWARD 1
#define BACKWARD 0

const float Kp = 0.0002f;
const float Ki = 0.00001f;

const int PulsePerTurn = 3200;
const float Pi = 3.14159f;
const float InchesToMm = 25.4f;
const float WheelDiameter = 3 * InchesToMm;
const float WheelCircumference = WheelDiameter * Pi;
const float radiusRobot = 187; //mm rayon cerle robot
const float CONVERSION_DEGRE_RAD = 2*Pi/360;

void moteur_init(void);
int32_t conversion_mmpulse(int32_t mm);
int32_t conversion_pulsemm(int32_t pulse);
void set_wantedSpeed(bool side, float vitesse);
int32_t get_distanceParcouru(void);
void set_distanceParcouru(uint32_t distanceMM);
void set_distanceAFaire(uint32_t distanceMM);
float calculPID(float speed, float currentPulse, int errorTotal, int totalLoop);
void bouger(bool onOff,bool direction);
void bougerDistance(int32_t distanceMM, bool direction);
void tourner(bool direction, float angle);



#endif