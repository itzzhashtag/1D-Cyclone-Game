#include <FastLED.h>
#include <TM1637Display.h>

// ================= LED CONFIG =================
#define NUM_LEDS 50
#define DATA_PIN 13

#define SCORE_PIN 6
#define SCORE_LEDS 4

CRGB leds[NUM_LEDS];
CRGB sleds[SCORE_LEDS];

// ================= DISPLAY =================
#define CLK 9
#define DIO 10
TM1637Display display(CLK, DIO);

// ================= BUTTON =================
#define BTN 12

// ================= GAME =================
byte gameState = 0;
byte Position = 0;
byte level = 1;

unsigned long lastInputTime = 0;
unsigned long gameTimer = 0;

bool findRandom = true;
byte spot = 0;

int score = 0;
int combo = 0;

// Dynamic speed
int baseSpeed = 80;

// ================= SETUP =================
void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812B, SCORE_PIN, GRB>(sleds, SCORE_LEDS);

  pinMode(BTN, INPUT_PULLUP);

  display.setBrightness(5);
  display.showNumberDec(0);

  Serial.begin(9600);
}

// ================= LOOP =================
void loop() {

  // IDLE TIMEOUT (2 MIN)
  if (millis() - lastInputTime > 120000 && gameState != 0) {
    resetGame();
  }

  if (gameState == 0) {
    idleMode();
  } else {
    playMode();
  }
}

// ================= IDLE MODE =================
void idleMode() {
  fill_rainbow(leds, NUM_LEDS, millis() / 10, 10);
  FastLED.show();

  display.showNumberDec(score);

  if (digitalRead(BTN) == LOW) {
    delay(300);
    startGame();
  }
}

// ================= START =================
void startGame() {
  clearLEDS();
  score = 0;
  combo = 0;
  level = 1;
  Position = 0;
  findRandom = true;
  gameState = 1;
  lastInputTime = millis();
}

// ================= PLAY =================
void playMode() {

  int speed = baseSpeed - (level * 5);
  if (speed < 15) speed = 15;

  int targetSize = max(1, 4 - (level / 3)); // shrinks over levels

  if (millis() - gameTimer > speed) {
    gameTimer = millis();

    if (findRandom) {
      spot = random(2, NUM_LEDS - 2);
      findRandom = false;
    }

    drawTarget(targetSize);
    moveRunner();

    FastLED.show();
  }

  // BUTTON PRESS
  if (digitalRead(BTN) == LOW) {
    delay(200);
    lastInputTime = millis();

    if (abs(Position - spot) <= targetSize) {
      winRound();
    } else {
      loseGame();
    }
  }

  display.showNumberDec(score);
}

// ================= DRAW TARGET =================
void drawTarget(int size) {
  clearLEDS();

  for (int i = -size; i <= size; i++) {
    if (i == 0)
      leds[spot + i] = CRGB::Green;
    else
      leds[spot + i] = CRGB::Orange;
  }
}

// ================= RUNNER =================
void moveRunner() {
  leds[Position] = CRGB::Red;

  Position++;
  if (Position >= NUM_LEDS) Position = 0;
}

// ================= WIN =================
void winRound() {
  combo++;

  int points = 10 + (combo * 2);
  score += points;

  level++;

  flashColor(CRGB::Green);

  findRandom = true;
}

// ================= LOSE =================
void loseGame() {
  showDead();
  flashColor(CRGB::Red);
  resetGame();
}

// ================= DEAD DISPLAY =================
void showDead() {
  uint8_t data[] = {
    display.encodeDigit(13), // D
    display.encodeDigit(14), // E
    display.encodeDigit(10), // A
    display.encodeDigit(13)  // D
  };
  display.setSegments(data);
  delay(1500);
}

// ================= RESET =================
void resetGame() {
  gameState = 0;
  score = 0;
  combo = 0;
  level = 1;
}

// ================= EFFECT =================
void flashColor(CRGB color) {
  for (int i = 0; i < 2; i++) {
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(200);
    clearLEDS();
    FastLED.show();
    delay(200);
  }
}

// ================= CLEAR =================
void clearLEDS() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}