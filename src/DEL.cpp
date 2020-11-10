#include <DEL.h>

void del_init()
{
  //Configurer # pins Arduino sortie digitale
  pinMode(pinRouge, OUTPUT);
  pinMode(pinBleu, OUTPUT);
  pinMode(pinJaune, OUTPUT);  

  //Mise à zéro pin Adrduino
  digitalWrite(pinRouge, 0);   
  digitalWrite(pinBleu, 0);
  digitalWrite(pinJaune, 0); 
}

// enum Color {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};

void gererDEL(Color capteurcouleur){

  if(capteurcouleur == BLACK){
  // rien faire
  }
  else if(capteurcouleur == RED){

    //Allumer DEL rouge
    digitalWrite(pinRouge, 1);

    //Fermer les autres
    digitalWrite(pinJaune, 0);
    digitalWrite(pinBleu, 0);
  }
  else if(capteurcouleur == YELLOW){

    //Allumer DEL jaune
      digitalWrite(pinJaune, 1);

    //Fermer les autres
    digitalWrite(pinRouge, 0);
    digitalWrite(pinBleu, 0);
  }
  else if(capteurcouleur == CYAN){

    //Allumer DEL bleu
    digitalWrite(pinBleu, 1);

    //Fermer les autres
    digitalWrite(pinRouge, 0);
    digitalWrite(pinJaune, 0);
  }
}
  void toggle_led()         // Alumer DEL quand il voit une ligne
  {
    digitalWrite(41,1);
    delay(50);
    digitalWrite(41,0);
  }
