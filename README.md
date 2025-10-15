# Duck & Dot 🐤 vs ⬤  
A game where ducks and dots battle for grid supremacy. Now with *recursive existential crises*.

# Take a look at demo Gameplay:
 [[Core2]](https://www.youtube.com/watch?v=KBp2Sd1Cru8)
 [[Gameboy Advance 🕹]](https://youtu.be/GVt2afJWg8k?si=pvv2dt76D094o3T2)
## How to Play
- **D-Pad**: Move cursor.  
- **A Button**: Place your symbol.  
- **Win by**:
  - Usual tictactoe rules (when 3x3)
  - Getting **4-in-a-row** (after expansion).  
  - Or creating a **2×2 square** of your symbol.  
- If the grid fills, it **expands from a random corner**.  
  The ducks/dots are taking over!

---

## Build Instructions (GBA)
1. Clone the repo  
2. Run `setup.sh`.  
3. Load `duck_dot.gba` into your favorite emulator — or a real GBA if you’re feeling nostalgic.

---

## Setup Instructions (M5Stack Core2)
1. Make sure you have **UIFlow** or **M5Stack Core2 MicroPython firmware** installed.  
   - You can flash it easily using **M5Burner** from [M5Stack’s official site](https://m5stack.com/pages/download).  
2. Clone this repo or copy the following files to your device:  
   ```
   main.py
   res/img/
   ```  
   Ensure the folder structure looks like this:  
   ```
   /flash
     ├── main.py
     └── res/
         └── img/
             ├── duck.png
             ├── dot.png
             └── ...
   ```
3. Connect your M5Stack Core2 via USB.  
4. Use **uPyCraft**, **Thonny**, or **VS Code + PyMakr** to upload the files.  
5. Press the **Reset** button — the ducks and dots should start brawling on your screen!

---

## Known Issues
- Ducks occasionally unionize and demand better working conditions.  
- Grid expansion may cause mild existential dread.  
- M5Stack version might cause unexpected cuteness overload.
