#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Константы
constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 900;
constexpr float PLAYER_SPEED = 5.0f;
constexpr float BULLET_SPEED = 7.0f;
constexpr float ENEMY_WIDTH = 60;
constexpr float ENEMY_HEIGHT = 40;
constexpr float ENEMY_BULLET_SPEED = 3.0f;
constexpr float ENEMY_FIRE_CHANCE = 0.002f;
constexpr float BOSS_SPEED = 2.0f;

class Entity {
public:
    virtual void Update() = 0;
    virtual void Draw() const = 0;
    virtual Rectangle GetRect() const = 0;
    virtual ~Entity() {}
};

class Bullet : public Entity {
protected:
    Rectangle rect;
    float speed;
    bool fromPlayer;
    bool active;

public:
    Bullet(float x, float y, float speed, bool fromPlayer);

    void Update() override;
    void Draw() const override;
    Rectangle GetRect() const override;

    bool IsActive() const;
    bool IsFromPlayer() const;
    void Deactivate();
};

class Player : public Entity {
public:
    Texture2D player_texture;
    bool godMode;

    Player();

    void Update() override;
    void Draw() const override;
    Rectangle GetRect() const override;
    Bullet Shoot() const;
};

class Enemy : public Entity {
public:
    Texture2D enemy_texture;

    Enemy(float x, float y);

    void Update() override;
    void Draw() const override;
    Rectangle GetRect() const override;

    void MoveHorizontally(float dx);
    void MoveDown();
    bool IsAlive() const;
    void Kill();
    Bullet Shoot() const;

private:
    Rectangle rect;
    bool alive;
};

class Boss : public Entity {
public:
    Texture2D boss_texture;

    Boss();

    void Update() override;
    void Draw() const override;
    Rectangle GetRect() const override;

    bool IsAlive() const;
    void TakeDamage();
    std::vector<Bullet> Shoot();

private:
    Rectangle rect;
    int health;
    float speed;
    bool movingRight;
    float shootCooldown;
    float shootTimer;
};

class EnemyFleet {
public:
    EnemyFleet();
    void Reset();
    void Update(std::vector<Bullet>& enemyBullets);
    void Draw();
    std::vector<Enemy>& GetEnemies();
    bool CheckPlayerCollision(const Rectangle& playerRect);
    bool AllDead() const;

private:
    std::vector<Enemy> enemies;
    float speed;
    bool movingRight;
};

class Game {
public:
    Game();
    void Run();

private:
    void Update(float dt);
    void Draw();
    void Restart();

    Player player;
    EnemyFleet fleet;
    std::vector<Bullet> bullets;
    Boss* boss;
    bool gameOver;
    bool victory;
    int score;
    int level;
    float shootCooldown;
    float shootTimer;

    // Аудио и текстуры
    Music music;
    Sound shootSound, hitSound, winSound, loseSound;
    Texture2D player_texture, enemy_texture, boss_texture, scene_lose, scene_win;
};

