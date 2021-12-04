#include <Arduboy2.h>

//this has got to be unnecessary haha
#define UINT16_MAX 65535

//how long to freeze on a new state
#define NEW_STATE_WAIT 15

//how high gameplayTimer goes
#define GAMEPLAY_TIMER_MAX 7200.0

#define TITLE 0
#define GAMEPLAY 1

#define LOGS_MAX 8

//horizontal acceleration on holding a direction
#define ACCEL_GRAV_OFF 3
#define ACCEL_GRAV_ON 1
//max horizontal velocity
#define MAX_HSPEED 10
//vertical acceleration
#define G_GRAV_ON 6
#define VERT_AIR_RES_GRAV_ON 2
#define TERMINAL_VEL 24
//initial upward velocity on a jump
#define JUMP_INIT_VEL -16
//how many ACCEL_TICKS a jump can be unaffected by gravity for
#define JUMP_INIT_TIMER 4
//how often (frames) to evaluate acceleration
#define ACCEL_TICKS 5

#define SCREEN_WD HEIGHT

//co-ords have precision of 1/16 a pixel. stored as int, calculate
//on-screen location by dividing by 16.
#define PREC 16

Arduboy2 ab;
int state;
int16_t newStateTimer;
uint16_t globalTimer;

//currently controls difficulty in frequency of log spawns, and their width.
uint16_t gameplayTimer;

void setState(int state_)
{
  newStateTimer = NEW_STATE_WAIT;
  state = state_;

  resetPlayer();
  resetLogs();
  gameplayTimer = 0;
}

void incrementTimers()
{
  if (++globalTimer >= UINT16_MAX)
    globalTimer = 0;

  if (gameplayTimer < GAMEPLAY_TIMER_MAX)
    ++gameplayTimer;
}

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

  bool gravity{false};

  //is it just spawned, hence suspended in midair?
  bool suspended{};

  int standingOnLog();
  bool checkGravity();
  void applyGroundFriction();
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

//update and return a mob's gravity bool
bool Mob::checkGravity()
{
  if (suspended)
  {
    gravity = false;
  }
  else if (gravity)
  {
    //check if moving down and close enough to 'stick' to a log
    if (int newLog; dy >= 0 && (newLog = standingOnLog()) != -1)
    {
      dy = logs[newLog].dy;//(this is kind of a weird place to do it)
      y = logs[newLog].y - h*PREC;//b/c currently can be a pixel up or down, and
                                  //misaligned subpixels cause slight jitter as they
                                  //drop on different frames
      gravity = false;
    }
  }
  else
  {
    //check for log. if not, turn on gravity
    if (standingOnLog() == -1)
    {
      gravity = true;
    }
  }

  return gravity;
}

//currently inefficient but adequate. could be replaced with a lookup table.
void Mob::applyGroundFriction()
{
  dx = (float)dx * 3/4;
}

//different handling to other mobs
void applyPlayerAccel()
{
  if (player.gravity)
  {
    //do not slow with sideways 'air resistance'. 
    if (ab.pressed(UP_BUTTON))
    {
      player.dx -= ACCEL_GRAV_ON;
    }
    else if (ab.pressed(DOWN_BUTTON))
    {
      player.dx += ACCEL_GRAV_ON;
    }

    if (player.jumpTimer == 0)
      player.dy += G_GRAV_ON;
  }
  else
  {
    if (ab.pressed(UP_BUTTON))
    {
      if (player.dx > 0)
        player.dx /= 2;
      player.dx -= ACCEL_GRAV_OFF;
    }
    else if (ab.pressed(DOWN_BUTTON))
    {
      if (player.dx < 0)
        player.dx /= 2;
      player.dx += ACCEL_GRAV_OFF;
    }
    else
    {
      player.applyGroundFriction();
    }
  }

  if (player.dx < -MAX_HSPEED)
    player.dx = -MAX_HSPEED;//better to use some gradual air-res slowdown? probably not

  if (player.dx > MAX_HSPEED)
    player.dx = MAX_HSPEED;

  if (player.dy > TERMINAL_VEL)
    player.dy = max(player.dy - VERT_AIR_RES_GRAV_ON, TERMINAL_VEL);

  if (player.jumpTimer > 0)
  {
    if (ab.pressed(A_BUTTON))
      player.jumpTimer -= 1;
    else
      player.jumpTimer = 0;
  }
}

void jump()
{
  player.dy = JUMP_INIT_VEL;
  player.gravity = true;
  player.jumpTimer = JUMP_INIT_TIMER;
}

void controlPlayer()
{
  if (!player.gravity)
  {
    if (ab.justPressed(A_BUTTON))
    {
      jump();
    }
  }

  if (player.suspended)
    if (ab.justPressed(UP_BUTTON | DOWN_BUTTON | A_BUTTON))
      player.suspended = false;
}

void movePlayer()
{
  player.x += player.dx;
  player.y += player.dy;

  if (player.x + player.w < 0)
    player.x += SCREEN_WD * PREC;

  if (player.x + player.w >= SCREEN_WD)
    player.x -= SCREEN_WD * PREC;
}

//if player goes OOB
void respawnPlayer()
{
  //glitch: if left/right is held here, then jump, weird things.
  if (player.y > SCREEN_HT*PREC)
  {
    resetPlayer();
    resetLogs();
    gameplayTimer = 0;//slightly awkward place to put it perhaps
  }
}

void drawPlayer()
{
}

void handlePlayer()
{
  player.checkGravity();

  if (globalTimer % ACCEL_TICKS == 0)
    applyPlayerAccel();

  controlPlayer();
  movePlayer();
  respawnPlayer();
  drawPlayer();
}

void resetPlayer()
{
  player.w = PLAYER_WD;
  player.h = PLAYER_HT;
  player.x = (SCREEN_WD - player.w)/2 * PREC;
  player.y = SCREEN_HT/2 * PREC;
  player.dx = 0;
  player.dy = 0;
  player.gravity = false;
  player.suspended = true;
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
