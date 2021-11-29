#include <Arduboy2.h>

//this has got to be unnecessary haha
#define UINT16_MAX 65535

//how long to freeze on a new state
#define NEW_STATE_WAIT 15

#define TITLE 0
#define GAMEPLAY 1

Arduboy2 ab;
int state;
int16_t newStateTimer;
uint16_t globalTimer;

void setState(int state_)
{
  newStateTimer = NEW_STATE_WAIT;
  state = state_;
void incrementTimers()
{
  if (++globalTimer >= UINT16_MAX)
    globalTimer = 0;
bool decrementNewStateTimer()
{
  if (newStateTimer > 0)
  {
    --newStateTimer;
    return true;
  }
  else
  {
    return false;
  }
}

void handleLogs()
{
}


void handlePlayer()
{
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

  //if newStateTimer > 0, perform only partial actions
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
      if (!decrementNewStateTimer())
      {
        handlePlayer();
        handleLogs();

        if(ab.justPressed(B_BUTTON))
          setState(TITLE);
      }

      break;
  }

  ab.display();
}
