#include "raylib.h"
#include <vector>
#include <cmath>
#include <random>
#include <memory>

// Constants for game settings
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float PLAYER_SPEED = 200.0f;
const float ENEMY_SPEED = 80.0f;
const int PLAYER_HEALTH = 100;
const int ENEMY_HEALTH = 30;
const int BOSS_HEALTH = 150;
const float PLAYER_SHOOT_COOLDOWN = 0.3f;
const float ENEMY_SHOOT_COOLDOWN = 1.5f;
const float PROJECTILE_SPEED = 400.0f;

// Enum for direction
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// Base Entity struct that all game objects inherit from
struct Entity {
    float x;
    float y;
    float radius;
    int health;
    int maxHealth;
    bool active;
    Color color;
    Direction facing;
    
    // Constructor
    Entity(float startX, float startY, float r, int hp, Color c) {
        x = startX;
        y = startY;
        radius = r;
        health = hp;
        maxHealth = hp;
        active = true;
        color = c;
        facing = RIGHT;
    }
    
    // Basic drawing function
    virtual void Draw() const {
        if (active) {
            DrawCircle(x, y, radius, color);
            
            // Draw health bar if damaged
            if (health < maxHealth) {
                DrawRectangle(x - radius, y - radius - 10, 2 * radius, 5, RED);
                DrawRectangle(x - radius, y - radius - 10, 2 * radius * health / maxHealth, 5, GREEN);
            }
        }
    }
    
    // Function to take damage
    void TakeDamage(int amount) {
        health -= amount;
        if (health <= 0) {
            health = 0;
            active = false;
        }
    }
    
    // Function to check collision with another entity
    bool IsColliding(Entity* other) {
        float dx = x - other->x;
        float dy = y - other->y;
        float distance = sqrt(dx*dx + dy*dy);
        return distance < (radius + other->radius);
    }
};

// Projectile struct for bullets
struct Projectile : public Entity {
    float speedX;
    float speedY;
    bool isEnemyProjectile;
    int damage;
    
    // Constructor
    Projectile() : Entity(0, 0, 5, 1, YELLOW) {
        speedX = 0;
        speedY = 0;
        isEnemyProjectile = false;
        damage = 10;
        active = false;
    }
    
    // Function to fire a projectile
    void Fire(float startX, float startY, Direction dir, bool fromEnemy) {
        x = startX;
        y = startY;
        active = true;
        isEnemyProjectile = fromEnemy;
        
        // Set properties based on who fired it
        if (fromEnemy) {
            color = RED;
            damage = 5;
        } else {
            color = YELLOW;
            damage = 10;
        }
        
        // Set velocity based on direction
        speedX = 0;
        speedY = 0;
        
        switch (dir) {
            case UP:
                speedY = -PROJECTILE_SPEED;
                break;
            case RIGHT:
                speedX = PROJECTILE_SPEED;
                break;
            case DOWN:
                speedY = PROJECTILE_SPEED;
                break;
            case LEFT:
                speedX = -PROJECTILE_SPEED;
                break;
        }
    }
    
    // Update position of projectile
    void Update(float deltaTime) {
        if (active) {
            x += speedX * deltaTime;
            y += speedY * deltaTime;
        }
    }
    
    // Override Draw to handle const correctness
    void Draw() const {
        if (active) {
            DrawCircle(x, y, radius, color);
        }
    }
};

// Player struct
struct Player : public Entity {
    float speedX;
    float speedY;
    float shootCooldown;
    
    // Constructor
    Player(float startX, float startY) : Entity(startX, startY, 15, PLAYER_HEALTH, BLUE) {
        speedX = 0;
        speedY = 0;
        shootCooldown = 0;
    }
    
    // Update player position based on input
    void Update(float deltaTime) {
        // Reset speed
        speedX = 0;
        speedY = 0;
        
        // Handle keyboard input
        if (IsKeyDown(KEY_W)) {
            speedY = -PLAYER_SPEED;
            facing = UP;
        }
        if (IsKeyDown(KEY_S)) {
            speedY = PLAYER_SPEED;
            facing = DOWN;
        }
        if (IsKeyDown(KEY_A)) {
            speedX = -PLAYER_SPEED;
            facing = LEFT;
        }
        if (IsKeyDown(KEY_D)) {
            speedX = PLAYER_SPEED;
            facing = RIGHT;
        }
        
        // Update position
        x += speedX * deltaTime;
        y += speedY * deltaTime;
        
        // Update cooldown
        if (shootCooldown > 0) {
            shootCooldown -= deltaTime;
        }
    }
    
    // Check if player can shoot
    bool CanShoot() {
        return shootCooldown <= 0;
    }
    
    // Reset shoot cooldown
    void ResetShootCooldown() {
        shootCooldown = PLAYER_SHOOT_COOLDOWN;
    }
    
    // Draw player with direction indicator
    void Draw() const override {
        Entity::Draw();
        
        // Draw direction indicator
        float arrowX = x;
        float arrowY = y;
        
        switch(facing) {
            case UP:
                arrowY = y - radius - 10;
                break;
            case RIGHT:
                arrowX = x + radius + 10;
                break;
            case DOWN:
                arrowY = y + radius + 10;
                break;
            case LEFT:
                arrowX = x - radius - 10;
                break;
        }
        
        DrawLine(x, y, arrowX, arrowY, WHITE);
    }
};

// Enemy struct
struct Enemy : public Entity {
    float speedX;
    float speedY;
    float shootCooldown;
    bool aggro;
    std::mt19937* rng;
    float moveTimer;
    
    // Constructor
    Enemy(float startX, float startY, std::mt19937* randomGen) : Entity(startX, startY, 12, ENEMY_HEALTH, RED) {
        speedX = 0;
        speedY = 0;
        shootCooldown = 0;
        aggro = false;
        rng = randomGen;
        moveTimer = 0;
        ChangeDirection();
    }
    
    // Change movement direction randomly
    void ChangeDirection() {
        std::uniform_real_distribution<float> angleDist(0, 6.28318f); // 2*PI
        float angle = angleDist(*rng);
        
        speedX = cos(angle) * ENEMY_SPEED;
        speedY = sin(angle) * ENEMY_SPEED;
        
        // Set facing direction based on velocity
        if (fabs(speedX) > fabs(speedY)) {
            facing = speedX > 0 ? RIGHT : LEFT;
        } else {
            facing = speedY > 0 ? DOWN : UP;
        }
    }
    
    // Update enemy movement and state
    virtual void Update(float deltaTime, Player* player) {
        // Check if player is nearby
        float dx = player->x - x;
        float dy = player->y - y;
        float distToPlayer = sqrt(dx*dx + dy*dy);
        
        aggro = distToPlayer <= 150.0f;
        
        // Move randomly or update facing direction
        if (!aggro) {
            moveTimer += deltaTime;
            if (moveTimer >= 2.0f) { // Change direction every 2 seconds
                ChangeDirection();
                moveTimer = 0;
            }
        } else {
            // Face player when in aggro range
            if (fabs(dx) > fabs(dy)) {
                facing = dx > 0 ? RIGHT : LEFT;
            } else {
                facing = dy > 0 ? DOWN : UP;
            }
        }
        
        // Update position
        x += speedX * deltaTime;
        y += speedY * deltaTime;
        
        // Update cooldown
        if (shootCooldown > 0) {
            shootCooldown -= deltaTime;
        }
    }
    
    // Check if enemy can shoot
    bool CanShoot() {
        return shootCooldown <= 0 && aggro;
    }
    
    // Reset shoot cooldown
    void ResetShootCooldown() {
        shootCooldown = ENEMY_SHOOT_COOLDOWN;
    }
};

// Boss struct inherits from Enemy
struct Boss : public Enemy {
    // Constructor
    Boss(float startX, float startY, std::mt19937* randomGen) : Enemy(startX, startY, randomGen) {
        radius = 25;
        health = BOSS_HEALTH;
        maxHealth = BOSS_HEALTH;
        color = PURPLE;
    }
    
    // Override update for boss-specific behavior
    void Update(float deltaTime, Player* player) override {
        Enemy::Update(deltaTime, player);
        
        // Boss has special movement pattern
        speedX = cos(GetTime() * 0.5f) * ENEMY_SPEED * 0.5f + speedX * 0.5f;
        speedY = sin(GetTime() * 0.3f) * ENEMY_SPEED * 0.5f + speedY * 0.5f;
    }
    
    // Override draw for boss-specific visuals
    void Draw() const override {
        if (active) {
            DrawCircle(x, y, radius, color);
            
            // Boss has a larger health bar
            DrawRectangle(x - radius, y - radius - 10, 2 * radius, 8, RED);
            DrawRectangle(x - radius, y - radius - 10, 2 * radius * health / maxHealth, 8, GREEN);
            
            // Label the boss
            DrawText("BOSS", x - 20, y - radius - 25, 20, YELLOW);
        }
    }
};

// Room struct for level design
struct Room {
    float x;
    float y;
    float width;
    float height;
    std::vector<std::unique_ptr<Enemy>> enemies;
    bool cleared;
    bool hasBoss;
    
    // Constructor
    Room(float posX, float posY, float w, float h, bool boss = false) {
        x = posX;
        y = posY;
        width = w;
        height = h;
        cleared = false;
        hasBoss = boss;
    }
    
    // Copy constructor to handle unique_ptr properly
    Room(const Room& other) : x(other.x), y(other.y), width(other.width), 
                             height(other.height), cleared(other.cleared), 
                             hasBoss(other.hasBoss) {
        // We don't copy enemies, as this would require copying unique_ptrs
        // which isn't directly possible
    }
    
    // Move constructor and assignment
    Room(Room&& other) = default;
    Room& operator=(Room&& other) = default;
    
    // Add enemy to room
    void AddEnemy(float enemyX, float enemyY, std::mt19937* rng) {
        enemies.push_back(std::make_unique<Enemy>(enemyX, enemyY, rng));
    }
    
    // Add boss to room
    void AddBoss(float bossX, float bossY, std::mt19937* rng) {
        enemies.push_back(std::make_unique<Boss>(bossX, bossY, rng));
    }
    
    // Update room and contained enemies
    void Update(float deltaTime, Player* player) {
        // Update all active enemies
        for (auto& enemy : enemies) {
            if (enemy && enemy->active) {
                enemy->Update(deltaTime, player);
                
                // Keep enemies inside room
                enemy->x = std::max(x + enemy->radius, std::min(enemy->x, x + width - enemy->radius));
                enemy->y = std::max(y + enemy->radius, std::min(enemy->y, y + height - enemy->radius));
            }
        }
        
        // Check if room is cleared
        cleared = true;
        for (const auto& enemy : enemies) {
            if (enemy && enemy->active) {
                cleared = false;
                break;
            }
        }
    }
    
    // Draw room and contents
    void Draw() const {
        // Draw room border (green if cleared, red if not)
        DrawRectangleLines(x, y, width, height, cleared ? GREEN : RED);
        
        // Draw enemies
        for (const auto& enemy : enemies) {
            if (enemy) {  // Make sure the enemy pointer is valid
                enemy->Draw();
            }
        }
        
        // Show exit indicator if room is cleared
        if (cleared) {
            DrawText("NEXT ROOM -->", x + width - 150, y + height / 2, 20, GREEN);
        } else {
            // Show count of remaining enemies
            int remainingEnemies = 0;
            for (const auto& enemy : enemies) {
                if (enemy && enemy->active) {
                    remainingEnemies++;
                }
            }
            char enemyText[50];
            sprintf(enemyText, "Enemies: %d", remainingEnemies);
            DrawText(enemyText, x + width / 2 - 50, y + 20, 20, RED);
        }
    }
    
    // Check if a point is inside the room
    bool ContainsPoint(float pointX, float pointY) {
        return (pointX >= x && pointX <= x + width && pointY >= y && pointY <= y + height);
    }
};

// Game class manages the overall game state
class Game {
private:
    // Game state variables
    bool isMainMenu;
    bool isGameOver;
    Player* player;
    std::vector<Room> rooms;
    int currentRoom;
    std::vector<Projectile> projectiles;
    std::mt19937 rng;
    
public:
    // Constructor
    Game() {
        isMainMenu = true;
        isGameOver = false;
        player = new Player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        currentRoom = 0;
        
        // Initialize RNG
        std::random_device rd;
        rng = std::mt19937(rd());
        
        // Initialize projectiles
        projectiles.resize(100);
        
        // Create rooms
        ResetGame();
    }
    
    // Destructor
    ~Game() {
        delete player;
    }
    
    // Reset game to initial state
    void ResetGame() {
        // Reset player
        delete player;
        player = new Player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        
        // Clear rooms
        rooms.clear();
        currentRoom = 0;
        
        // Create 5 rooms
        for (int i = 0; i < 5; i++) {
            bool isBossRoom = (i == 4); // Last room has boss
            
            // Create room with the specified position and size
            Room room(i * 800.0f, 0, 800, 600, isBossRoom);
            
            if (isBossRoom) {
                // Boss room
                room.AddBoss(i * 800.0f + 400, 300, &rng);
            } else {
                // Regular room with random enemies
                std::uniform_int_distribution<int> enemyCountDist(3, 6);
                int enemyCount = enemyCountDist(rng);
                
                for (int j = 0; j < enemyCount; j++) {
                    std::uniform_real_distribution<float> xDist(i * 800.0f + 100, i * 800.0f + 700);
                    std::uniform_real_distribution<float> yDist(100, 500);
                    
                    room.AddEnemy(xDist(rng), yDist(rng), &rng);
                }
            }
            
            // Move the room into the rooms vector
            rooms.push_back(std::move(room));
        }
        
        // Reset projectiles
        for (auto& projectile : projectiles) {
            projectile.active = false;
        }
        
        isGameOver = false;
    }
    
    // Main update function
    void Update() {
        float deltaTime = GetFrameTime();
        
        if (isMainMenu) {
            // Main menu logic
            if (IsKeyPressed(KEY_ENTER)) {
                isMainMenu = false;
            }
        } else if (isGameOver) {
            // Game over logic
            if (IsKeyPressed(KEY_ENTER)) {
                isMainMenu = true;
                ResetGame();
            }
        } else {
            // Game playing logic
            UpdateGame(deltaTime);
        }
    }
    
    // Update game state when playing
    void UpdateGame(float deltaTime) {
        // Update player
        player->Update(deltaTime);
        
        Room& room = rooms[currentRoom];
        
        // Keep player inside current room
        player->x = std::max(room.x + player->radius, std::min(player->x, room.x + room.width - player->radius));
        player->y = std::max(room.y + player->radius, std::min(player->y, room.y + room.height - player->radius));
        
        // Handle player shooting
        if (IsKeyDown(KEY_SPACE) && player->CanShoot()) {
            FireProjectile(player->x, player->y, player->facing, false);
            player->ResetShootCooldown();
        }
        
        // Handle enemy shooting
        for (auto& enemy : room.enemies) {
            if (enemy->active && enemy->CanShoot()) {
                FireProjectile(enemy->x, enemy->y, enemy->facing, true);
                enemy->ResetShootCooldown();
            }
        }
        
        // Update projectiles
        UpdateProjectiles(deltaTime);
        
        // Update current room
        room.Update(deltaTime, player);
        
        // Check for room transition
        if (room.cleared && player->x > room.x + room.width - 50) {
            if (currentRoom < rooms.size() - 1) {
                currentRoom++;
                player->x = rooms[currentRoom].x + 50;
            }
        } else if (!room.cleared && player->x > room.x + room.width - 50) {
            // Block player from leaving if enemies still alive
            player->x = room.x + room.width - 50;
        }
        
        // Check win/lose conditions
        if (currentRoom == rooms.size() - 1 && rooms[currentRoom].cleared) {
            isGameOver = true; // Victory
        }
        
        if (player->health <= 0) {
            isGameOver = true; // Defeat
        }
    }
    
    // Fire projectile from entity
    void FireProjectile(float sourceX, float sourceY, Direction dir, bool isEnemy) {
        // Find inactive projectile
        for (auto& projectile : projectiles) {
            if (!projectile.active) {
                // Calculate spawn position
                float spawnX = sourceX;
                float spawnY = sourceY;
                
                switch (dir) {
                    case UP:
                        spawnY -= 20;
                        break;
                    case RIGHT:
                        spawnX += 20;
                        break;
                    case DOWN:
                        spawnY += 20;
                        break;
                    case LEFT:
                        spawnX -= 20;
                        break;
                }
                
                projectile.Fire(spawnX, spawnY, dir, isEnemy);
                break;
            }
        }
    }
    
    // Update all projectiles and handle collisions
    void UpdateProjectiles(float deltaTime) {
        Room& room = rooms[currentRoom];
        
        for (auto& projectile : projectiles) {
            if (projectile.active) {
                projectile.Update(deltaTime);
                
                // Check if projectile left the room
                if (!room.ContainsPoint(projectile.x, projectile.y)) {
                    projectile.active = false;
                    continue;
                }
                
                // Handle player projectiles hitting enemies
                if (!projectile.isEnemyProjectile) {
                    for (auto& enemy : room.enemies) {
                        if (enemy->active && projectile.IsColliding(enemy.get())) {
                            enemy->TakeDamage(projectile.damage);
                            projectile.active = false;
                            break;
                        }
                    }
                } 
                // Handle enemy projectiles hitting player
                else {
                    if (projectile.IsColliding(player)) {
                        player->TakeDamage(projectile.damage);
                        projectile.active = false;
                    }
                }
            }
        }
    }
    
    // Draw the game
    void Draw() {
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (isMainMenu) {
            DrawMainMenu();
        } else if (isGameOver) {
            DrawGameOver();
        } else {
            DrawGame();
        }
        
        EndDrawing();
    }
    
    // Draw main menu
    void DrawMainMenu() {
        DrawText("TOP-DOWN SHOOTER", SCREEN_WIDTH/2 - 150, 200, 30, WHITE);
        DrawText("Press ENTER to Start", SCREEN_WIDTH/2 - 120, 300, 20, WHITE);
        DrawText("WASD to move, SPACE to shoot", SCREEN_WIDTH/2 - 170, 350, 20, LIGHTGRAY);
    }
    
    // Draw game over screen
    void DrawGameOver() {
        if (player->health <= 0) {
            DrawText("GAME OVER - YOU DIED!", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 50, 30, WHITE);
        } else {
            DrawText("YOU WIN! BOSS DEFEATED!", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 50, 30, WHITE);
        }
        
        DrawText("Press ENTER to return to main menu", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 50, 20, LIGHTGRAY);
    }
    
    // Draw game state
    void DrawGame() {
        // Set up camera to follow player
        Camera2D camera = { 0 };
        camera.target = (Vector2){ player->x, player->y };
        camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        
        // Begin camera mode
        BeginMode2D(camera);
        
        // Draw room
        rooms[currentRoom].Draw();
        
        // Draw player
        player->Draw();
        
        // Draw projectiles
        for (const auto& projectile : projectiles) {
            projectile.Draw();
        }
        
        // Show message if room is not cleared and player tries to exit
        Room& currentRoomRef = rooms[currentRoom];
        if (!currentRoomRef.cleared && player->x > currentRoomRef.x + currentRoomRef.width - 50) {
            DrawText("Defeat all enemies to proceed!", player->x - 200, player->y - 50, 20, RED);
        }
        
        // End camera mode
        EndMode2D();
        
        // Draw UI (not affected by camera)
        DrawPlayerUI();
    }
    
    // Draw player UI
    void DrawPlayerUI() {
        // Draw health bar
        DrawRectangle(20, 20, 200, 30, RED);
        DrawRectangle(20, 20, 200 * player->health / player->maxHealth, 30, GREEN);
        
        char healthText[30];
        sprintf(healthText, "HEALTH: %d/%d", player->health, player->maxHealth);
        DrawText(healthText, 30, 25, 20, WHITE);
        
        // Draw room counter
        char roomText[20];
        sprintf(roomText, "ROOM: %d/%d", currentRoom + 1, (int)rooms.size());
        DrawText(roomText, SCREEN_WIDTH - 150, 20, 20, WHITE);
        
        // Show boss warning in final room
        if (currentRoom == rooms.size() - 1 && !rooms[currentRoom].cleared) {
            DrawText("WARNING: BOSS AHEAD!", SCREEN_WIDTH/2 - 150, 20, 25, RED);
        }
    }
};

// Main function
int main() {
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Top-Down Shooter");
    SetTargetFPS(60);
    
    // Create game
    Game* game = new Game();
    
    // Main game loop
    while (!WindowShouldClose()) {
        game->Update();
        game->Draw();
    }
    
    // Cleanup
    delete game;
    CloseWindow();
    
    return 0;
}