// Flappy bird for ncurses, with no signals - Ryan Brooks, ryan@hack.net
//

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>


//#define DEBUG

#define DELAY 100		// main game time unit, milliseconds
#define bigG -.3  	// Gravitational accel  in this universe in Characters per delay unit^2
#define BIRD 'b'    // Bird character
#define WING '/'    // Flapping bird character
#define PILLAR '#'  // Pillar character
#define END 1000    // Total number of pillars to traverse
#define GAP 35      // Gap between pillars in characters  
#define WIDTH 5     // Pillar width, must be >2
#define GATE .25    // Gates are 25% of the screen
#define SPEED 2     // Columns scrolled per time unit
#define FLAP 1.1    // Flap "inertia cancellation"

// Globals

int max_x, max_y;
int *pillar;
int gate;
int score;  

// Function names are self explanatory

void restoreScreen() {

  usleep(10000);
  clear();
  curs_set(TRUE);
  refresh();
  echo();
  endwin();

}

void initScreen() {

 initscr();   // Global var `stdscr` is created by the call to `initscr()`  
 noecho();   
 curs_set(FALSE);
 start_color();
 clear();
 cbreak();
 getmaxyx(stdscr, max_y, max_x);
 if(max_y<18 || max_x<40) {
    restoreScreen();
    printf("Screen is too small.\n");
    exit(-1);
 }
 nodelay(stdscr, TRUE);

}


void buildLevel() {

  int i;

  // Initialize game

  srand(time(0));
  score=0;

  pillar = (int *)malloc(sizeof(int)*(END+1)); 
  if(pillar == NULL )  {
    printf("Couldn't allocate %i bytes, failing.\n",END+1);
    restoreScreen();
    exit(-1);
  }
  
  // Build game map (random)

  for(i=0;i<END;i++) pillar[i] = (1.0-GATE)*(float)max_y*(float)rand()/RAND_MAX;     

}


void drawChar(char c, int x, int y) 
{

	move(y,x);
	addch(c);

}

void waitForKey()
{

  getchar();

}

void endGame() 
{

      beep();
      move(max_y/2, max_x/2-6);
      printw(" G A M E  O V E R ");
      move(max_y-3, max_x/2-7);
      printw(" Press any key to quit ");
      refresh();
      sleep(1);
      flushinp();
      waitForKey();
      restoreScreen();
      free(pillar);
      exit(0);

}


void drawBird(int x, int y) 
{

  if(mvinch(y,x)==PILLAR) endGame();    // collision!  To do add collisions with numbers
  drawChar(BIRD,x,y);                  // draw new one

}


void drawBirdFlapping(int x, int y) 
{

  if(mvinch(y,x)==PILLAR) endGame();    // collision!  To do add collisions with numbers
  drawChar(WING,x,y);                  // draw new one

}


void drawPillars(int position) 
{
int i;  // loop for pillarz
int j;  // loop for bricks in pillar
int mul;  // pillar counter
int loc; // interior of pillar counter for loop


for(i=0;i<END+1;i++) {        // This loop runs through the all the map columns, and draws pillars that are on the screen.
  
  loc = max_x-WIDTH+i*(GAP+WIDTH);

  if(loc>position && loc<position+max_x) {
           // onscreen, so draw              
           // stalagmite                 
    for(mul=0;mul<pillar[i];mul++)  for(j=0;j<WIDTH;j++) if(loc-position+j<max_x) drawChar(PILLAR,loc-position+j,mul);                     
           // stalagtite
    for(mul=pillar[i]+gate;mul<max_y;mul++) for(j=0;j<WIDTH;j++) if(loc-position+j<max_x) drawChar(PILLAR,loc-position+j,mul);   

    }

  }

}


void drawScore(int position)
{

  int adjustedScore;

  adjustedScore = position/(WIDTH+GAP)+1-(max_x-WIDTH+GAP)/(WIDTH+GAP);   // TEMP this lags a bit
  if(adjustedScore<0) adjustedScore=0;

  move(3,max_x/2);
  printw("%04i",adjustedScore);

}

void intro() 
{

  move(max_y/2-2, max_x/2-10);
  printw("Welcome to ncurses Flappy Bird");
  move(max_y/2+2,max_x/2-3);
  printw("ryan@hack.net");
  move(max_y/2+4,max_x/2-3);
  printw("Instructions");
  move(max_y/2+5,max_x/2-9);
  printw("Press w to flap, or x to exit.");
  move(max_y/2+7,max_x/2-9);
  printw("Any key to start.  Get Ready!");
  refresh();
  getchar();

}

// Main Game

int main() {

  int position;
	int key;
	int birdx, birdy;
  float velocity;
  int ch;

 initScreen();
 intro();
 refresh();
 buildLevel(); 

 birdy = .67*max_y;	     // start the bird up at 2/3rds of the screen vertically
 birdx = .15*max_x;	     // put bird 15% in to the screen.
 gate = GATE*max_y;
 velocity = 0.0;		    // not falling, yet.
 position = 0;          // this is the horizontal position in the map.



 while(1) {				  // MAIN LOOP

 	ch = getch();	    // No wait check for press

 	switch(tolower(ch)) {

		case 'x' :      // X keypress (Exit)
      free(pillar);
			restoreScreen();
			#ifdef DEBUG
  			printf("Clean exit\n");
        printf("Random max was %u.\n",RAND_MAX);
        printf("Screen was opened with max x %i, max y %i\n", max_x,max_y);
        printf("Bird was last at x %i, y %i\n",birdx,birdy);
        printf("with last velocity %f.\n",velocity);
        printf("First pillar: %i, second pillar: %i, last pillar: %i.\n",pillar[0],pillar[1],pillar[END]);
        printf("End position on map: %i\n", position);
        printf("Gate size was %i\n.", gate);
			#endif
			return 0;

		case 'w':        // W keypress  (Flap)
      if(velocity>0) velocity=velocity/FLAP;      // weird bird inertia correction.  Initially, I let a flap just add to velocity, but that
			velocity=velocity+2*bigG;                  // is not how the original game works.  A flap is more magical and helps with gameplay.
      if(birdy>0) birdy--;
      break;                                    // perhaps this could be reimplemented as a terminal velocity, to keep it real.

 	}

 	 velocity = velocity - (float) bigG;	// let's make gravity
 	 birdy = (float) birdy + velocity;    // apply force to bird

   position=position+SPEED;             // * You're moving forward regardless, that's the game.*

 	 if(birdy >= max_y) {                 // case of bird sitting on bottom of screen
 	 	birdy = max_y-1;
 	 	velocity = 0;
 	 }

   if(birdy < 0) {                      // case of bird hitting top of screen
     birdy=0;
     velocity=0;
   }

   // Render screen layers

   drawPillars(position);
   drawScore(position);
 	 if(tolower(ch)=='w') {
    drawBirdFlapping(birdx, birdy); } 
    else {
    drawBird(birdx, birdy);
  }
 	 refresh();
 	 usleep(DELAY*1000);     
   clear();

 }


}

