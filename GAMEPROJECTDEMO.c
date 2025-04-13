#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_BULLETS 300
#define MAX_ENEMIES 50
#define MAX_PARTICLES 200
#define SHOTGUN_PELLETS 5
#define MAX_BUILDINGS 100

typedef enum {
    WEAPON_PISTOL,
    WEAPON_SHOTGUN,
    WEAPON_MACHINEGUN
} WeaponType;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
    float radius;
    Color color;
} Bullet;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
    float radius;
    int health;
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float life;
    Color color;
} Particle;

typedef struct {
    Vector2 position;
    WeaponType weapon;
    float shootCooldown;
    float fireTimer;
    int health;
    int score;
} Player;

typedef struct {
    Rectangle rect;
} Building;

Camera2D camera = { 0 };
Player player = { 0 };
Bullet bullets[MAX_BULLETS] = { 0 };
Enemy enemies[MAX_ENEMIES] = { 0 };
Particle particles[MAX_PARTICLES] = { 0 };
Building buildings[MAX_BUILDINGS];
int buildingCount = 0;

void SpawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].position = (Vector2){ GetRandomValue(-500, 500), GetRandomValue(-500, 500) };
            enemies[i].radius = 10;
            enemies[i].health = 3;
            enemies[i].active = true;
            break;
        }
    }
}

void SpawnParticles(Vector2 pos, int count, Color color) {
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (particles[i].life <= 0) {
            particles[i].position = pos;
            particles[i].velocity = (Vector2){ GetRandomValue(-30, 30) / 10.0f, GetRandomValue(-30, 30) / 10.0f };
            particles[i].life = 1.0f;
            particles[i].color = color;
            count--;
        }
    }
}

void ShootBullet(Vector2 dir, float speed, float radius, Color color) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].position = player.position;
            bullets[i].velocity = Vector2Scale(Vector2Normalize(dir), speed);
            bullets[i].radius = radius;
            bullets[i].color = color;
            bullets[i].active = true;
            break;
        }
    }
}
//Handles the speed and size of the bullet projectiles shot by the player
void HandleShooting(Vector2 mouseWorld) {
    Vector2 direction = Vector2Subtract(mouseWorld, player.position);
    switch (player.weapon) {
        case WEAPON_PISTOL:
            if (player.fireTimer >= player.shootCooldown) {
                ShootBullet(direction, 1000.0f, 4, BLUE);//change the float value to change the projectile speed
                player.fireTimer = 0;
            }
            break;
        case WEAPON_SHOTGUN:
            if (player.fireTimer >= player.shootCooldown) {
                for (int i = 0; i < SHOTGUN_PELLETS; i++) {
                    float angleOffset = ((float)GetRandomValue(-10, 10)) * DEG2RAD;
                    Vector2 dirRotated = Vector2Rotate(direction, angleOffset);
                    ShootBullet(dirRotated, 900.0f, 3, ORANGE);//change the float value to change the projectile speed
                }
                player.fireTimer = 0;
            }
            break;
        case WEAPON_MACHINEGUN:
            if (player.fireTimer >= player.shootCooldown) {
                ShootBullet(direction, 1200.0f, 3, RED);//change the float value to change the projectile speed
                player.fireTimer = 0;
            }
            break;
    }
}

int main() {
    InitWindow(0, 0, "GAMEPROJECTDEMO");
    ToggleFullscreen();
    SetTargetFPS(120);//set your fps using inside this argument

    Vector2 screenSize = { GetScreenWidth(), GetScreenHeight() };

    player.position = (Vector2){ 0, 0 };
    player.weapon = WEAPON_PISTOL;
    player.shootCooldown = 0.5f;
    player.fireTimer = 0;
    player.health = 100;
    player.score = 0;

    // Generate buildings in a grid, leaving roads
    for (int i = -1000; i <= 1000; i += 300) {
        for (int j = -1000; j <= 1000; j += 300) {
            if ((i + j) % 600 == 0) continue;
            if (buildingCount < MAX_BUILDINGS) {
                buildings[buildingCount].rect = (Rectangle){ i, j, 60, 60 };
                buildingCount++;
            }
        }
    }

    float enemySpawnTimer = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        player.fireTimer += dt;
        enemySpawnTimer += dt;

        // Weapon switching
        if (IsKeyPressed(KEY_ONE)) {
            player.weapon = WEAPON_PISTOL;
            player.shootCooldown = 0.5f;
        } else if (IsKeyPressed(KEY_TWO)) {
            player.weapon = WEAPON_SHOTGUN;
            player.shootCooldown = 1.0f;
        } else if (IsKeyPressed(KEY_THREE)) {
            player.weapon = WEAPON_MACHINEGUN;
            player.shootCooldown = 0.1f;
        }

        // Movement for the player
        Vector2 move = { 0 };
        if (IsKeyDown(KEY_W)) move.y -= 1;
        if (IsKeyDown(KEY_S)) move.y += 1;
        if (IsKeyDown(KEY_A)) move.x -= 1;
        if (IsKeyDown(KEY_D)) move.x += 1;

        if (Vector2Length(move) > 0) {
            Vector2 newPos = Vector2Add(player.position, Vector2Scale(Vector2Normalize(move), 200 * dt));
            Rectangle playerRect = { newPos.x - 12, newPos.y - 12, 24, 24 };
            bool collides = false;

            for (int i = 0; i < buildingCount; i++) {
                if (CheckCollisionRecs(playerRect, buildings[i].rect)) {
                    collides = true;
                    break;
                }
            }

            if (!collides) player.position = newPos;
        }

        camera.target = player.position;
        camera.offset = (Vector2){ screenSize.x / 2, screenSize.y / 2 };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;

        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            HandleShooting(mouseWorld);
        }

        // Update bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].position = Vector2Add(bullets[i].position, Vector2Scale(bullets[i].velocity, dt));
                if (Vector2Length(Vector2Subtract(bullets[i].position, player.position)) > 1000) {
                    bullets[i].active = false;
                } else {
                    // Bullet hits building
                    Rectangle bulletRect = { bullets[i].position.x - bullets[i].radius, bullets[i].position.y - bullets[i].radius, 
                                             bullets[i].radius * 2, bullets[i].radius * 2 };
                    for (int j = 0; j < buildingCount; j++) {
                        if (CheckCollisionRecs(bulletRect, buildings[j].rect)) {
                            bullets[i].active = false;
                            break;
                        }
                    }
                }
            }
        }

        // Spawn enemies
        if (enemySpawnTimer > 1.5f) {
            SpawnEnemy();
            enemySpawnTimer = 0;
        }

        // Update enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                Vector2 toPlayer = Vector2Subtract(player.position, enemies[i].position);
                if (Vector2Length(toPlayer) > 5) {
                    enemies[i].position = Vector2Add(enemies[i].position, Vector2Scale(Vector2Normalize(toPlayer), 50 * dt));
                }

                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (bullets[j].active &&
                        CheckCollisionCircles(bullets[j].position, bullets[j].radius, enemies[i].position, enemies[i].radius)) {
                        bullets[j].active = false;
                        enemies[i].health--;
                        SpawnParticles(enemies[i].position, 10, enemies[i].health <= 0 ? YELLOW : GRAY);
                        if (enemies[i].health <= 0) {
                            enemies[i].active = false;
                            player.score += 100;
                        }
                    }
                }

                if (CheckCollisionCircles(player.position, 12, enemies[i].position, enemies[i].radius)) {
                    player.health -= 10;
                    enemies[i].active = false;
                    SpawnParticles(player.position, 20, RED);
                }
            }
        }

        // Update particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                particles[i].position = Vector2Add(particles[i].position, Vector2Scale(particles[i].velocity, dt));
                particles[i].life -= dt;
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);

        // Draw buildings
        for (int i = 0; i < buildingCount; i++) {
            DrawRectangleRec(buildings[i].rect, DARKBLUE);
        }

        // Draw player
        DrawCircleV(player.position, 12, GREEN);

        // Draw bullets
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active)
                DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
        }

        // Draw enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active)
                DrawCircleV(enemies[i].position, enemies[i].radius, MAROON);
        }

        // Draw particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                DrawCircleV(particles[i].position, 2, Fade(particles[i].color, particles[i].life));
            }
        }

        EndMode2D();

        // User-Interface
        DrawText(TextFormat("Health: %d", player.health), 20, 20, 20, RAYWHITE);
        DrawText(TextFormat("Score: %d", player.score), 20, 50, 20, YELLOW);
        DrawText("1: Pistol  2: Shotgun  3: MG", 20, 80, 20, LIGHTGRAY);
        if (player.health <= 0)
            DrawText("GAME OVER", GetScreenWidth() / 2 - 100, GetScreenHeight() / 2, 40, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
