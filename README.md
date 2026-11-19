# 3D AI-Based Emergency Evacuation Simulator

An interactive, visually impressive 3D Emergency Evacuation Simulator built natively with **C++**, **OpenGL**, and **FreeGLUT / GLUT**. The application simulates real-time emergency egress scenarios inside a complex multi-room building, where multiple civilians autonomously calculate safe evacuation routes to the nearest accessible emergency exits using **A\* Pathfinding**, while dynamically responding to spreading fire hazards, obstacles, and crowd collisions.

---

## 🏛️ System Architecture

```
EmergencyEvacuationSimulator/
├── main.cpp                        # GLUT window setup, OpenGL state initialization, main event loop
├── Building.h / Building.cpp       # 3D architectural floor plan, walls, corridors, rooms, and 4 exits
├── Person.h / Person.cpp           # Articulated 3D human character, walking animation, crowd dynamics
├── Fire.h / Fire.cpp               # Multi-layered fire hazard simulation, particle system, dynamic lighting
├── Pathfinding.h / Pathfinding.cpp # Discrete navigation grid, 8-directional A* search, route smoothing
├── Camera.h / Camera.cpp           # 3D orbital perspective camera and tactical top-down plan view
├── Simulation.h / Simulation.cpp   # Core state coordinator, real-time rerouting, 2D dashboard HUD
├── EmergencyEvacuationSimulator.cbp# Ready-to-build Code::Blocks project file
├── build.bat                       # One-click Windows batch compilation script (MinGW g++)
└── README.md                       # Comprehensive documentation and setup guide
```

---

## ✨ Core Modules & Technical Implementation

### Module 1: 3D Building Environment
- **Floor Plan**: Symmetrical $40\text{ m} \times 40\text{ m}$ architectural facility with 4 distinct functional zones:
  - **Office A** (North-West): Conference tables and executive cubicles.
  - **Server Lab B** (North-East): High-density computer server racks with illuminated status LEDs.
  - **Cafeteria C** (South-West): Dining tables and food service counter.
  - **Research Lab D** (South-East): Chemical workbenches and equipment dividers.
- **Corridor Network**: Cross-axial central corridors (4-meter width) connecting all rooms to 4 exterior emergency exits.
- **Emergency Exits**: North Exit, South Exit, East Exit, and West Exit equipped with green glowing ground pads, illuminated overhead "EXIT" signage boxes, and pulsing guidance strobe indicators.
- **Lighting & Materials**: Fixed-function OpenGL lighting model (`GL_LIGHT0` directional overhead light + `GL_LIGHT1` dynamic hazard point light), with specific diffuse, ambient, and specular material properties assigned per architectural element.

### Module 2: Human Character Simulation
- **Articulated 3D Human Model**: Constructed using OpenGL primitives featuring head, hair, torso, swinging upper/lower arms, hands, and swinging legs with shoes.
- **Sinusoidal Walk Cycle**: Procedurally animated limb swing angles ($\theta_{leg} = \sin(\text{phase}) \times 26^\circ$, $\theta_{arm} = -\sin(\text{phase}) \times 22^\circ$) synchronized to movement velocity.
- **Smooth Orientation**: Smooth angular interpolation toward the velocity vector ($\phi = \text{atan2}(v_x, v_z)$).
- **Civilian Palette**: Multiple uniform shades of blue (Royal Blue, Cyan, Cobalt, Teal, Navy, Sky Blue) representing civilians and staff.

### Module 3: Fire and Hazard Simulation
- **Multi-Layered Particle System**: Each fire hazard maintains 90 individual particles that rise with randomized turbulence, progressing through a realistic thermodynamic color gradient:
  - Base: Incandescent white/yellow ($\text{life} < 0.35$).
  - Body: Fiery orange to crimson red ($0.35 \le \text{life} < 0.75$).
  - Plume: Translucent rising smoke grey ($\text{life} \ge 0.75$).
- **Geometric Flame Tongues**: Intersecting vertical quads oscillating with high-frequency trigonometric harmonics under additive blending (`glBlendFunc(GL_SRC_ALPHA, GL_ONE)`).
- **Floor Danger Perimeter**: Pulsating red translucent disc and safety ring indicating the lethal hazard radius ($r = 2.2\text{ m}$).
- **Dynamic Fire Lighting**: Real-time point light (`GL_LIGHT1`) casting warm flickering orange-red light onto surrounding walls, corridors, and evacuees.

### Module 4: AI-Based A* Pathfinding
- **Navigation Grid**: $84 \times 84$ discrete spatial grid ($0.5\text{ m}$ per cell resolution) mapping static walls and dynamic fire hazards.
- **8-Directional Graph Search**:
  - Cardinal moves: cost $1.0$.
  - Diagonal moves: cost $\sqrt{2} \approx 1.4142$.
  - **Corner-Cutting Prevention**: Diagonal transitions require both orthogonal adjacent cells to be unobstructed, ensuring characters never clip through wall corners.
- **Multi-Exit Evaluation**: Evaluates all unblocked emergency exits and selects the path with the minimum geometric travel distance.
- **Dynamic Real-Time Rerouting**: When a new fire hazard is ignited (via `H` key), affected grid cells become impassable; active evacuees recalculate new routes within milliseconds.
- **Trapped Detection**: If all exit paths are cut off by fire, the person transitions to `STATE_TRAPPED`, halts movement, and displays a pulsing 3D warning marker above their head.
- **Visual Route Lines**: Evacuation paths are rendered as glowing green anti-aliased trails on the floor.

### Module 5: Collision Detection & Crowd Steering
- **AABB Wall Collisions**: Continuous collision checks against all wall bounding boxes with character radius padding ($r = 0.35\text{ m}$), enabling smooth wall sliding.
- **Social Force Crowd Separation**: Nearby evacuees exert mutual repulsive forces when distance $< 0.65\text{ m}$, preventing stacking and causing crowds to naturally fan out along corridors.
- **Hazard Repulsion**: Hard collision boundary around fire perimeters preventing evacuees from walking into active flames.

### Module 6: Camera System
- **Dual Projection Modes**:
  1. **3D Orbit Perspective View**: Free orbital camera with pitch clamping ($12^\circ$ to $85^\circ$), yaw rotation, and zoom distance clamping ($12\text{ m}$ to $70\text{ m}$).
  2. **Top-Down Tactical Plan View**: Direct overhead orthographic/perspective camera looking straight down from $Y = 44\text{ m}$, providing a complete architectural view of all rooms and evacuee flows.
- **Mouse & Keyboard Controls**: Smooth orbit using Arrow keys or click-and-drag with the left mouse button, plus zoom via `+`/`-` or mouse scroll wheel.

### Module 7: Simulation Control Panel & Tactical HUD
Rendered via an orthographic 2D overlay (`gluOrtho2D`) with semi-transparent glassmorphic panels:
- **Title Bar**: Application header with real-time `STATUS: SIMULATING` or `STATUS: PAUSED` badge.
- **Telemetry Box**:
  - `Simulation Time`: Elapsed time in `MM:SS.S`.
  - `Total Evacuees`: Initial civilian headcount.
  - `Evacuated Safely`: Total count and percentage completed (in vibrant green).
  - `Currently Moving`: Evacuees in transit (in cyan).
  - `Trapped / Blocked`: Count of civilians with no viable route (flashes red when $> 0$).
  - `Active Fire Hazards`: Count of burning hazard locations.
  - `Operational Exits`: Ratio of safe to total emergency exits.
  - `Camera View`: Active camera perspective.
  - `Evacuation Routes`: Toggle state (`VISIBLE` / `HIDDEN`).
- **Dynamic Warning Banners**:
  - `[ ! ] CRITICAL ALERT`: High-priority red blinking banner displayed across the top when any citizen is trapped.
  - `[ * ] OPERATION COMPLETE`: High-visibility green banner displayed when all reachable people reach the exits.
- **Interactive Control Bar**: Permanent key binding guide at the bottom of the screen.

---

## 🎮 Keyboard & Mouse Controls

| Control | Action |
|---|---|
| <kbd>SPACE</kbd> | **Start / Pause** simulation |
| <kbd>R</kbd> | **Restart** simulation (resets positions, clears fires, resets timer) |
| <kbd>H</kbd> | **Activate Additional Hazard** (spawns new fire at next critical checkpoint) |
| <kbd>C</kbd> | **Toggle Camera Mode** (3D Orbital Perspective $\leftrightarrow$ Top-Down View) |
| <kbd>P</kbd> | **Toggle Route Lines** (Show / Hide green path lines) |
| <kbd>↑</kbd> <kbd>↓</kbd> | **Pitch Camera** (tilt up / down) |
| <kbd>←</kbd> <kbd>→</kbd> | **Yaw Camera** (rotate left / right around building) |
| <kbd>+</kbd> or <kbd>=</kbd> | **Zoom In** |
| <kbd>-</kbd> or <kbd>_</kbd> | **Zoom Out** |
| **Left Click + Drag** | **Orbit Camera** in 3D space |
| **Scroll Wheel** | **Zoom In / Out** |
| <kbd>ESC</kbd> | **Exit** application |

---

## 🛠️ Code::Blocks Setup & Configuration Instructions

### Prerequisites
- **Code::Blocks IDE** (version 20.03 with MinGW bundled is recommended).
- **FreeGLUT** or **GLUT** development package for MinGW.

### Step 1: Install FreeGLUT in MinGW (If not already installed)
If FreeGLUT is not yet configured in your Code::Blocks MinGW folder:
1. Download the **FreeGLUT MinGW package** (`freeglut-MinGW.zip`).
2. Extract the files into your MinGW installation directory (typically `C:\Program Files\CodeBlocks\MinGW`):
   - Copy `include\GL\freeglut.h`, `glut.h`, `freeglut_ext.h`, `freeglut_std.h` into `MinGW\include\GL\`
   - Copy `lib\libfreeglut.a` (or `libglut32.a`) into `MinGW\lib\`
   - Copy `bin\freeglut.dll` (or `glut32.dll`) into `C:\Windows\System32` (for 32-bit: `C:\Windows\SysWOW64`) or keep it inside the project folder next to `EmergencyEvacuationSimulator.exe`.

### Step 2: Open Project in Code::Blocks
1. Launch **Code::Blocks**.
2. Click **File** $\rightarrow$ **Open...**
3. Navigate to `EmergencyEvacuationSimulator/` and select **`EmergencyEvacuationSimulator.cbp`**.

### Step 3: Verify Compiler & Linker Settings
The `.cbp` project file already contains all necessary linker settings. To verify manually:
1. Go to **Project** $\rightarrow$ **Build options...**
2. In the left panel, select the root project name **EmergencyEvacuationSimulator**.
3. Under the **Linker settings** tab, verify the following libraries in **Link libraries** (or **Other linker options**):
   ```
   -lfreeglut
   -lopengl32
   -lglu32
   -lwinmm
   -lgdi32
   ```
   *(Note: If your system uses classic GLUT instead of FreeGLUT, replace `-lfreeglut` with `-lglut32`)*.
4. Under **Compiler settings** $\rightarrow$ **Other compiler options**, ensure `-std=c++11` is present.
5. Click **OK**.

### Step 4: Build and Run
1. Press <kbd>F9</kbd> (or click **Build $\rightarrow$ Build and run**).
2. The application will compile, link, and launch the 3D Emergency Evacuation Simulator window!

---

## 💻 Command-Line Build Instructions (MinGW / GCC)

If you prefer building directly from the command line using MinGW `g++`:

### Using the Batch Script
Simply double-click **`build.bat`** or run:
```cmd
build.bat
```

### Manual Compilation
Open PowerShell or Command Prompt in the `EmergencyEvacuationSimulator` folder and run:
```cmd
g++ -O2 -Wall -std=c++11 main.cpp Building.cpp Person.cpp Fire.cpp Pathfinding.cpp Camera.cpp Simulation.cpp -o EmergencyEvacuationSimulator.exe -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
```

*(Fallback for classic GLUT):*
```cmd
g++ -O2 -Wall -std=c++11 main.cpp Building.cpp Person.cpp Fire.cpp Pathfinding.cpp Camera.cpp Simulation.cpp -o EmergencyEvacuationSimulator.exe -lglut32 -lopengl32 -lglu32 -lwinmm -lgdi32
```

Run the compiled executable:
```cmd
EmergencyEvacuationSimulator.exe
```

---

## 🔬 Demonstration Guide & Test Scenarios

### Scenario 1: Standard Evacuation
1. Launch the simulator.
2. Press <kbd>SPACE</kbd> if paused to begin simulation.
3. Observe all 16 civilians identifying their nearest exit (North, South, East, or West) and walking along green route lines.
4. The telemetry dashboard shows the evacuated percentage increasing in real-time.

### Scenario 2: Dynamic Hazard Rerouting
1. While civilians are moving along the corridors, press <kbd>H</kbd> to activate an additional fire hazard in the central intersection.
2. Observe the pathfinding system immediately reacting:
   - Green path lines dynamically recompute around the fire hazard.
   - Civilians whose forward path was cut off turn around and divert through secondary room doors and cross-corridors.

### Scenario 3: Trapped Civilian Alert
1. Press <kbd>H</kbd> multiple times to ignite fires blocking all exit avenues from a room.
2. Civilians inside the blocked room detect that all exits are unreachable.
3. Their state transitions to `STATE_TRAPPED`:
   - Movement halts.
   - A pulsing red 3D hazard icon appears above their heads.
   - The on-screen dashboard displays a red **CRITICAL ALERT: EVACUEES TRAPPED** banner.

### Scenario 4: Tactical Top-Down Mode
1. Press <kbd>C</kbd> at any point during evacuation to switch to the top-down tactical overview.
2. Review the building-wide evacuation flow across all 4 quadrants simultaneously.
3. Press <kbd>C</kbd> again to return to the 3D orbital perspective.

---

## 📜 Compliance & Compatibility
- **Language**: Standard C++ (C++11 or higher).
- **Graphics Pipeline**: OpenGL 1.1 / 2.1 fixed-function compatibility profile for 100% universal compatibility across all Windows GPUs and virtual machines without shader compilation or driver dependencies.
- **Windowing API**: GLUT / FreeGLUT.
- **Dependencies**: Uses standard Windows system libraries (`opengl32`, `glu32`, `gdi32`, `winmm`) and FreeGLUT. No external game engines or third-party frameworks.
