#include "raylib.h"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <ctime>
#include <raymath.h>


//#define DEBUG

constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 900;
constexpr float PLAYER_SPEED = 5.0f;
constexpr float BULLET_SPEED = 7.0f;
constexpr float ENEMY_WIDTH = 60;
constexpr float ENEMY_HEIGHT = 40;
constexpr float ENEMY_BULLET_SPEED = 3.0f;
constexpr float ENEMY_FIRE_CHANCE = 0.002f;
constexpr float BOSS_SPEED = 2.0f;





// Абстрактный класс для наследования
class Entity {
public:
    virtual void Update() = 0;
    virtual void Draw() const = 0;
    virtual Rectangle GetRect() const = 0;
    virtual ~Entity() {}
};

// Пуля
class Bullet : public Entity {
protected:
    Rectangle rect;
    float speed;
    bool fromPlayer;
    bool active;

public:
    Bullet(float x, float y, float speed, bool fromPlayer)
        : speed(speed), fromPlayer(fromPlayer), active(true) {
        rect = { x, y, 5, 15 };
    }

    void Update() override {
        rect.y += (fromPlayer ? -speed : speed);
        if (rect.y < 0 || rect.y > SCREEN_HEIGHT)
            active = false;
    }

    void Draw() const override {
        DrawRectangleRec(rect, fromPlayer ? GREEN : RED);
    }

    Rectangle GetRect() const override { return rect; }

    bool IsActive() const { return active; }

    bool IsFromPlayer() const { return fromPlayer; }

    void Deactivate() { active = false; }
};

// Игрок
class Player : public Entity {
private:
    Rectangle rect;

public:
    bool godMode = false;
    Texture2D player_texture;
    Player() {
        rect = { SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT - 80, 80, 40 };
    }

    void Update() override {
        if (IsKeyDown(KEY_LEFT)) rect.x -= PLAYER_SPEED;
        if (IsKeyDown(KEY_RIGHT)) rect.x += PLAYER_SPEED;
        rect.x = Clamp(rect.x, 0, SCREEN_WIDTH - rect.width);
    }

    void Draw() const override {
        if (godMode){
            DrawRectangleRec(rect, WHITE);
        }
        else {
           DrawTexture(player_texture,rect.x,rect.y,WHITE);
        }
    }

    Rectangle GetRect() const override { return rect; }

    Bullet Shoot() const {
        return Bullet(rect.x + rect.width / 2 - 2, rect.y, BULLET_SPEED, true);
    }
};

// Враги
class Enemy : public Entity {
private:
    Rectangle rect;
    bool alive;

public:
    Texture2D enemy_texture;
    Enemy(float x, float y) : alive(true) {
        rect = { x, y, ENEMY_WIDTH, ENEMY_HEIGHT };
    }

    void Draw() const override {
        if (alive) {
            DrawTexture(enemy_texture, rect.x, rect.y, WHITE);
        }
    }

    Rectangle GetRect() const override { return rect; }

    void Update() override {}

    void MoveHorizontally(float dx) { rect.x += dx; }

    void MoveDown() { rect.y += ENEMY_HEIGHT; }

    bool IsAlive() const { return alive; }

    void Kill() { alive = false; }

    Bullet Shoot() const {
        return Bullet(rect.x + ENEMY_WIDTH / 2, rect.y + ENEMY_HEIGHT, ENEMY_BULLET_SPEED, false);
    }
};

// Босс
class Boss : public Entity {
private:
    Rectangle rect;
    int health;
    float speed;
    bool movingRight;

    float shootCooldown = 0.5f;
    float shootTimer = 0.0f;

public:
    Texture2D boss_texture;
    Boss() {
        rect = { SCREEN_WIDTH / 2 - 60, 50, 128, 128 };
        health = 20; 
        speed = BOSS_SPEED;
        movingRight = true;
        shootTimer = 0.0f;
    }

    void Update() override {
        float dt = GetFrameTime();
        if (movingRight) {
            rect.x += speed;
            if (rect.x + rect.width >= SCREEN_WIDTH) movingRight = false;
        }
        else {
            rect.x -= speed;
            if (rect.x <= 0) movingRight = true;
        }
        shootTimer -= dt;
    }

    void Draw() const override {
        DrawTexture(boss_texture, rect.x, rect.y, WHITE);
    }

    Rectangle GetRect() const override { return rect; }

    bool IsAlive() const { return health > 0; }

    void TakeDamage() { health--; }

    std::vector<Bullet> Shoot()  {
        std::vector<Bullet> bullets;
        if (shootTimer <= 0.0f) {
            bullets.push_back(Bullet(rect.x + rect.width / 2 - 2, rect.y + rect.height, ENEMY_BULLET_SPEED, false));
            bullets.push_back(Bullet(rect.x + rect.width / 2 - 10, rect.y + rect.height, ENEMY_BULLET_SPEED, false));
            bullets.push_back(Bullet(rect.x + rect.width / 2 + 10, rect.y + rect.height, ENEMY_BULLET_SPEED, false));
            shootTimer = shootCooldown;
        }
        return bullets;
    }
};

class EnemyFleet {
private:
    std::vector<Enemy> enemies;
    float speed;
    bool movingRight;

public:
    EnemyFleet() {
        Reset();
    }

    void Reset() {
        enemies.clear();
        speed = 1.5f;
        movingRight = true;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 11; j++) {
                enemies.emplace_back(100 + j * 50, 50 + i * 40);
            }
        }
    }

    void Update(std::vector<Bullet>& enemyBullets) {
        bool needToDescend = false;

        for (auto& enemy : enemies) {
            if (!enemy.IsAlive()) continue;
            Rectangle rect = enemy.GetRect();
            if ((movingRight && rect.x + rect.width >= SCREEN_WIDTH) ||
                (!movingRight && rect.x <= 0)) {
                needToDescend = true;
                movingRight = !movingRight;
                break;
            }
        }

        for (auto& enemy : enemies) {
            if (!enemy.IsAlive()) continue;
            if (needToDescend) enemy.MoveDown();
            else enemy.MoveHorizontally(movingRight ? speed : -speed);

            if ((float)rand() / RAND_MAX < ENEMY_FIRE_CHANCE) {
                enemyBullets.push_back(enemy.Shoot());
            }
        }

        int aliveCount = std::count_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.IsAlive(); });
        if (aliveCount > 0)
            speed = 1.5f + (55 - aliveCount) * 0.05f;
    }

    void Draw() {
        for (auto& enemy : enemies)
            enemy.Draw();
    }

    std::vector<Enemy>& GetEnemies() {
        return enemies;
    }

    bool CheckPlayerCollision(const Rectangle& playerRect) {
        for (auto& enemy : enemies)
            if (enemy.IsAlive() && CheckCollisionRecs(enemy.GetRect(), playerRect))
                return true;
        return false;
    }

    bool AllDead() const {
        return std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return !e.IsAlive(); });
    }
};

class Game {
private:
    Player player;
    EnemyFleet fleet;
    std::vector<Bullet> bullets;
    Boss* boss;
    bool gameOver = false;
    bool victory = false;
    int score = 0;
    int level = 1;
    float shootCooldown = 0.3f;
    float shootTimer = 0.0f;
    Music music;
    Sound shootSound;
    Sound hitSound;
    Sound winSound;
    Sound loseSound;
    Texture2D player_texture, enemy_texture,boss_texture,scene_lose;

public:

    Game() : boss(nullptr) {

    }
    

    void Run() {
        #ifdef DEBUG  
            player.godMode = true;
        #endif

        InitAudioDevice();
        SetMasterVolume(1.0f);
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Invaders OOP");
        player_texture = LoadTextureFromImage(LoadImage("player.png"));
        enemy_texture = LoadTextureFromImage(LoadImage("enemy.png"));
        boss_texture = LoadTextureFromImage(LoadImage("boss.png"));
        scene_lose = LoadTextureFromImage(LoadImage("Player_dead.png"));


        

        
        player.player_texture = player_texture;
        for (auto& enemy : fleet.GetEnemies()) {
            enemy.enemy_texture = enemy_texture;
        }
        music = LoadMusicStream("Soundtrack.wav");
        shootSound = LoadSound("LaserShot.wav");
        hitSound   = LoadSound("Explosion.wav");
       // winSound   = LoadSound("assets/sounds/win.wav");
        // loseSound  = LoadSound("assets/sounds/lose.wav");
        SetMusicVolume(music, 0.1f);
        SetSoundVolume(shootSound, 0.3f);
        SetSoundVolume(hitSound, 1.0f);
        PlayMusicStream(music);
        SetTargetFPS(60);
        srand(time(nullptr));

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            shootTimer -= dt;

            UpdateMusicStream(music);
            Update(dt);
            Draw();
        }
        UnloadMusicStream(music);
        UnloadSound(shootSound);
        UnloadSound(hitSound);
      //  UnloadSound(winSound);
     //   UnloadSound(loseSound);
        UnloadTexture(player_texture);
        UnloadTexture(enemy_texture);
        UnloadTexture(boss_texture);
        CloseAudioDevice();
        CloseWindow();
    }

    void Update(float dt) {
        if (gameOver || victory) {
            if (IsKeyPressed(KEY_R)) Restart();
            return;
        }

        player.Update();

        if (IsKeyDown(KEY_SPACE) && shootTimer <= 0.0f) {
            bullets.push_back(player.Shoot());
            PlaySound(shootSound);
            shootTimer = shootCooldown;
        }

        for (auto& bullet : bullets)
            bullet.Update();

        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](Bullet& b) { return !b.IsActive(); }), bullets.end());

        if (boss != nullptr && boss->IsAlive()) {
            boss->Update();
            auto bossBullets = boss->Shoot();
            bullets.insert(bullets.end(), bossBullets.begin(), bossBullets.end());
        }

        fleet.Update(bullets);

        for (auto& bullet : bullets) {
            if (!bullet.IsFromPlayer()) continue;

            for (auto& enemy : fleet.GetEnemies()) {
                if (enemy.IsAlive() && CheckCollisionRecs(bullet.GetRect(), enemy.GetRect())) {
                    enemy.Kill();
                    bullet.Deactivate();
                    PlaySound(hitSound);
                    score += 100;
                    break;
                }
            }
        }

        if (boss != nullptr && boss->IsAlive()) {
            for (auto& bullet : bullets) {
                if (!bullet.IsFromPlayer() && CheckCollisionRecs(bullet.GetRect(), player.GetRect())) {
                    if (!player.godMode) {
                        gameOver = true;
                        // PlaySound(loseSound);
                        StopMusicStream(music);

                    }
                    bullet.Deactivate();
                }

                if (bullet.IsFromPlayer() && CheckCollisionRecs(bullet.GetRect(), boss->GetRect())) {
                    boss->TakeDamage();
                    bullet.Deactivate();
                    PlaySound(hitSound);
                    score += 500;
                }
            }
        }

        for (auto& bullet : bullets) {
            if (!bullet.IsFromPlayer() && CheckCollisionRecs(bullet.GetRect(), player.GetRect())) {
                if (!player.godMode) {
                    gameOver = true;
                    // PlaySound(loseSound);
                    StopMusicStream(music);
                }
                bullet.Deactivate();
            }
        }

        if (fleet.CheckPlayerCollision(player.GetRect())) {
            if (!player.godMode) {
                gameOver = true;
                // PlaySound(loseSound);
                StopMusicStream(music);
            }
        }

        if (fleet.AllDead() && level == 1) {
            level++;
            fleet.Reset();
        } else if (fleet.AllDead() && level == 2) {
            level++;
            delete boss;
            boss = new Boss();
            boss->boss_texture = boss_texture;
        }

        if (boss != nullptr && !boss->IsAlive()) {
            victory = true;
         //   PlaySound(winSound);
            StopMusicStream(music);
        }
    }

    void Draw() {
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameOver) {
            DrawTexture(scene_lose, 0, 0, WHITE);
            DrawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 240, 40, RED);
            DrawText(TextFormat("Score: %d", score), SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 180, 20, GRAY);
            DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 120, 20, LIGHTGRAY);
        } else if (victory) {
            DrawText("VICTORY!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 40, 40, GREEN);
            DrawText(TextFormat("Score: %d", score), SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 + 10, 20, GRAY);
            DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 40, 20, LIGHTGRAY);
        } else {
            player.Draw();
            fleet.Draw();
            if (boss != nullptr) boss->Draw();

            for (const auto& bullet : bullets)
                bullet.Draw();

            DrawText(TextFormat("Score: %d", score), 10, 10, 20, LIGHTGRAY);
            DrawText(TextFormat("Level: %d", level), 10, 40, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    void Restart() {
        gameOver = false;
        victory = false;
        score = 0;
        level = 1;
        fleet.Reset();
        if (boss != nullptr) {
            delete boss;
            boss = nullptr;
        }
        PlayMusicStream(music);
    }
};

int main() {


    Game game;
    game.Run();

    return 0;
}
