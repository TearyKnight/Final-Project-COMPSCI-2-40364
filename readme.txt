===============================================================================
                       TOP-DOWN SHOOTER GAME
===============================================================================

A simple C++ top-down shooter game built with raylib.

-------------------------------------------------------------------------------
PROJECT OVERVIEW
-------------------------------------------------------------------------------
This project demonstrates various C++ programming concepts:

1. POINTERS AND STRUCTURES
   - Extensive use of pointers for dynamic memory management
   - Custom structs for game entities (Player, Enemy, Projectile)

2. CLASSES AND INHERITANCE
   - Base Entity class with derived classes (Player, Enemy, Boss)
   - Virtual functions for polymorphic behavior (e.g., Draw, Update)

3. ADVANCED C++ FEATURES
   - Smart pointers (std::unique_ptr) for automatic memory management
   - Random number generation using the <random> library
   - Move semantics for efficient resource handling

4. DATA STRUCTURES
   - std::vector for dynamic collections of game objects
   - Custom structs for organization and encapsulation

-------------------------------------------------------------------------------
GAME FEATURES
-------------------------------------------------------------------------------
- Player movement and shooting in four directions (WASD to move, SPACE to shoot)
- Multiple enemy types with different behaviors
- Room-based level progression
- Boss battle in the final room
- Health system and projectile collisions
- Simple game state management (main menu, gameplay, game over)

-------------------------------------------------------------------------------
REQUIREMENTS
-------------------------------------------------------------------------------
- MinGW-w64 (for g++ compiler)
- raylib library
- WindowsOS
-------------------------------------------------------------------------------
INSTALLATION (WINDOWS)
-------------------------------------------------------------------------------
1. Install MinGW-w64:
   - Download from: https://www.mingw-w64.org/downloads/
   - During installation, select x86_64 architecture
   - Add MinGW-w64 bin directory to your system PATH
     (typically C:\mingw64\bin)

2. Install raylib:
   - Download from: https://www.raylib.com/
   - Extract to C:\raylib

-------------------------------------------------------------------------------
COMPILATION
-------------------------------------------------------------------------------
Open Command Prompt and navigate to the project directory:

1. Compile the main game:
   g++ main.cpp -o topdownshooter.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm

2. Compile the tests:
   g++ tests.cpp -o tests.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm

-------------------------------------------------------------------------------
RUNNING THE GAME
-------------------------------------------------------------------------------
1. Run the main game:
   topdownshooter.exe

2. Run the tests:
   tests.exe

-------------------------------------------------------------------------------
CONTROLS
-------------------------------------------------------------------------------
- WASD: Move player
- SPACE: Shoot
- ENTER: Start game / Return to menu

-------------------------------------------------------------------------------
ADDITIONAL NOTES
-------------------------------------------------------------------------------
- The game uses a circular entity system for collision detection
- Room progression requires defeating all enemies before moving forward
- The final room contains a boss with special movement patterns and increased health
- Smart pointers manage enemy lifetime to prevent memory leaks

===============================================================================
                             END OF README
===============================================================================