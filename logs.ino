#include <Arduboy2.h>

//this has got to be unnecessary haha
#define UINT16_MAX 65535

#define TITLE 0
#define GAMEPLAY 1

Arduboy2 ab;
int state;
uint16_t globalTimer;

void setState(int state_)
{
  state = state_;
void incrementTimers()
{
  if (++globalTimer >= UINT16_MAX)
    globalTimer = 0;
}

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

  incrementTimers();

  switch (state)
  {
    case TITLE:
      ab.setCursor(0,0);
      ab.print("TITLE");

      if (!decrementNewStateTimer())
      {
        if(ab.justPressed(A_BUTTON | B_BUTTON))
          setState(GAMEPLAY);
      }

      break;

    case GAMEPLAY:
        if(ab.justPressed(B_BUTTON))
          setState(TITLE);

      break;
  }

  ab.display();
}
