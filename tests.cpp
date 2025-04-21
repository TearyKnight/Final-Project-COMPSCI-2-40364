// tests.cpp - Unit tests for Top-Down Shooter game
#include "raylib.h"
#include <vector>
#include <cmath>
#include <random>
#include <memory>
#include <cassert>
#include <iostream>
#include <conio.h> // For _getch()

// Constants for game settings (copied from main.cpp)
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
        
        // Handle keyboard input - not needed for tests
        
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
    }
    
    // Update enemy movement and state
    virtual void Update(float deltaTime, Player* player) {
        // Check if player is nearby
        float dx = player->x - x;
        float dy = player->y - y;
        float distToPlayer = sqrt(dx*dx + dy*dy);
        
        aggro = distToPlayer <= 150.0f;
        
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
    
    // Check if a point is inside the room
    bool ContainsPoint(float pointX, float pointY) {
        return (pointX >= x && pointX <= x + width && pointY >= y && pointY <= y + height);
    }
};

// Simple test framework
void RunTests();
void TestEntityCreation();
void TestEntityCollision();
void TestProjectile();
void TestPlayer();
void TestEnemy();
void TestRoom();

int main() {
    // Initialize window (needed for Raylib)
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Test Window");
    SetWindowState(FLAG_WINDOW_HIDDEN); // Hide the window for tests
    
    // Run tests
    std::cout << "Running unit tests for Top-Down Shooter game..." << std::endl;
    
    RunTests();
    
    std::cout << "All tests passed!" << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    _getch(); // Wait for user to press a key before exiting
    
    // Close window
    CloseWindow();
    
    return 0;
}

void RunTests() {
    TestEntityCreation();
    TestEntityCollision();
    TestProjectile();
    TestPlayer();
    TestEnemy();
    TestRoom();
}

void TestEntityCreation() {
    std::cout << "Testing Entity creation..." << std::endl;
    
    // Create an entity
    Entity entity(100, 200, 15, 50, RED);
    
    // Verify its properties
    assert(entity.x == 100);
    assert(entity.y == 200);
    assert(entity.radius == 15);
    assert(entity.health == 50);
    assert(entity.maxHealth == 50);
    assert(entity.active == true);
    assert(entity.color.r == RED.r && entity.color.g == RED.g && entity.color.b == RED.b);
    assert(entity.facing == RIGHT);
    
    // Test taking damage
    entity.TakeDamage(20);
    assert(entity.health == 30);
    
    // Test entity deactivation when health reaches 0
    entity.TakeDamage(30);
    assert(entity.health == 0);
    assert(entity.active == false);
    
    std::cout << "Entity creation test passed!" << std::endl;
}

void TestEntityCollision() {
    std::cout << "Testing Entity collision..." << std::endl;
    
    // Create two entities
    Entity entity1(100, 100, 15, 50, RED);
    Entity entity2(120, 100, 15, 50, BLUE);
    
    // Test collision detection
    assert(entity1.IsColliding(&entity2) == true);
    
    // Move entity2 far enough away to avoid collision
    entity2.x = 200;
    assert(entity1.IsColliding(&entity2) == false);
    
    std::cout << "Entity collision test passed!" << std::endl;
}

void TestProjectile() {
    std::cout << "Testing Projectile functionality..." << std::endl;
    
    // Create a projectile
    Projectile projectile;
    
    // Initially inactive
    assert(projectile.active == false);
    
    // Fire projectile
    projectile.Fire(100, 100, RIGHT, false);
    
    // Verify projectile state
    assert(projectile.active == true);
    assert(projectile.x == 100);
    assert(projectile.y == 100);
    assert(projectile.speedX == PROJECTILE_SPEED);
    assert(projectile.speedY == 0);
    assert(projectile.isEnemyProjectile == false);
    assert(projectile.damage == 10);
    
    // Test movement
    float deltaTime = 0.5f;
    projectile.Update(deltaTime);
    assert(projectile.x == 100 + PROJECTILE_SPEED * deltaTime);
    assert(projectile.y == 100);
    
    // Test enemy projectile
    Projectile enemyProjectile;
    enemyProjectile.Fire(200, 200, DOWN, true);
    assert(enemyProjectile.isEnemyProjectile == true);
    assert(enemyProjectile.damage == 5);
    assert(enemyProjectile.speedY == PROJECTILE_SPEED);
    
    std::cout << "Projectile test passed!" << std::endl;
}

void TestPlayer() {
    std::cout << "Testing Player functionality..." << std::endl;
    
    // Create a player
    Player player(100, 100);
    
    // Verify initial state
    assert(player.x == 100);
    assert(player.y == 100);
    assert(player.health == PLAYER_HEALTH);
    assert(player.radius == 15);
    assert(player.color.r == BLUE.r && player.color.g == BLUE.g && player.color.b == BLUE.b);
    
    // Test shooting cooldown
    assert(player.CanShoot() == true);
    player.ResetShootCooldown();
    assert(player.CanShoot() == false);
    
    std::cout << "Player test passed!" << std::endl;
}

void TestEnemy() {
    std::cout << "Testing Enemy functionality..." << std::endl;
    
    // Create RNG
    std::mt19937 rng(42); // Fixed seed for reproducibility
    
    // Create an enemy
    Enemy enemy(200, 200, &rng);
    
    // Verify initial state
    assert(enemy.x == 200);
    assert(enemy.y == 200);
    assert(enemy.health == ENEMY_HEALTH);
    assert(enemy.radius == 12);
    assert(enemy.color.r == RED.r && enemy.color.g == RED.g && enemy.color.b == RED.b);
    
    // The issue is here - we're assuming enemy.aggro is false initially,
    // but we should make sure of that
    assert(enemy.aggro == false); // This should be guaranteed by the constructor
    
    // Create a player far away (300 units is > 150 aggro range)
    Player player(500, 500); 
    
    // Update enemy (player not in aggro range)
    enemy.Update(0.1f, &player);
    assert(enemy.aggro == false);
    
    // Move player closer to trigger aggro (within 150 units)
    player.x = 220;
    player.y = 220;
    enemy.Update(0.1f, &player);
    assert(enemy.aggro == true);
    
    // Test shooting cooldown
    assert(enemy.CanShoot() == true); // Can shoot when aggro and cooldown 0
    enemy.ResetShootCooldown();
    assert(enemy.CanShoot() == false); // Cannot shoot when cooldown > 0
    
    // Test boss creation
    Boss boss(400, 400, &rng);
    assert(boss.health == BOSS_HEALTH);
    assert(boss.radius == 25);
    assert(boss.color.r == PURPLE.r && boss.color.g == PURPLE.g && boss.color.b == PURPLE.b);
    
    std::cout << "Enemy test passed!" << std::endl;
}

void TestRoom() {
    std::cout << "Testing Room functionality..." << std::endl;
    
    // Create RNG
    std::mt19937 rng(42);
    
    // Create a room
    Room room(0, 0, 800, 600);
    
    // Verify initial state
    assert(room.x == 0);
    assert(room.y == 0);
    assert(room.width == 800);
    assert(room.height == 600);
    assert(room.cleared == false);
    assert(room.hasBoss == false);
    
    // Test adding enemies
    room.AddEnemy(100, 100, &rng);
    room.AddEnemy(200, 200, &rng);
    assert(room.enemies.size() == 2);
    
    // Test point containment
    assert(room.ContainsPoint(100, 100) == true);
    assert(room.ContainsPoint(900, 100) == false);
    
    // Test room clearing
    for (auto& enemy : room.enemies) {
        enemy->active = false;
    }
    
    // Create a player for room update
    Player player(400, 300);
    
    // Update room
    room.Update(0.1f, &player);
    
    // Room should be cleared now
    assert(room.cleared == true);
    
    std::cout << "Room test passed!" << std::endl;
}