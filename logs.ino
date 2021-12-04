#include <Arduboy2.h>

//this has got to be unnecessary haha
#define UINT16_MAX 65535

//how long to freeze on a new state
#define NEW_STATE_WAIT 15

#define TITLE 0
#define GAMEPLAY 1

#define LOGS_MAX 8

#define SCREEN_WD HEIGHT

//co-ords have precision of 1/16 a pixel. stored as int, calculate
//on-screen location by dividing by 16.
#define PREC 16

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

struct Log
{
  bool render{false};

  int x{};//subpx
  int y{};//subpx
  int len{};//px
  int dx{};//subpx
  int dy{};//subpx

  //behaviour
};
Log logs[LOGS_MAX];

void handleLogs()
{
struct Mob
{
  int x{};//subpx
  int y{};//subpx
  int w{};//px
  int h{};//px
  int dx{0};//subpx
  int dy{0};//subpx

  int standingOnLog();
};
Mob player;

//near-enough collision check with subpixels. return -1 for 'no'.
int Mob::standingOnLog()
{
  for (int i=0; i<LOGS_MAX; ++i)
  {
    if (!logs[i].render)
      continue;

    //is the log's surface within +/- 1 vertical pixel of the mob's base?
    if (   logs[i].y + PREC >= y + h*PREC
        && logs[i].y - PREC <= y + h*PREC )
    {
      //is the mob or its shadow horizontally on?
      if ((   logs[i].x                    <= x + w*PREC
           && logs[i].x + logs[i].len*PREC >= x )
       || (   logs[i].x                    <= x + w*PREC + SCREEN_WD*PREC
           && logs[i].x + logs[i].len*PREC >= x + SCREEN_WD*PREC)
         )
      {
        return i;
      }
    }
  }

  return -1;
}

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
