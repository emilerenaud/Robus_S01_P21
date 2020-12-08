#ifndef BOUTON_H_
#define BOUTON_H_

#include <Arduino.h>

void InitBouton();
void GererBouton();
uint8_t EtatBTN(uint8_t id_btn);
bool FrontMontant(uint8_t id_btn);

#endif //BOUTON_H