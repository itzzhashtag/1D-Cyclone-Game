#include <FastLED.h>
#include <TM1637Display.h>

// =====================================================
// ================= CONFIG =============================
// =====================================================
#define NUM_LEDS 50
#define DATA_PIN 13

#define BTN 12
#define BUZZER 3

#define CLK 9
#define DIO 10

#define IDLE_TIMEOUT 120000   // 2 minutes

// =====================================================
// ================= OBJECTS ============================
// =====================================================
CRGB leds[NUM_LEDS];
TM1637Display display(CLK, DIO);

// =====================================================
// ================= GAME STATE =========================
// =====================================================
byte gameState = 0;   // 0 = idle, 1 = playing
byte Position = 0;
byte level = 1;

unsigned long lastInputTime = 0;
unsigned long gameTimer = 0;

bool newTarget = true;
int spot = 0;

int score = 0;
int combo = 0;

// =====================================================
// ================= ADAPTIVE SYSTEM ====================
// =====================================================
int difficultyBias = 0;   // -5 (easy) to +5 (hard)
int successStreak = 0;
int failStreak = 0;

// =====================================================
// ================= BUTTON (ANTI-HOLD) =================
// =====================================================
// Detects ONLY one click (no repeat on hold)
bool lastButtonState = HIGH;

bool buttonPressed() {
  bool current = digitalRead(BTN);

  // Detect falling edge (HIGH → LOW)
  if (lastButtonState == HIGH && current == LOW) {
    delay(20); // debounce
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

// Sound effects
void sPress() { beep(1000, 40); }
void sNear()  { beep(600, 80); }
void sHit()   { beep(1200, 60); beep(1500, 60); }
void sBull()  { beep(2000, 80); beep(2500, 100); }

// Mario-style death sound
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
  Serial.println("Cyclone v4 Ready");
}

// =====================================================
// ================= LOOP ===============================
// =====================================================
void loop() {

  // Auto reset if idle too long
  if (millis() - lastInputTime > IDLE_TIMEOUT && gameState != 0) {
    resetGame();
  }

  if (gameState == 0) idleMode();
  else playMode();
}

// =====================================================
// ================= IDLE MODE ==========================
// =====================================================
void idleMode() {

  // Rainbow idle animation
  fill_rainbow(leds, NUM_LEDS, millis() / 10, 10);
  FastLED.show();

  showIdleDisplay();

  // Wait for SINGLE button press
  if (buttonPressed()) {
    sPress();
    startGame();
  }
}

// =====================================================
// ================= START GAME =========================
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
// ================= PLAY MODE ==========================
// =====================================================
void playMode() {

  // -------- Adaptive Difficulty --------

  int baseSpeed = 80 - level * 3;
  int baseSize  = 4 - level / 4;

  // Apply skill-based bias
  int speed = baseSpeed - (difficultyBias * 5);
  int targetSize = baseSize - (difficultyBias / 2);

  // Add randomness (arcade feel)
  int chaos = random(-2, 3);
  speed += chaos * 3;
  targetSize += chaos / 2;

  // Clamp safe values
  speed = constrain(speed, 15, 120);
  targetSize = constrain(targetSize, 1, 5);

  // -------- LED Update --------
  if (millis() - gameTimer > speed) {
    gameTimer = millis();

    if (newTarget) {
      spot = random(2, NUM_LEDS - 2);
      newTarget = false;

      Serial.print("Target: ");
      Serial.println(spot);
    }

    drawTarget(targetSize);
    moveRunner();
    FastLED.show();
  }

  // -------- Button Press --------
  if (buttonPressed()) {
    lastInputTime = millis();
    handleHit(targetSize);
  }

  // Show score
  display.showNumberDec(score);
}

// =====================================================
// ================= HIT LOGIC ==========================
// =====================================================
void handleHit(int targetSize) {

  sPress();

  // Get actual visible LED position
  int hitPos = Position - 1;
  if (hitPos < 0) hitPos = NUM_LEDS - 1;

  // Circular distance calculation
  int dist = abs(hitPos - spot);
  dist = min(dist, NUM_LEDS - dist);

  Serial.print("Hit:");
  Serial.print(hitPos);
  Serial.print(" Spot:");
  Serial.print(spot);
  Serial.print(" Dist:");
  Serial.println(dist);

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
// =====================================================
void win() {

  level++;

  successStreak++;
  failStreak = 0;

  // Increase difficulty gradually
  if (successStreak >= 2 && difficultyBias < 5) {
    difficultyBias++;
    successStreak = 0;
    Serial.println("Difficulty ↑");
  }

  flash(CRGB::Green);
  newTarget = true;
}

// =====================================================
// ================= LOSE ===============================
// =====================================================
void lose() {

  Serial.println("DEAD");

  failStreak++;
  successStreak = 0;

  // Reduce difficulty if struggling
  if (failStreak >= 2 && difficultyBias > -5) {
    difficultyBias--;
    failStreak = 0;
    Serial.println("Difficulty ↓");
  }

  showDead();
  flash(CRGB::Red);

  sDead();     // full sound
  delay(300);  // pause before reset

  resetGame();
}

// =====================================================
// ================= DISPLAY ============================
// =====================================================
void showIdleDisplay() {
  uint8_t data[] = {
    display.encodeDigit(0),
    0x40,
    0x40,
    display.encodeDigit(0)
  };
  display.setSegments(data);
}

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
// ================= LED CONTROL ========================
// =====================================================
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
// =====================================================
void resetGame() {
  gameState = 0;
  score = 0;
  combo = 0;
  level = 1;

  Serial.println("Reset");
}