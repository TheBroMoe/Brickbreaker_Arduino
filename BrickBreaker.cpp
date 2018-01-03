/*  Brickbreaker
By Mohammad Kebbi
Student iD: 1496572
The goal of the game is player must smash a wall of bricks by deflecting a bouncing ball with a paddle.
The paddle may move horizontally and is controlled with the joystick, the
When all the bricks have been destroyed, a victory screen is displayed.

Acknowledgements:
	*	Used code developed in class for the paddles movement
  - Refer to paddleMovement and redrawPaddle functions

	* Received Help from Jason Cannon on how to develop two dimensional arrays
  - Refer to structure bricks as well as brickArray

	* For paddle collision, received help from Songhui Zhang
  -Refer to paddleCollisionTest
*/

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>

// These are the four touchscreen analog pins
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 5   // can be a digital pin
#define XP 4   // can be a digital pin

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

// joystick pins
#define JOY_VERT_ANALOG A1
#define JOY_HORIZ_ANALOG A0
#define JOY_SEL 2

// width/height of the display when rotated horizontally
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// constants for the joystick
#define JOY_DEADZONE 64
#define JOY_CENTER 512
#define JOY_SPEED 128
//#define JOY_STEPS_PER_PIXEL 64

//Block definitions
int brickWidth= 32;
int brickHeight= 16;
#define brickRow 5 //There are 5 rows of bricks
#define brickCol 10 //There are 10 columns of bricks
#define NUM_BRICKS (brickRow*brickCol) // there are 50 total bricks in the game


// Use hardware SPI (on Mega2560, #52, #51, and #50) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//Paddle Definitions
int paddleX, paddleY;
int prevPaddleX, prevPaddleY;
int paddleWidth=32;
int paddleHeight=5;

//Ball definitions
int ballR= 3; // radius
int ballX; //position of ball in X direction
int ballY; //position of ball in Y direction
int prevballX; // previous position of ballX
int prevballY; //previous position of ballY

// velocities of balls. Ball is incremented by xVel and yVel
int xVel;
int yVel;

//score is meant to keep track of game's progress
int score;

//setDifficulty is the difficulty player chooses
bool setDifficulty;
// Represents if ball is in play
int ballCounter;

bool ballStatus;
// Keeps array of the possible colours of the bricsk
uint16_t colourList[5] = {ILI9341_RED, ILI9341_BLUE, ILI9341_GREEN, ILI9341_ORANGE, ILI9341_MAGENTA};

/* Creation of structure bricks. This holds each compoment of each brick including
  whether or not it's in play, it's location, and it's colour */
struct bricks {
  bool inPlay; // defines if a brick is in play or not
  int Xlocation; //X location of brick
  int Ylocation; // Y location of brick
  uint16_t colour; // colour of brick
};
/* bricksArray of type bricks takes the stucture of bricks and organizes each
brick based on their which row and column they are postioned in. This was done
Using a two dimensional array by the rows and columns of the bricks*/
bricks brickArray[brickRow][brickCol];

// redrawPaddle draws the paddles new position and draws white over it's old position
void redrawPaddle(){
	tft.fillRect(prevPaddleX, prevPaddleY, paddleWidth,paddleHeight, ILI9341_WHITE);

	tft.fillRect(paddleX, paddleY, paddleWidth,paddleHeight, ILI9341_BLACK);
}

// startScreen welcomes players and prepares them for the game
void startScreen(){
	tft.fillScreen(ILI9341_BLACK);
	tft.setCursor(20,100);
	tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
	tft.print("Welcome to Brickbreaker!");
	delay(1000);
	for (int i=2; i>=0;i--){
		tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(140,100);
		tft.setTextColor(ILI9341_WHITE); tft.setTextSize(5);
		tft.print(i+1);
		delay(1000);
	}
}


//defineBricks sets the array and structure for each brick in the game
void defineBricks(){
  for(int i=0; i<brickRow;i++){
    for(int j=0; j< brickCol; j++){

          bricks* curr_brick = &brickArray[i][j];
          curr_brick->inPlay=1; // all bricks will start in play
          curr_brick->Xlocation= brickWidth*j;
          curr_brick->Ylocation= brickHeight*i;
          curr_brick->colour = colourList[i];
    }
  }
}

void setup() {
	init();

	pinMode(JOY_SEL, INPUT_PULLUP);

	Serial.begin(9600);

	tft.begin();

  Serial.println("OK!");

	tft.setRotation(-1);
	tft.setTextWrap(false);

}

// Pre allocate draw functions

void drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);

void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);

//drawBricks function draws all bricks on the screen if they are in play
void drawBricks(){
  for(int i=0; i<brickRow;i++){
    for(int j=0; j< brickCol; j++){
      if(brickArray[i][j].inPlay==1){
        tft.fillRect(brickArray[i][j].Xlocation, brickArray[i][j].Ylocation,brickWidth,brickHeight, brickArray[i][j].colour);
        tft.drawRect(brickArray[i][j].Xlocation,brickArray[i][j].Ylocation,brickWidth,brickHeight, ILI9341_BLACK);
      }
    }
  }
}
// Pre allocate mainMenu function
void mainMenu();
// Displays screen showing user how many lives they have left and repositions all the objects
bool ballsRemaining(){
  ballCounter--;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(50,10);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(3);
  tft.print("Balls Left: " );
  tft.print(ballCounter);
  paddleX= 128;
 	paddleY= 230;
  ballX=160;
  ballY=120;
  prevballX=ballX;
  prevballY=ballY;

  delay(2000);


  tft.fillScreen(ILI9341_WHITE);

	tft.fillRect(paddleX, paddleY, 32,5, ILI9341_BLACK);

	drawBricks();
  return true;
}
// Displays Instructions
void Instructions(){
  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(0,10);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.println("Instructions: Control the ");
  tft.println(" paddle with the joystick");
  tft.println(" to move left and right.");
  tft.println(" Your goal is to destroy ");
  tft.println(" all the bricks and make ");
  tft.println(" sure the ball does ");
  tft.println(" not touch the ground!");
  tft.println(" Good Luck!");
  tft.println();
  tft.println(" Touch the screen to");
  tft.println(" return to main menu");
  // user has to touch screen to break loop and return to main menu
  while(true){
    TSPoint p = ts.getPoint();
    if(p.z > MINPRESSURE ){
      break;
    }
  }
    mainMenu();
}

// Starts the game and puts all the essential pieces where they are meant to be
void startGame(){
  //inital paddleX and paddleY Values
	paddleX= 128;
 	paddleY= 230;
  // initial ball values
  ballX=160;
  ballY=120;
  prevballX=ballX;
  prevballY=ballY;
  // Set the Vel of x and y based on selected difficulty
  if(setDifficulty==0){
    // Ball would increase by increments of 1 if difficulty is 'easy'
    xVel=1;
    yVel= 1;
  }
  else if(setDifficulty==1){
    //Vel is double the value if difficulty is 'hard'
    xVel=2;
    yVel=2;
  }
  ballCounter=2; // player starts with 3 lives (or 2 spare balls)
  score=0; //score starts at 0
  ballStatus=true;

  // define the bricks
  defineBricks();
  //displays welcome screen
	startScreen();

  //draw game screen
	tft.fillScreen(ILI9341_WHITE);
  //Draw paddle in centre of screen
	tft.fillRect(paddleX, paddleY, 32,5, ILI9341_BLACK);
  //Draw the Bricks
	drawBricks();
}

// User starts in this menu. The main menu Shows off the first screen and prompts
// User to make a selection
void mainMenu(){
  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(20,10);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.print("Mohammad Kebbi Presents");

  tft.setCursor(20,50);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(4);
  tft.print("Brickbreaker");

  tft.drawRect(40,125,100,50,ILI9341_BLACK);

  tft.setCursor(65,135);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.print("Play");
  tft.setCursor(65,150);
  tft.print("Game!");

  tft.drawRect(200,125,100,50,ILI9341_BLACK);
  tft.setCursor(215,130);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.print("How To");
  tft.setCursor(225,150);
  tft.print("Play!");

  // Puts user in loop until they make a selection with the touchscreen
  while(true){
    TSPoint p = ts.getPoint();

    if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
			continue; // just go to the top of the loop again
		}
		// Scale from ~0->1000 to tft.width using the calibration #'s
		p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
		p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

    // If user selects startGame box
    if(p.y>=130&&p.y<=205){
      if(p.x>=150&&p.x<=220){

        tft.fillScreen(ILI9341_WHITE);
        tft.setCursor(15,40);
        tft.setTextColor(ILI9341_BLACK); tft.setTextSize(3);
        tft.print("Select Difficulty");

        tft.drawRect(40,125,100,50,ILI9341_BLACK);

        tft.setCursor(55,135);
        tft.setTextColor(ILI9341_BLACK); tft.setTextSize(3);
        tft.print("Easy");

        tft.drawRect(200,125,100,50,ILI9341_BLACK);
        tft.setCursor(215,135);
        tft.setTextColor(ILI9341_BLACK); tft.setTextSize(3);
        tft.print("Hard");
        // puts user in a loop again until they select Difficulty
          while(true){
            TSPoint p = ts.getPoint();

            if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
        			continue; // just go to the top of the loop again
        		}
        		// Scale from ~0->1000 to tft.width using the calibration #'s
        		p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
        		p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
            if(p.y>=130&&p.y<=205){
              if(p.x>=150&&p.x<=220){
                setDifficulty=0;
                break;
              }
            }
            else if(p.y>=10&&p.y<=70){
              if(p.x>=145&&p.x<=205){
                setDifficulty=1;
                break;
              }
            }
          }
        startGame();
        break;
      }
    }
    // If user selects 'How to Play' box
    else if(p.y>=10&&p.y<=70){
      if(p.x>=145&&p.x<=205){
        Instructions();
        break;
      }
    }
  }
}
// Game over screen if player runs out of lives
void gameOver(){
	tft.fillScreen(ILI9341_BLACK);
	tft.setCursor(65,100);
	tft.setTextColor(ILI9341_WHITE); tft.setTextSize(4);
	tft.print("Game Over");
	delay(3000);
  // Return to Main Menu
  mainMenu();
}
// Victory screen if player successfully destroys all bricks
void victory(){
	tft.fillScreen(ILI9341_BLACK);
	tft.setCursor(65,100);
	tft.setTextColor(ILI9341_WHITE); tft.setTextSize(4);
	tft.print("You Win!");
	delay(3000);
  // Return to Main Menu
  mainMenu();
}
// Determines paddle's movement based on movement of joystick
void paddleMovement() {
  int h = analogRead(JOY_HORIZ_ANALOG);

	prevPaddleX=paddleX;
	prevPaddleY=paddleY;

	paddleX += (JOY_CENTER - h) / JOY_SPEED;

	paddleX = constrain(paddleX,0,288);

	if (paddleX != prevPaddleX) {
    redrawPaddle();
  }

  delay(10);
}
// Draws the balls current position
void drawBall(){
  tft.fillCircle(prevballX,prevballY,ballR,ILI9341_WHITE);// draw over old ball position
  tft.fillCircle(ballX,ballY,ballR,ILI9341_BLACK); //draw new ball position
  // redraw paddle at current position so that ball does not white over paddle
  tft.fillRect(paddleX, paddleY, paddleWidth,paddleHeight, ILI9341_BLACK);
}
//updateBall will check for collision and if no collision occurs ball is still in
//play and balls position gets updated
bool updateBall(){
  if(ballX>TFT_WIDTH-2*ballR||ballX<0+2*ballR){ //if ball hits the left or right of the screen
    xVel*=-1;
  }
  if(ballY>TFT_HEIGHT){ // if ball hits bottom border
    return false;
  }
  else if(ballY<= 0+ballR*2){ // if ball hits top border
    yVel*=-1;
  }
  // save balls position
  prevballX=ballX;
  prevballY=ballY;
  //update balls location
  ballX+=xVel;
  ballY+=yVel;
  return true;
}
// Checks if ball collides with paddle
void paddleCollisionTest(){
  // define regions of ball for ease of simplificity
  int ballBottom=ballY+ballR;
  int ballLeft= ballX-ballR;
  int ballRight=ballX+ballR;
  int paddleEdge=paddleX+paddleWidth;

  // if ball is about to hit paddle
  if(ballBottom>paddleY){
    // if ball hits paddle
    if(ballX>=paddleX&&ballX<=paddleEdge){
      // balls velocity is now reversed
      yVel*=-1;
    }
    // cases for if ball collides at specific angles with ball
    // if paddle collides with left edge of paddle
    if(ballRight>=paddleX&&ballLeft<paddleX){
      if(xVel>0){
        xVel*=-1;
      }
      yVel*=-1;
    }
    //if paddle collides with right edge of paddle
    else if(ballLeft<=paddleEdge&&ballRight>paddleEdge){
     if(xVel<0){
       xVel*=-1;
     }
     yVel*=-1;
    }
  }
}

// deleteBlock removes block if block is no longer in play
void deleteBlock(int row, int col){
  tft.fillRect(brickArray[row][col].Xlocation, brickArray[row][col].Ylocation,brickWidth,brickHeight, ILI9341_WHITE); //draw brick as white to erase it
  brickArray[row][col].inPlay=0; // brick is no longer in play
  score++; // add to score
  delay(10);
}

// brickCollisionTest checks if ball has collided with any bricks
void brickCollisionTest(){

  // definitions for simplification
  int ballTop= ballY-ballR;
  int ballBottom=ballY+ballR;
  int ballLeft= ballX-ballR;
  int ballRight=ballX+ballR;
  //Changes the definitions based on the adjustment of xVel and yVel
  if(setDifficulty==1){
     ballTop+=1;
     ballBottom-=1;
     ballLeft-= 1;
     ballRight+=1;
  }
  for(int i=0; i<brickRow; i++){
    for(int j=0; j<brickCol; j++){
      if(brickArray[i][j].inPlay==1){
        // If Ball collides with Top or bottom brick edges
        if(ballBottom==brickArray[i][j].Ylocation||ballTop==brickArray[i][j].Ylocation+brickHeight){
          if(ballRight>brickArray[i][j].Xlocation&&ballLeft<brickArray[i][j].Xlocation+brickWidth){
           deleteBlock(i,j); //delete Block
            yVel*=-1;// reverse y direction
          }
        }
        //If Ball collides with Right or left brick edges
        if(ballRight==brickArray[i][j].Xlocation||ballLeft==brickArray[i][j].Xlocation+brickWidth){
          if(ballBottom>brickArray[i][j].Ylocation&&ballTop<brickArray[i][j].Ylocation+brickHeight){
            deleteBlock(i,j); //delete Block
            xVel*=-1;// reverse x direction
          }
        }
      }
    }
  }
}

// main loop that runs the game
int main(){
	setup();

  mainMenu(); // Start user in mainMenu

	while(true){

    drawBall();//update ball position and check for border collision
    paddleMovement(); //update paddle position
    ballStatus= updateBall(); // check for border collision and update balls position
    paddleCollisionTest();// Check for paddle collision
    brickCollisionTest();// Check for brick collision

    // if ball hits bottom border
    if(ballStatus==false){
      //if ballCounter is 0 player loses
      if(ballCounter==0){
        gameOver();
        continue;// starts at the top of the loop after returning to main menu
      }
      ballStatus=ballsRemaining();
    }
    //if bricks destroyed is equivilent to the total number of bricks, player wins
    if(score==NUM_BRICKS){
      victory();
    }
	}

	Serial.end();

	return 0;
}
