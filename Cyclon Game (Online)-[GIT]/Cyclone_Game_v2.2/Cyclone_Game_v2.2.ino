#include <FastLED.h>
#include <TM1637Display.h>

// ================= CONFIG =================
#define NUM_LEDS 50
#define DATA_PIN 13
#define BTN 12
#define BUZZER 3

#define CLK 9
#define DIO 10

#define IDLE_TIMEOUT 120000

// ================= OBJECTS =================
CRGB leds[NUM_LEDS];
TM1637Display display(CLK, DIO);

// ================= GAME =================
byte gameState = 0;
byte Position = 0;
byte level = 1;

unsigned long lastInputTime = 0;
unsigned long gameTimer = 0;

bool newTarget = true;
int spot = 0;

int score = 0;
int combo = 0;

// ================= SOUND =================
void beep(int f, int d) {
  tone(BUZZER, f, d);
  delay(d);
  noTone(BUZZER);
}

// short sounds
void sPress() { beep(1000, 40); }
void sNear()  { beep(600, 80); }
void sHit()   { beep(1200, 60); beep(1500, 60); }
void sBull()  { beep(2000, 80); beep(2500, 100); }

// 🎵 Mario-style death sound (smooth descending)
void sDead() {
  int notes[] = {900, 800, 700, 600, 500, 400, 300, 200};
  int duration[] = {80, 80, 80, 100, 120, 150, 180, 220};

  for (int i = 0; i < 8; i++) {
    tone(BUZZER, notes[i], duration[i]);
    delay(duration[i] + 20);
  }
  noTone(BUZZER);
}

// ================= SETUP =================
void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  pinMode(BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  display.setBrightness(5);

  Serial.begin(9600);
  Serial.println("Cyclone v3.1 Ready");
}

// ================= LOOP =================
void loop() {

  if (millis() - lastInputTime > IDLE_TIMEOUT && gameState != 0) {
    resetGame();
  }

  if (gameState == 0) idleMode();
  else playMode();
}

// ================= IDLE =================
void idleMode() {
  fill_rainbow(leds, NUM_LEDS, millis() / 10, 10);
  FastLED.show();

  showIdleDisplay();

  if (digitalRead(BTN) == LOW) {
    delay(250);
    sPress();
    startGame();
  }
}

// ================= START =================
void startGame() {
  clearLEDs();

  score = 0;
  combo = 0;
  level = 1;
  Position = 0;

  newTarget = true;
  gameState = 1;
  lastInputTime = millis();

  Serial.println("Game Start");
}

// ================= PLAY =================
void playMode() {

  int speed = max(15, 80 - level * 5);
  int targetSize = max(1, 4 - level / 3);

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

  if (digitalRead(BTN) == LOW) {
    delay(180);
    lastInputTime = millis();
    handleHit(targetSize);
  }

  display.showNumberDec(score);
}

// ================= HIT =================
void handleHit(int targetSize) {

  sPress();

  int hitPos = Position - 1;
  if (hitPos < 0) hitPos = NUM_LEDS - 1;

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

// ================= WIN =================
void win() {
  level++;
  flash(CRGB::Green);
  newTarget = true;
}

// ================= LOSE =================
void lose() {
  Serial.println("DEAD");

  showDead();          // display first
  flash(CRGB::Red);    // visual

  sDead();             // 🎵 full sound (not cut)

  delay(300);          // small pause (natural feel)

  resetGame();
}

// ================= DISPLAY =================
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

// ================= LED =================
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

// ================= RESET =================
void resetGame() {
  gameState = 0;
  score = 0;
  combo = 0;
  level = 1;

  Serial.println("Reset");
}