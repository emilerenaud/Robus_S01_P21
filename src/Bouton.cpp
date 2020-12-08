#include "Bouton.h" 
  
// PINS BOUTONS
#define BTN_DEL_ROUGE 40
#define BTN_DEL_BLEU 38
#define BTN_DEL_JAUNE 39

const uint8_t BOUTONS[] = {BTN_DEL_ROUGE, BTN_DEL_BLEU, BTN_DEL_JAUNE};
const uint8_t SIZE_BTN = sizeof(BOUTONS)/sizeof(BOUTONS[0]);

uint8_t buttonStates[SIZE_BTN];
bool buttonRises[SIZE_BTN]; 


// Debounce variables
unsigned long lastDebounceTimes[SIZE_BTN];
unsigned long debounceDelay = 50;


void InitBouton(){
  for( uint8_t i = 0; i < SIZE_BTN; i++)
  { 
    lastDebounceTimes[i] = millis();
    buttonStates[i] = LOW ;
    pinMode(BOUTONS[i], INPUT);
  }
}

// Fonction debouce
void GererBouton(){ 
   
   for( uint8_t i = 0; i < SIZE_BTN; i++)
   {     
 
     static uint8_t lastButtonStates[SIZE_BTN] = {LOW};
     uint8_t reading = digitalRead(BOUTONS[i]);
     buttonRises[i] = 0;
     if (reading != lastButtonStates[i]) {
       lastDebounceTimes[i] = millis();
     }

    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
  
    if (reading != buttonStates[i]) {
      buttonStates[i] = reading;
      // Serial.println(buttonStates[i]); //DEBUG
      buttonRises[i] = buttonStates[i];
     }
    }
    lastButtonStates[i] = reading;
   }
 
}

uint8_t EtatBTN(uint8_t id_btn){
  return buttonStates[id_btn];
}

bool FrontMontant(uint8_t id_btn){
  return buttonRises[id_btn];
}