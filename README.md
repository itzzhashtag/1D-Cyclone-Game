<div align="center">

# 🎯 1D LED Cyclone Game 🔥

**by [Aniket Chowdhury](mailto:micro.aniket@gmail.com) (aka `#Hashtag`)**

<img src="https://img.shields.io/badge/Status-Working-brightgreen?style=for-the-badge&logo=arduino" />
<img src="https://img.shields.io/badge/Built%20with-Arduino-blue?style=for-the-badge&logo=arduino" />
<img src="https://img.shields.io/badge/Game-Arcade-red?style=for-the-badge" />
<img src="https://img.shields.io/badge/Level-Adaptive-orange?style=for-the-badge" />

</div>

---

<div align="center">
Cyclone Game is a fast-paced LED-based reflex arcade game built using Arduino, featuring adaptive difficulty, multiple gameplay modes, dynamic target zones, sound effects, and EEPROM-based high score tracking.
</div>

---

## 🎮 Project Overview

Cyclone is a **reaction timing game** where a moving LED ("runner") rotates around a strip, and the player must press a button precisely when it aligns with a target zone.

The game evolves with:

* Increasing speed
* Shrinking safe zones
* Random difficulty spikes
* A brutal **Perfect Mode (no margin)**

---

## ⚡ Core Gameplay Mechanics

* 🔴 **Runner LED** continuously loops around the strip
* 🎯 **Target Zone** appears at random positions
* ⏱️ Player must **time button press perfectly**
* 🎯 Accuracy determines score:

  * 🟢 Bullseye = Maximum points
  * 🟡 Inner zone = Good hit
  * 🟠 Outer zone = Edge hit
  * ❌ Miss = Game Over

---

## 🎚️ Game Modes

Switch modes using the **Mode Button**:

| Mode               | Description                      |
| ------------------ | -------------------------------- |
| **REST (Default)** | Adaptive + mixed gameplay        |
| **EAZY**           | Slower speed + larger safe zones |
| **MED**            | Balanced gameplay                |
| **HARD**           | Fast + minimal or no safe zone   |

---

## 😈 Perfect Mode

A special difficulty spike:

* No safe zone (`targetSize = 0`)
* Only exact alignment works
* Appears randomly in higher levels

> “One LED. One chance.”

---

## 🧠 Intelligent Difficulty System

* Dynamic speed scaling with level
* Random variation to avoid predictability
* Human-friendly progression curve
* Child-friendly early gameplay

---

## 🔊 Sound System

* Button press feedback
* Near miss tone
* Hit / Bullseye confirmation
* 🎵 Descending "death" sound (arcade-style)

---

## 🖥️ Display System (TM1637)

| Screen    | Meaning                  |
| --------- | ------------------------ |
| `0--0`    | Idle state               |
| `HiSr`    | High score label         |
| `XXXX`    | Score / High Score       |
| Mode Name | Eazy / Med / Hard / Rest |
| `DEAD`    | Game over                |

---

## 🏆 High Score System

* Stored using EEPROM (persistent memory)
* Automatically updates when beaten
* Displayed periodically in idle mode

---

## 🎨 Visual Effects

* 🌈 Rainbow idle animation
* 🎯 Color-coded target zones
* 🟢 Win flash
* 🔴 Death flash
* 🔵 Perfect mode indicator

---

## 🧠 Smart Input Handling

* Debounced button logic
* Prevents long-press multi-trigger
* Clean single-click detection
* Separate Mode & Game buttons

---

## 🧩 Hardware Used

| Component          | Description     |
| ------------------ | --------------- |
| Arduino Nano / Uno | Microcontroller |
| WS2812B LED Strip  | 50 LEDs         |
| TM1637 Display     | 4-digit display |
| Push Buttons (x2)  | Game + Mode     |
| Buzzer             | Audio feedback  |
| Power Supply       | 5V stable       |

---

## 🔌 Pin Configuration

| Component   | Pin |
| ----------- | --- |
| LED Strip   | D13 |
| Game Button | D12 |
| Mode Button | D11 |
| Buzzer      | D3  |
| TM1637 CLK  | D9  |
| TM1637 DIO  | D10 |

---

## 🛠️ Libraries Used

* `FastLED.h`
* `TM1637Display.h`
* `EEPROM.h`

---

## 🧪 Simulation

👉 https://wokwi.com/projects/461095303071447041

---

## 📝 Setup Instructions

1. Upload the `.ino` file to Arduino Nano/Uno
2. Connect components as per pin configuration
3. Power the system (5V recommended)
4. Use:

   * **Mode Button** → change difficulty
   * **Game Button** → start & play
5. Try to beat your **High Score** 🏆

---

## 📊 Gameplay Flow

```
Idle → Select Mode → Start Game
     ↓
Runner Moves → Player Press
     ↓
Hit? → Yes → Level Up
     ↓
Miss → DEAD → Save Score → Reset
```

---

## 🚀 Future Improvements

* 🎮 Combo / streak multiplier system
* 🏆 Separate high scores per mode
* ⚡ Speed burst / event-based gameplay
* 🎯 Moving target system
* 🎵 Non-blocking advanced sound engine
* 📱 Bluetooth / mobile control
* 🌐 Web-based scoreboard
* 🎨 OLED / TFT UI upgrade

---

## 👤 Author & Contact

👨 **Name:** Aniket Chowdhury (aka Hashtag)  
📧 **Email:** [micro.aniket@gmail.com](mailto:micro.aniket@gmail.com)  
💼 **LinkedIn:** [itzz-hashtag](https://www.linkedin.com/in/itzz-hashtag/)  
🐙 **GitHub:** [itzzhashtag](https://github.com/itzzhashtag)  
📸 **Instagram:** [@itzz_hashtag](https://instagram.com/itzz_hashtag)

---

## 📜 License

This project is released under a **Modified MIT License (Non-Commercial)**.

🚫 Commercial use is not allowed without permission
🤝 For collaboration, contact the author

---

## ❤️ Acknowledgements

This project is built from scratch with continuous iteration, testing, and refinement.

If you like this project:

* ⭐ Star the repo
* 🍴 Fork it
* 🔧 Build your own version

---

> *“Simple components. Arcade-level experience.”* – Hashtag
