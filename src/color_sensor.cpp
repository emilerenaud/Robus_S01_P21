#include <color_sensor.h>

String color[8] = {"Black", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White"};
const uint8_t NbColorReads = 10;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

Color getColor(uint16_t red, uint16_t green, uint16_t blue, uint16_t clear)
{
  short temp = 0;
  //float r, g, b;
  uint16_t lowAverage = 1 * (red + green + blue) / 3;

/*
  r = red; r /= clear;
  g = green; g /= clear;
  b = blue; b /= clear;

  if (r > ColorFloor)
    temp += 1;
  if (g > ColorFloor)
    temp += 2;
  if (b > ColorFloor)
    temp += 4;
*/

  if (red > lowAverage)
    temp += 1;
  if (green > lowAverage)
    temp += 2;
  if (blue > lowAverage)
    temp += 4;

  return (Color)temp;
}

Color getColor2(uint16_t r, uint16_t g, uint16_t b, uint16_t c)
{
  short temp = 0;

  r *= (float)cRef / c;
  g *= (float)cRef / c;
  b *= (float)cRef / c;

  if (r > rRef)
    temp += 1;
  if (g > gRef)
    temp += 2;
  if (b > bRef)
    temp += 4;

  return (Color)temp;
}

uint8_t getHighestValue(uint8_t tab[8])
{
  uint8_t highest = 0;
  for (uint8_t i = 0; i < 8; i++)
    if (tab[i] > highest)
      highest = tab[i];
  return highest;
}

Color getTrueColor()
{
  uint16_t clear, red, green, blue;
  uint8_t colors[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  tcs.setInterrupt(false);

  do
  {
    delay(60);
    tcs.getRawData( &red, &green, &blue, &clear);

    colors[getColor2(red, green, blue, clear)]++;
  }
  while (getHighestValue(colors) < NbColorReads);
  
  tcs.setInterrupt(true);

  for (uint8_t i = 0; i < 8; i++)
    if (colors[i] == NbColorReads)
      return (Color)i;

  return BLACK;
  //printColor(red, green, blue, clear);
}

void printColor(uint16_t red, uint16_t green, uint16_t blue, uint16_t clear)
{ 
  Serial.print(red * (float)cRef / clear - rRef);
  Serial.print(" | ");
  Serial.print(green * (float)cRef / clear - gRef);
  Serial.print(" | ");
  Serial.print(blue * (float)cRef / clear - bRef);
  Serial.print(" | ");
  Serial.print(clear);
  Serial.print(" : ");
  Serial.println(color[getColor2(red, green, blue, clear)]);
}

void ColorSensorInit(uint16_t* rRef, uint16_t* gRef, uint16_t* bRef, uint16_t* cRef)
{
  if (tcs.begin()) 
  {
    Serial.println("Found sensor");
  } 
  else 
  {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  tcs.setInterrupt(false);
  delay(60);
  tcs.getRawData( rRef, gRef, bRef, cRef);
  tcs.setInterrupt(true);
}