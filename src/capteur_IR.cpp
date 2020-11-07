#include <capteur_IR.h>

bool Infrarouge ()

{
  if (ROBUS_ReadIR(0) <= 340)       // Doit être brancher dans le 0
  {
    return 0;   // < 15cm
  }
  else
  {
    return 1;   // < 15cm
  }
}