#include <FastLED.h>
#include <TM1637Display.h>

// ================= LED CONFIG =================
#define NUM_LEDS 50
#define DATA_PIN 13

CRGB leds[NUM_LEDS];

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
int spot = 0;

int score = 0;
int combo = 0;

int baseSpeed = 80;

// ================= SETUP =================
void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  pinMode(BTN, INPUT_PULLUP);

  display.setBrightness(5);

  Serial.begin(9600);
  Serial.println("=== Cyclone START ===");
}

// ================= LOOP =================
void loop() {

  // IDLE TIMEOUT (2 MIN)
  if (millis() - lastInputTime > 120000 && gameState != 0) {
    Serial.println("Idle Reset");
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

  showPress(); // 👈 FIX: show PRESS instead of score

  if (digitalRead(BTN) == LOW) {
    delay(300);
    Serial.println("Button Pressed → Game Start");
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

  int targetSize = max(1, 4 - (level / 3));

  if (millis() - gameTimer > speed) {
    gameTimer = millis();

    if (findRandom) {
      spot = random(2, NUM_LEDS - 2);
      findRandom = false;

      Serial.print("New Target: ");
      Serial.println(spot);
    }

    drawTarget(targetSize);
    moveRunner();

    FastLED.show();
  }

  // BUTTON PRESS
  if (digitalRead(BTN) == LOW) {
    delay(200);
    lastInputTime = millis();

    // ✅ FIXED HIT DETECTION
    int hitPos = Position - 1;
    if (hitPos < 0) hitPos = NUM_LEDS - 1;

    int dist = abs(hitPos - spot);
    dist = min(dist, NUM_LEDS - dist);

    Serial.print("Hit: ");
    Serial.print(hitPos);
    Serial.print(" Spot: ");
    Serial.print(spot);
    Serial.print(" Dist: ");
    Serial.println(dist);

    if (dist <= targetSize) {
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

    int index = spot + i;

    // ✅ wrap fix
    if (index < 0) index += NUM_LEDS;
    if (index >= NUM_LEDS) index -= NUM_LEDS;

    if (i == 0)
      leds[index] = CRGB::Green;
    else
      leds[index] = CRGB::Orange;
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

  Serial.print("WIN | Score: ");
  Serial.print(score);
  Serial.print(" Level: ");
  Serial.println(level);

  flashColor(CRGB::Green);

  findRandom = true;
}

// ================= LOSE =================
void loseGame() {
  Serial.println("DEAD");

  showDead();
  flashColor(CRGB::Red);
  resetGame();
}

// ================= DISPLAY =================

// 0--0 message
void showPress() {
  uint8_t data[] = {
    display.encodeDigit(0), // 0
    0x40,                   // -
    0x40,                   // -
    display.encodeDigit(0)  // 0
  };
  display.setSegments(data);
}

// DEAD message
void showDead() {
  uint8_t data[] = {
    display.encodeDigit(13),
    display.encodeDigit(14),
    display.encodeDigit(10),
    display.encodeDigit(13)
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