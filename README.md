# 🎯 HuntDMA — Ultimate DMA Solution for Hunt: Showdown

A high-performance DMA-based project for Hunt: Showdown, forked from the **Whitebrim** base. This version includes extended features, advanced ballistics prediction, and comprehensive ESP coverage.

---

## ✨ Features

### 👤 1. Player Visuals (ESP)
* **Detailed Labels:** Player name, distance, HP readout, and health bars.
* **Advanced Boxes:** 2D box frames with adjustable head dot/circle size and offset.
* **Tactical:** Snap lines, dead player display (with separate max distance), and in-game player list overlay.
* **Team:** Friend highlighting with separate color configurations.
* **Chams:** 6 diverse rendering modes.

### 🔫 2. Aimbot & Ballistics (Hardware Required)
* **Hardware Support:** Native support for `KMBox B / B+ / Makcu / KMBox Net` (Auto-detect or manual COM/IP).
* **Precision Control:** Configurable hotkeys, adjustable FOV circle, and smoothing sliders.
* **Targeting Logic:** Priority based on Distance, Crosshair, or both.
* **Advanced Prediction:**
    * **Movement Prediction:** Leads targets based on velocity and bullet travel time.
    * **Bullet Drop Compensation:** Gravity-based curve with adjustable gravity scale and drop power.
    * **Weapon Presets:** 50+ Hunt weapons included—auto-fills bullet speed and ballistics.

### 👹 3. Boss & POI awareness
* **Boss ESP:** Name and distance labels with configurable distance and colors.
* **Points of Interest:** * Extraction points, Resupply stations, and Clues.
    * Cash registers, Pouches, Posters, Blueprints, and Gun Oil.
    * Seasonal destructibles and Boons (with separate color coding).

### 💎 4. Loot & Supply Tracking
* **Ammo Boxes:** Per-type toggle (Compact, Medium, Long, Shotgun, Special).
* **Health & Utility:** Medkits and Supply Boxes.
* **Special:** Track rare Blood Bond item locations.

### ⚠️ 5. Trap & Hazard Detection
* **Trap ESP:** Bear Traps, Trip Mines, and Darksight Dynamite.
* **Barrels:** Gunpowder, Oil, and Bio Barrels (Separate colors for traps vs. barrels).

### 🌿 6. Trait ESP (35+ Supported)
* **High-Tier/Legendary:** Deathcheat, Necromancer, Berserker, Rampage, Relentless, Remedy, Shadow, Shadowleap, Blademancer, Corpseseer, Gunrunner.
* **Standard:** Doctor, Fanning, Serpent, Greyhound, Lightfoot, Gatorlegs, Ghoul, and many more.
* **Control:** Individual toggles and max distance settings for every trait.

### 🗺️ 7. Overlay & HUD
* **2D Radar:** Draws players on the map with adjustable scale and offsets.
* **Crosshair:** 5 types (Circle/Rect) with lowered crosshair position support.
* **Performance:** Real-time FPS and Object counters.

### ⚙️ 8. System & Security
* **Security:** 🛡️ **Stream Proof** (Windows Display Affinity prevents recording/capture).
* **Configuration:** Save/Load settings via JSON files.
* **UI/UX:** Multi-language (English, Russian), DPI-aware scaling, and developer mode.

---

## 🚀 Installation & Requirements

1. **Hardware:** A secondary PC and a DMA Card (e.g., LeetDMA, Squirrel).
2. **Controller:** KMBox (B/B+/Net) is required for Aimbot functionality.
3. **Dependencies:** Ensure all required libraries are installed.
4. **Setup:**
   * Clone the repository: `git clone https://github.com/DameDamez/HuntDMAtest`
   * Build the project in Visual Studio (Release x64).
   * Configure your `config.json` or use the in-game menu.

---

## 🤝 Credits
* Original Base: **Whitebrim**
* Forked & Maintained by: **DameDamez**

---

## ⚠️ Disclaimer
This project is for educational and research purposes only. Use at your own risk. The developers are not responsible for any game bans or legal consequences.

## Credits
* [PCILeech](https://github.com/ufrisk/pcileech)
* [MemProcFS](https://github.com/ufrisk/MemProcFS)
* [insanefury](https://www.unknowncheats.me/forum/3809820-post343.html)
* [DMALibrary](https://github.com/Metick/DMALibrary/tree/Master)
* [Original HuntDMA by InterSDM](https://github.com/IntelSDM/HuntDMA)
* All amazing folks on [UnknownCheats](https://www.unknowncheats.me/forum/other-fps-games/350352-hunt-showdown.html)
