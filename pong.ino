#include <RGBmatrixPanel.h>
#include <gamma.h>
#include <math.h>
#include <Fonts/Picopixel.h>
#include "pong.h"

long randNumber;

byte scoreToWin = 3;

int cpuReactionTime;
int cpuPaddleSpeed;

int maxBallSpeed;

double vX = 1;
double vY = -0.75;

double x = 25;
double y = 10;

int joystickX;
int joystickY;

int playerPaddlePos = 10;
int computerPaddlePos = 10;

bool playerHitTheBallLast = true;

bool computerScored;
bool playerScored;

byte playerScore;
byte computerScore;

bool paused = false;

bool debug = false;

int ballColor = WHITE;

int backgroundColor = BLACK;


RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);
String createPicker(char* title, char ** options, byte numberOfOptions) {
  bool done = false;

  byte selected = 0;

  while (!done) {

    // display menu

    matrix.setCursor(0, 4);

    matrix.setTextColor(WHITE);
    matrix.println(title);

    for (int i = 0; i < numberOfOptions; i++) {
      matrix.setTextColor(WHITE);
      if (i == selected) {
        matrix.setTextColor(GREEN);
      }

      matrix.println(options[i]);
    }

    // Get the x and y joystick values.
    joystickY = map(analogRead(yaxis), 0, 1023, 100, -100);

    bool btn1Pressed = analogRead(btn1) == 0;

    Serial.println(btn1Pressed);

    if (btn1Pressed) {
      matrix.fillScreen(backgroundColor);
      done = true;
      return options[selected];
    }

    // down
    if (joystickY <= -30) {
      selected++;
    }

    // up
    if (joystickY >= 30) {
      selected--;
    }

    if (selected < 1) {
      selected = 0;
    }

    if (selected > numberOfOptions - 1) {
      selected = numberOfOptions - 1;
    }

    delay(300);
  }
}

bool isBtnPressed(byte button) {
  if (button == 1) {
    return analogRead(btn1) == 0;
  }
  return false;
}

void loop() {
  randomSeed(analogRead(0));
  delay(50);

  // Clear the previous ball's location by settings the area to backgroundColor.
  matrix.fillCircle(x, y, BALL_RADIUS, backgroundColor);

  // Update the ball's position by adding the velocity.
  x += vX;
  y += vY;

  joystickCheck();

  autoMoveComputerPaddle();

  hitWalls();

  // Draw center line.
  matrix.drawLine(0, 32, 32, 32, WHITE);

  // Draw the player's score.
  matrix.setCursor(0, 39);
  matrix.print(playerScore);

  // Draw the computer's score.
  matrix.setCursor(0, 29);
  matrix.print(computerScore);

   matrix.fillRect(computerPaddlePos - 4, CPU_PAD_Y, 4, 3, backgroundColor);
  matrix.fillRect(computerPaddlePos + PADDLE_WIDTH, CPU_PAD_Y, 4, 3, backgroundColor);

    // Draw the player's paddle.
  matrix.fillRect(playerPaddlePos - 3, PLYR_PAD_Y, 2, 3, backgroundColor);
  matrix.fillRect(playerPaddlePos + PADDLE_WIDTH, PLYR_PAD_Y, 2, 3, backgroundColor);

  // Draw the ball.
  matrix.fillCircle(x, y, BALL_RADIUS, ballColor);


  matrix.fillRect(playerPaddlePos, PLYR_PAD_Y, PADDLE_WIDTH, 3, matrix.Color333(0, 255, 255));

  if (debug) {
    matrix.drawLine(0, 60, 32, 60, backgroundColor);
    matrix.drawPixel(playerPaddlePos, 60, BLUE);
    matrix.drawPixel(playerPaddlePos + 1, 60, BLUE);
    matrix.drawPixel(playerPaddlePos + 2, 60, BLUE);
    matrix.drawPixel(playerPaddlePos + 8, 60, RED);
    matrix.drawPixel(playerPaddlePos + 7, 60, RED);
    matrix.drawPixel(playerPaddlePos + 6, 60, RED);
    matrix.drawPixel(playerPaddlePos + 4, 60, GREEN);
    matrix.drawPixel(playerPaddlePos + 3, 60, GREEN);
    matrix.drawPixel(playerPaddlePos + 5, 60, GREEN);
  }

  // Draw the computer's paddle.
  matrix.fillRect(computerPaddlePos, CPU_PAD_Y, PADDLE_WIDTH, 3, matrix.Color333(255, 255, 0));

  checkIfBallHitPaddle();

  // If a player scored, reset the values, then show a countdown.
  if (playerScored || computerScored) {
    playerScored = false;
    computerScored = false;
    showCountdown(3, 1, 1000);
  }
} // end loop

void setup() {
  matrix.begin();
  Serial.begin(9600);

  pinMode(xaxis, INPUT);
  pinMode(yaxis, INPUT);
  pinMode(btn1, INPUT);

  // Set the screen to be in portrait mode.
  matrix.setRotation(1);

  // Set the font to a super tiny font.
  matrix.setFont(&Picopixel);

  char * options[] = {"Easy", "Medium", "Hard"};

  String result = createPicker("Play: ", options, 3);

  cpuReactionTime = 45;
  cpuPaddleSpeed = 2;
  if (result == "Hard") {
    Serial.println("hard");
    cpuReactionTime = 64;
    cpuPaddleSpeed = 3;
    maxBallSpeed = 4;
  }
  if (result == "Medium") {
    cpuReactionTime = 30;
    cpuPaddleSpeed = 2;
    maxBallSpeed = 3;
  }
  if (result == "Easy") {
    cpuReactionTime = 10;
    cpuPaddleSpeed = 1;
    maxBallSpeed = 2;
  }
}

// Resets the game after a score.
void resetGame() {

  playerHitTheBallLast = true;

  vX = 1;
  vY = -0.75;

  matrix.fillScreen(backgroundColor);

  x = 16;
  y = 32;

  playerPaddlePos = 10;
  computerPaddlePos = 10;
}

// Shows a countdown on the screen.
void showCountdown(int from, int to, int delayMs) {

  for (int i = from; i > to - 1; i--) {
    matrix.fillRect(5, 5, 8, 8, backgroundColor);
    matrix.setCursor(7, 12);
    matrix.print(i);
    delay(delayMs);
  }

  matrix.fillRect(5, 5, 8, 8, backgroundColor);
}

void scored() {
  if (playerScored) {
    playerScore += 1;
  } else {
    computerScore += 1;
  }

  matrix.setCursor(5, 12);   // start at top left, with one pixel of spacing
  matrix.setTextSize(1);    // size 1 == 8 pixels high

  matrix.setTextColor(WHITE);
  matrix.print("Score!");

  delay(3000);
  resetGame();
}


void checkIfBallHitPaddle() {
  if (y >= 60 - BALL_RADIUS) {
    if (checkIfBallHitPlayerPaddle()) {
      Serial.println("Ball hit player's paddle.");
      reflectBall();
    }
  }

  if (y <= 3 + BALL_RADIUS) {
    if (checkIfBallHitComputerPaddle()) {
      Serial.println("Ball hit computer's paddle.");
      reflectBall();
    }
  }
}

void reflectBall() {
  vY *= -1;
  vX *= 1.15;
  if (vX > maxBallSpeed) {
    vX = maxBallSpeed;
  }
  if (vX < -maxBallSpeed) {
    vX = -maxBallSpeed;
  }

  vY *= 1.15;
  if (vY > maxBallSpeed) {
    vY = maxBallSpeed;
  }
  if (vY < -maxBallSpeed) {
    vY = -maxBallSpeed;
  }
}

// handles joystick input
void joystickCheck() {

  // Get the x and y joystick values.
  joystickY = map(analogRead(yaxis), 0, 1023, 100, -100);
  joystickX = map(analogRead(xaxis), 0, 1023, 100, -100);

  // right
  if (joystickX >= 30) {
    matrix.fillRect(playerPaddlePos, PLYR_PAD_Y, 2, 3, backgroundColor);
    playerPaddlePos += 2;
  }

  // down
  if (joystickY <= -30) {

  }

  // up
  if (joystickY >= 30) {

  }

  // left
  if (joystickX <= -30) {
    matrix.fillRect(playerPaddlePos + PADDLE_WIDTH,  PLYR_PAD_Y, 2, 5, backgroundColor);
    playerPaddlePos -= 2;
  }

  if (playerPaddlePos < 0) {
    playerPaddlePos = 0;
  }

  if (playerPaddlePos > 32 - PADDLE_WIDTH) {
    playerPaddlePos = 32 - PADDLE_WIDTH;
  }
}

// Controls the computer's paddle. Goes towards the ball at all times.
void autoMoveComputerPaddle() {
  int middleOfPaddle = computerPaddlePos + PADDLE_WIDTH / 2;
  int v = cpuPaddleSpeed;

  if (y > cpuReactionTime) {
    v = 1;
  }
  if (x > middleOfPaddle) {
    matrix.fillRect(computerPaddlePos, CPU_PAD_Y, 2, 3, backgroundColor);
    computerPaddlePos += v;
  } else {
    matrix.fillRect(computerPaddlePos + PADDLE_WIDTH, CPU_PAD_Y, 2, 5, backgroundColor);
    computerPaddlePos -= v;
  }
}

// checks if the ball hits a paddle
bool checkIfBallHitComputerPaddle() {

  // checks if the ball his the player's paddle
  if (x > computerPaddlePos && x < computerPaddlePos + PADDLE_WIDTH) {
    if (!playerHitTheBallLast)
      return false;

       if (x > computerPaddlePos && x < computerPaddlePos + PADDLE_WIDTH / 3) {
      Serial.println("left hit");
      ballColor = BLUE;
      if (vX < 0) {
         vX *= 1.15;
      }else {
        vX *= -1.15;
      }
    }

    if (x > computerPaddlePos + 3 && x < computerPaddlePos + PADDLE_WIDTH / 3) {
      Serial.println("middle hit");
      ballColor = GREEN;
    }

    if (x > computerPaddlePos + 6) {
      Serial.println("right hit");
      ballColor = RED;

      if (vX < 0) {
         vX *= -1.15;
      }else {
        vX *= 1.15;
      }
    }
      
    playerHitTheBallLast = false;
    return true;
  }

  // ball did not hit a paddle - SCORE?
  return false;
}

// checks if the ball hits a paddle
bool checkIfBallHitPlayerPaddle() {

  // checks if the ball his the player's paddle
  if (x > playerPaddlePos && x < playerPaddlePos + PADDLE_WIDTH) {
    if (playerHitTheBallLast)
      return false;

      

    if (x > playerPaddlePos && x < playerPaddlePos + PADDLE_WIDTH / 3) {
      Serial.println("left hit");
      ballColor = BLUE;
      if (vX < 0) {
         vX *= 1.15;
      }else {
        vX *= -1.15;
      }
    }

    if (x > playerPaddlePos + 3 && x < playerPaddlePos + PADDLE_WIDTH / 3) {
      Serial.println("middle hit");
      ballColor = GREEN;
    }

    if (x > playerPaddlePos + 6) {
      Serial.println("right hit");
      ballColor = RED;

      if (vX < 0) {
         vX *= -1.15;
      }else {
        vX *= 1.15;
      }
    }

    



    playerHitTheBallLast = true;
    return true;
  }

  // ball did not hit a paddle - SCORE?
  return false;
}

void hitWalls() {
  if (y <= 0) {
    playerScored = true;
    scored();
  }

  if (y >= 64) {
    computerScored = true;
    scored();
  }

  if (x <= -1 + BALL_RADIUS * 2 || x > 32 - BALL_RADIUS * 2) {

    if (x < 0) {
      x = 5;
    }
    
    vX = vX * -1;
  }
}
