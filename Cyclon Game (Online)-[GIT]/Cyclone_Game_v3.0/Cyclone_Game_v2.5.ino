#include <FastLED.h>
#include <TM1637Display.h>
#include <EEPROM.h>

// =====================================================
// ================= CONFIG =============================
// =====================================================
#define NUM_LEDS 50
#define DATA_PIN 13

#define BTN 12
#define BUZZER 3

#define CLK 9
#define DIO 10

#define IDLE_TIMEOUT 120000

// =====================================================
// ================= OBJECTS ============================
// =====================================================
CRGB leds[NUM_LEDS];
TM1637Display display(CLK, DIO);

// =====================================================
// ================= GAME STATE =========================
// =====================================================
byte gameState = 0;
byte Position = 0;
byte level = 1;

unsigned long lastInputTime = 0;
unsigned long gameTimer = 0;

bool newTarget = true;
int spot = 0;

int score = 0;
int combo = 0;

// =====================================================
// ================= ADAPTIVE ===========================
// =====================================================
int difficultyBias = 0;
int successStreak = 0;
int failStreak = 0;

// =====================================================
// ================= EEPROM =============================
// =====================================================
int highScore = 0;

// =====================================================
// ================= IDLE DISPLAY =======================
// =====================================================
unsigned long idleTimer = 0;
byte idleState = 0;

// =====================================================
// ================= BUTTON =============================
// =====================================================
bool lastButtonState = HIGH;

bool buttonPressed() {
  bool current = digitalRead(BTN);

  if (lastButtonState == HIGH && current == LOW) {
    delay(20);
    lastButtonState = current;
    return true;
  }

  lastButtonState = current;
  return false;
}

// =====================================================
// ================= SOUND ==============================
// =====================================================
void beep(int f, int d) {
  tone(BUZZER, f, d);
  delay(d);
  noTone(BUZZER);
}

void sPress() { beep(1000, 40); }
void sNear()  { beep(600, 80); }
void sHit()   { beep(1200, 60); beep(1500, 60); }
void sBull()  { beep(2000, 80); beep(2500, 100); }

// Mario-style death
void sDead() {
  int notes[] = {900, 800, 700, 600, 500, 400, 300, 200};
  int duration[] = {80, 80, 80, 100, 120, 150, 180, 220};

  for (int i = 0; i < 8; i++) {
    tone(BUZZER, notes[i], duration[i]);
    delay(duration[i] + 20);
  }
  noTone(BUZZER);
}

// =====================================================
// ================= SETUP ==============================
// =====================================================
void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  pinMode(BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  display.setBrightness(5);

  Serial.begin(9600);

  // Load high score
  EEPROM.get(0, highScore);
  if (highScore < 0 || highScore > 9999) highScore = 0;

  Serial.print("High Score: ");
  Serial.println(highScore);
}

// =====================================================
// ================= LOOP ===============================
// =====================================================
void loop() {

  if (millis() - lastInputTime > IDLE_TIMEOUT && gameState != 0) {
    resetGame();
  }

  if (gameState == 0) idleMode();
  else playMode();
}

// =====================================================
// ================= IDLE ===============================
// =====================================================
void idleMode() {

  fill_rainbow(leds, NUM_LEDS, millis() / 10, 10);
  FastLED.show();

  // Cycle display every 3 sec
  if (millis() - idleTimer > 3000) {
    idleTimer = millis();
    idleState = (idleState + 1) % 3;
  }

  if (idleState == 0) showIdleDisplay();
  else if (idleState == 1) showHS();
  else display.showNumberDec(highScore);

  if (buttonPressed()) {
    sPress();
    startGame();
  }
}

// =====================================================
// ================= START ==============================
// =====================================================
void startGame() {
  clearLEDs();

  score = 0;
  combo = 0;
  level = 1;
  Position = 0;

  difficultyBias = 0;
  successStreak = 0;
  failStreak = 0;

  newTarget = true;
  gameState = 1;
  lastInputTime = millis();

  Serial.println("Game Start");
}

// =====================================================
// ================= PLAY ===============================
// =====================================================
void playMode() {

  int baseSpeed = 80 - level * 3;
  int baseSize  = 4 - level / 4;

  int speed = baseSpeed - (difficultyBias * 5);
  int targetSize = baseSize - (difficultyBias / 2);

  int chaos = random(-2, 3);
  speed += chaos * 3;
  targetSize += chaos / 2;

  speed = constrain(speed, 15, 120);
  targetSize = constrain(targetSize, 1, 5);

  if (millis() - gameTimer > speed) {
    gameTimer = millis();

    if (newTarget) {
      spot = random(2, NUM_LEDS - 2);
      newTarget = false;
    }

    drawTarget(targetSize);
    moveRunner();
    FastLED.show();
  }

  if (buttonPressed()) {
    lastInputTime = millis();
    handleHit(targetSize);
  }

  display.showNumberDec(score);
}

// =====================================================
// ================= HIT ================================
// =====================================================
void handleHit(int targetSize) {

  sPress();

  int hitPos = Position - 1;
  if (hitPos < 0) hitPos = NUM_LEDS - 1;

  int dist = abs(hitPos - spot);
  dist = min(dist, NUM_LEDS - dist);

  if (dist == 0) {
    sBull();
    score += 20;
    combo += 2;
    win();
  }
  else if (dist <= targetSize) {
    sHit();
    score += 10 + combo * 2;
    combo++;
    win();
  }
  else if (dist == targetSize + 1) {
    sNear();
    lose();
  }
  else {
    lose();
  }
}

// =====================================================
// ================= WIN ================================
void win() {
  level++;
  successStreak++;
  failStreak = 0;

  if (successStreak >= 2 && difficultyBias < 5) {
    difficultyBias++;
    successStreak = 0;
  }

  flash(CRGB::Green);
  newTarget = true;
}

// =====================================================
// ================= LOSE ===============================
void lose() {

  // Save high score
  if (score > highScore) {
    highScore = score;
    EEPROM.put(0, highScore);
  }

  failStreak++;
  successStreak = 0;

  if (failStreak >= 2 && difficultyBias > -5) {
    difficultyBias--;
    failStreak = 0;
  }

  showDead();
  flash(CRGB::Red);

  sDead();
  delay(300);

  resetGame();
}

// =====================================================
// ================= DISPLAY ============================

// 0--0
void showIdleDisplay() {
  uint8_t data[] = {
    display.encodeDigit(0),
    0x40,
    0x40,
    display.encodeDigit(0)
  };
  display.setSegments(data);
}

//  H.S  (with dots)
void showHS() {
  uint8_t data[] = {
    0x76, // H
    0x10, // i
    0x6D, // S
    0x50  // r
  };

  display.setSegments(data);
  display.setSegments(data, 4, 0x80); // enable colon
}

// DEAD
void showDead() {
  uint8_t data[] = {
    display.encodeDigit(13),
    display.encodeDigit(14),
    display.encodeDigit(10),
    display.encodeDigit(13)
  };
  display.setSegments(data);
}

// =====================================================
// ================= LED ================================
void drawTarget(int size) {
  clearLEDs();

  for (int i = -size; i <= size; i++) {
    int idx = spot + i;

    if (idx < 0) idx += NUM_LEDS;
    if (idx >= NUM_LEDS) idx -= NUM_LEDS;

    leds[idx] = (i == 0) ? CRGB::Green : CRGB::Orange;
  }
}

void moveRunner() {
  leds[Position] = CRGB::Red;
  Position = (Position + 1) % NUM_LEDS;
}

void flash(CRGB c) {
  for (int i = 0; i < 2; i++) {
    fill_solid(leds, NUM_LEDS, c);
    FastLED.show();
    delay(150);
    clearLEDs();
    FastLED.show();
    delay(150);
  }
}

void clearLEDs() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

// =====================================================
// ================= RESET ==============================
void resetGame() {
  gameState = 0;
  score = 0;
  combo = 0;
  level = 1;
}