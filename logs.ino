#include <Arduboy2.h>

Arduboy2 ab;

void setup()
{
  ab.begin();
  ab.initRandomSeed();
  ab.setFrameRate(60);
  ab.clear();
}

void loop()
{
  if (!ab.nextFrame())
    return;

  ab.clear();
  ab.pollButtons();

  ab.display();
}
