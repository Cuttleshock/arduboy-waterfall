#include <Arduboy2.h>

#define TITLE 0
#define GAMEPLAY 1

Arduboy2 ab;
int state;
void setState(int state_)
{
  state = state_;
}


void setup()
{
  ab.begin();
  ab.initRandomSeed();
  ab.setFrameRate(60);

  setState(TITLE);

  ab.clear();
}

void loop()
{
  if (!ab.nextFrame())
    return;

  ab.clear();
  ab.pollButtons();

  switch (state)
  {
    case TITLE:
      break;

    case GAMEPLAY:
      break;
  }

  ab.display();
}
