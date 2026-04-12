/*
________________________________________________________________________
  _   _    _    ____  _   _ _____   _     ____ 
 | | | |  / \  / ___|| | | |_   _| / \   / ___|
 | |_| | / _ \ \___ \| |_| | | |  / _ \ | |  _ 
 |  _  |/ ___ \ ___) |  _  | | | / ___ \| |_| |
 |_| |_/_/   \_\____/|_| |_| |_|/_/   \_\\____|

  Cyclone Game v3.1 — Using Arduino Nano + WS2812B
  Wokwi Simulation - "https://wokwi.com/projects/461095303071447041"
_________________________________________________________________________
  
  Name: Aniket Chowdhury [Hashtag]
  Email: micro.aniket@gmail.com
  GitHub: https://github.com/itzzhashtag
  Instagram: https://instagram.com/itzz_hashtag
  LinkedIn: https://www.linkedin.com/in/itzz-hashtag/
*/

#include <FastLED.h>        // LED strip control library
#include <TM1637Display.h>  // 4-digit display library
#include <EEPROM.h>         // Persistent memory (for high score)

// ===============================
// --- CONFIG ---
// ===============================
#define NUM_LEDS 50         // Total LEDs in strip
#define DATA_PIN 13         // Data pin for LED strip
#define BTN 12              // Main game button
#define MODE_BTN 11         // Mode selection button
#define BUZZER 3            // Buzzer pin
#define CLK 9               // TM1637 Clock pin
#define DIO 10              // TM1637 Data pin

// ===============================
// --- OBJECTS ---
// ===============================
CRGB leds[NUM_LEDS];        // LED array
TM1637Display display(CLK, DIO); // Display object

// ===============================
// --- GAME STATE ---
// ===============================
byte gameState = 0;         // 0 = idle, 1 = playing
byte Position = 0;          // Runner (red LED) position
byte level = 1;             // Game level
unsigned long gameTimer = 0; // Controls speed timing
bool newTarget = true;      // Flag to generate new target
int spot = 0;               // Target center position
int targetSize = 3;         // Safe zone size (fixed per round)
int highScore = 0;          // Stored high score (EEPROM)
int score = 0;              // Current score

// ===============================
// --- MODE SYSTEM ---
// ===============================
enum GameMode 
{
  MODE_REST,
  MODE_EAZY,
  MODE_MED,
  MODE_HARD
};
GameMode currentMode = MODE_REST;

unsigned long modeTimer = 0;// Mode display timer
bool showModeFlag = false;
unsigned long idleTimer = 0;// Idle display cycle
byte idleState = 0;

// ===============================
// --- BUTTON HANDLING ---
// ===============================
bool lastBtn = HIGH;
bool lastModeBtn = HIGH;
bool buttonPressed() {
  bool curr = digitalRead(BTN);
  if (lastBtn == HIGH && curr == LOW) 
  {
    delay(20);
    lastBtn = curr;
    return true;
  }
  lastBtn = curr;
  return false;
}

bool modePressed() {
  bool curr = digitalRead(MODE_BTN);
  if (lastModeBtn == HIGH && curr == LOW) 
  {
    delay(20);
    lastModeBtn = curr;
    return true;
  }
  lastModeBtn = curr;
  return false;
}

// ===============================
// --- SOUND ---
// ===============================
void beep(int f, int d) 
{
  tone(BUZZER, f, d);
  delay(d);
  noTone(BUZZER);
}
void sPress() 
{
  beep(1000, 40);
}
void sNear()  
{
  beep(600, 80);
}
void sHit()   
{
  beep(1200, 60);
}
void sBull()  
{
  beep(2000, 100);
}
void sDead() 
{
  int notes[] = {900, 800, 700, 600, 500, 400, 300, 200};
  for (int i = 0; i < 8; i++) 
  {
    tone(BUZZER, notes[i], 100);
    delay(120);
  }
  noTone(BUZZER);
}

// ===============================
// --- SETUP ---
// ===============================
void setup() 
{
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  display.setBrightness(5);
  EEPROM.get(0, highScore);
  if (highScore < 0 || highScore > 9999) highScore = 0;   // Safety check
  Serial.print("High Score Loaded: ");
  Serial.println(highScore);
  Serial.begin(9600);
  Serial.println("Cyclone v3.1 Ready");
}

// ===============================
// --- LOOP ---
// ===============================
void loop() 
{
  if (gameState == 0) idleMode();
  else playMode();
}

// ===============================
// --- IDLE MODE ---
// ===============================
void idleMode() 
{
  fill_rainbow(leds, NUM_LEDS, millis() / 10, 10);
  FastLED.show();
  if (modePressed()) // Mode change
  {
    currentMode = (GameMode)(((int)currentMode + 1) % 4);

    showModeFlag = true;
    modeTimer = millis();

    Serial.print("Mode: ");
    Serial.println((int)currentMode);
  }
  // Display handling
  if (showModeFlag) 
  {

    showMode();

    if (millis() - modeTimer > 10000) 
    {
      showModeFlag = false;
    }

  } 
  else 
  {

    // Cycle: 0--0 → HiSr → Score
    if (millis() - idleTimer > 3000) 
    {
      idleTimer = millis();
      idleState = (idleState + 1) % 3;
    }
    if (idleState == 0) showIdleDisplay();
    else if (idleState == 1) showHiSr();
    else display.showNumberDec(highScore);
    currentMode = MODE_REST;// 🔥 IMPORTANT: reset mode to default
  }
  if (buttonPressed())   // Start game
  {
    sPress();
    startGame();
  }
}

// ===============================
// --- START ---
// ===============================
void startGame() 
{
  score = 0;
  level = 1;
  Position = 0;
  newTarget = true;
  gameState = 1;
  Serial.println("Game Start");
}

// ===============================
// --- PLAY MODE ---
// ===============================
void playMode() 
{
  int speed;
  switch (currentMode) // Mode difficulty (ONLY speed changes)
  {
    case MODE_EAZY: speed = random(80, 110); break;
    case MODE_MED:  speed = random(50, 80); break;
    case MODE_HARD: speed = (random(0, 2) ? 25 : 40); break;
    default:        speed = constrain(80 - level * 3 + random(-6, 6), 20, 120); break;
  }
  if (millis() - gameTimer > speed) 
  {
    gameTimer = millis();
    if (newTarget)  // Generate target ONCE
    {
      spot = random(2, NUM_LEDS - 2);
      switch (currentMode)       // Lock size once (NO flicker)
      {
        case MODE_EAZY: targetSize = random(3, 5); break;
        case MODE_MED:  targetSize = random(2, 4); break;
        case MODE_HARD: targetSize = random(0, 2); // 0 or 1 break;
        default:
          int baseSize = constrain(4 - level / 4, 0, 5);
          if (level > 6 && random(0, 5) == 0)     // 🎯 chance to trigger PERFECT mode (size = 0)
          {
            targetSize = 0;   // 🔥 perfect mode
            Serial.println("PERFECT MODE (DEFAULT)");
          }
          else 
          {
            targetSize = max(1, baseSize); // normal safe zone
          }
          break;
      }
      newTarget = false;
      Serial.print("Target: ");
      Serial.print(spot);
      Serial.print(" Size: ");
      Serial.println(targetSize);
    }
    drawTarget(targetSize);
    moveRunner();
    FastLED.show();
  }
  if (buttonPressed()) 
  {
    handleHit(targetSize);
  }
  display.showNumberDec(score);
}

// ===============================
// --- HIT SYSTEM ---
// ===============================
void handleHit(int size) 
{
  sPress();  // button click sound
  int hit = Position - 1;// Get last runner position (actual hit point)
  if (hit < 0) hit = NUM_LEDS - 1;
  int dist = abs(hit - spot); // Distance calculation (wrap-safe)
  dist = min(dist, NUM_LEDS - dist);
  Serial.print("Hit: ");
  Serial.print(hit);
  Serial.print(" | Target: ");
  Serial.print(spot);
  Serial.print(" | Dist: ");
  Serial.println(dist);

  // ===============================
  // 🎯 PERFECT MODE (no safe zone)
  // ===============================
  if (size == 0) 
  {
    if (dist == 0) 
    {
      Serial.println("PERFECT HIT!");
      sBull();
      score += 30;   // higher reward
      win();
    } else 
    {
      Serial.println("MISS (Perfect Mode)");
      lose();
    }
    return;
  }

  // ===============================
  // 🎯 NORMAL MODE (multi-zone)
  // ===============================
  int inner = max(1, size / 2);
  if (dist == 0) 
  {
    Serial.println("BULLSEYE");
    sBull();
    score += 20;
    win();
  }
  else if (dist <= inner) 
  {
    Serial.println("GOOD HIT");
    sHit();
    score += 12;
    win();
  }
  else if (dist <= size) 
  {
    Serial.println("EDGE HIT");
    sNear();
    score += 8;
    win();
  }
  else 
  {
    Serial.println("MISS");
    lose();
  }
}

// ===============================
// --- WIN / LOSE ---
// ===============================
void win() 
{
  level++;
  Serial.println("WIN");
  flash(CRGB::Green);
  newTarget = true;
}

void lose() 
{
  Serial.println("DEAD");
  if (score > highScore)   // SAVE HIGH SCORE  
  {
    highScore = score;
    EEPROM.put(0, highScore);
    Serial.print("New High Score: ");
    Serial.println(highScore);
  }
  showDead();
  flash(CRGB::Red);
  sDead();
  delay(300);
  resetGame();
}
// ===============================
// --- DISPLAY ---
// ===============================
void showIdleDisplay() 
{
  uint8_t d[] = {0x3F, 0x40, 0x40, 0x3F}; // 0--0
  display.setSegments(d);
}

void showHiSr() 
{
  uint8_t d[] = {0x76, 0x10, 0x6D, 0x50}; // HiSr
  display.setSegments(d);
}

void showMode() 
{
  uint8_t d[4];
  switch (currentMode) 
  {
    case MODE_EAZY: d[0] = 0x79; d[1] = 0x77; d[2] = 0x5B; d[3] = 0x6E; break;
    case MODE_MED:  d[0] = 0x37; d[1] = 0x79; d[2] = 0x5E; d[3] = 0x00; break;
    case MODE_HARD: d[0] = 0x76; d[1] = 0x77; d[2] = 0x50; d[3] = 0x5E; break;
    default:        d[0] = 0x50; d[1] = 0x79; d[2] = 0x6D; d[3] = 0x78; break;
  }

  display.setSegments(d);
}

void showDead() 
{
  uint8_t d[] = {0x5E, 0x79, 0x77, 0x5E};
  display.setSegments(d);
}

// ===============================
// --- LED ---
// ===============================
void drawTarget(int size) 
{  
  fill_solid(leds, NUM_LEDS, CRGB::Black);      // Clear all LEDs first
  // ===============================
  // 🎯 PERFECT MODE (single LED)
  // ===============================
  if (size == 0) 
  {
    leds[spot] = CRGB::Blue;   // special color for danger
    return;
  }
  // ===============================
  // 🎯 NORMAL TARGET (multi-zone)
  // ===============================
  for (int i = -size; i <= size; i++) 
  {
    int idx = (spot + i + NUM_LEDS) % NUM_LEDS;
    if (i == 0) 
    {
      leds[idx] = CRGB::Green;      // center (bullseye)
    }
    else if (abs(i) <= size / 2) 
    {
      leds[idx] = CRGB::Yellow;     // inner zone
    }
    else 
    {
      leds[idx] = CRGB::Orange;     // outer zone
    }
  }
}

void moveRunner() 
{
  leds[Position] = CRGB::Red;
  Position = (Position + 1) % NUM_LEDS;
}

void flash(CRGB c) 
{
  for (int i = 0; i < 2; i++) 
  {
    fill_solid(leds, NUM_LEDS, c);
    FastLED.show();
    delay(150);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(150);
  }
}

// ===============================
// --- RESET ---
// ===============================
void resetGame() 
{
  gameState = 0;
  score = 0;
  level = 1;
  currentMode = MODE_REST;// 🔥 IMPORTANT: reset mode to default
  showModeFlag = false;     // Reset mode display state
  Serial.println("Reset → Back to REST mode");
}
// ===============================
// --- The END ---
// ===============================