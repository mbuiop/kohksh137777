#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

// فایل‌های هدر پروژه
#include "Engine/Core/GameEngine.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Input/InputHandler.h"
#include "Game/Entities/Player.h"
#include "Game/Entities/Enemy.h"
#include "Game/Entities/Coin.h"
#include "Game/Managers/GameManager.h"
#include "Game/Managers/LevelManager.h"
#include "Game/UI/UIManager.h"
#include "Game/Effects/ParticleSystem.h"

using namespace std;
using namespace std::chrono;

// تعریف ثابت‌های اصلی بازی
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const string GAME_TITLE = "🚀 Galactic Odyssey - سفر کهکشانی";
const int TARGET_FPS = 144;

class GalacticOdyssey : public GameEngine {
private:
    // سیستم‌های اصلی
    unique_ptr<RenderSystem> renderer;
    unique_ptr<PhysicsEngine> physics;
    unique_ptr<AudioManager> audio;
    unique_ptr<InputHandler> input;
    
    // مدیریت بازی
    unique_ptr<GameManager> gameManager;
    unique_ptr<LevelManager> levelManager;
    unique_ptr<UIManager> uiManager;
    unique_ptr<ParticleSystem> particles;
    
    // موجودیت‌های بازی
    shared_ptr<Player> player;
    vector<shared_ptr<Enemy>> enemies;
    vector<shared_ptr<Coin>> coins;
    
    // تایمرهای بازی
    high_resolution_clock::time_point lastFrameTime;
    double deltaTime;
    double frameAccumulator;
    int frameCount;
    
    // وضعیت بازی
    bool gameRunning;
    bool isPaused;
    bool showLoadingScreen;

public:
    GalacticOdyssey() : GameEngine(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE) {
        InitializeSystems();
    }

    ~GalacticOdyssey() {
        Cleanup();
    }

private:
    void InitializeSystems() {
        cout << "🎮 در حال راه‌اندازی بازی کهکشانی..." << endl;
        
        // نمایش صفحه لودینگ سینمایی
        ShowCinematicLoading();
        
        // مقداردهی اولیه سیستم‌ها
        renderer = make_unique<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
        physics = make_unique<PhysicsEngine>();
        audio = make_unique<AudioManager>();
        input = make_unique<InputHandler>();
        
        // مدیریت بازی
        gameManager = make_unique<GameManager>();
        levelManager = make_unique<LevelManager>();
        uiManager = make_unique<UIManager>(renderer.get());
        particles = make_unique<ParticleSystem>();
        
        // ایجاد بازیکن
        player = make_shared<Player>();
        player->Initialize(renderer.get());
        
        // بارگذاری منابع
        LoadGameAssets();
        
        // تنظیم وضعیت اولیه
        gameRunning = true;
        isPaused = false;
        showLoadingScreen = true;
        lastFrameTime = high_resolution_clock::now();
        deltaTime = 0.0;
        frameAccumulator = 0.0;
        frameCount = 0;
        
        // پخش موسیقی پس‌زمینه
        audio->PlayBackgroundMusic("Assets/Audio/galactic_theme.mp3");
        
        cout << "✅ بازی با موفقیت راه‌اندازی شد!" << endl;
    }

    void ShowCinematicLoading() {
        cout << "🎬 نمایش لودینگ سینمایی..." << endl;
        
        // شبیه‌سازی لودینگ با افکت‌های بصری
        vector<string> loadingStages = {
            "📦 در حال بارگذاری موتور گرافیکی...",
            "🔊 در حال راه‌اندازی سیستم صوتی...",
            "🎮 در حال تنظیم کنترل‌ها...",
            "🌌 در حال تولید دنیای بازی...",
            "✨ در حال بارگذاری افکت‌های ویژه...",
            "🚀 آماده برای پرتاب!"
        };
        
        for (const auto& stage : loadingStages) {
            cout << stage << endl;
            this_thread::sleep_for(milliseconds(800));
        }
        
        cout << "🎉 بارگذاری کامل شد!" << endl;
    }

    void LoadGameAssets() {
        cout << "📁 در حال بارگذاری منابع بازی..." << endl;
        
        // بارگذاری مدل‌های سه بعدی
        renderer->LoadModel("player_ship", "Assets/Models/player_ship.obj");
        renderer->LoadModel("enemy_volcano", "Assets/Models/volcano.obj");
        renderer->LoadModel("coin_planet", "Assets/Models/planet_coin.obj");
        
        // بارگذاری بافت‌ها
        renderer->LoadTexture("space_bg", "Assets/Textures/space_background.jpg");
        renderer->LoadTexture("nebula", "Assets/Textures/nebula.png");
        renderer->LoadTexture("starfield", "Assets/Textures/starfield.png");
        
        // بارگذاری شیدرها
        renderer->LoadShader("celestial", "Assets/Shaders/celestial.vert", "Assets/Shaders/celestial.frag");
        renderer->LoadShader("particle", "Assets/Shaders/particle.vert", "Assets/Shaders/particle.frag");
        
        // بارگذاری صداها
        audio->LoadSound("explosion", "Assets/Audio/explosion.wav");
        audio->LoadSound("coin_collect", "Assets/Audio/coin_collect.wav");
        audio->LoadSound("engine_hum", "Assets/Audio/engine_hum.wav");
        
        cout << "✅ منابع بازی بارگذاری شدند" << endl;
    }

    void Update(double dt) {
        if (!gameRunning) return;
        
        // پردازش ورودی
        ProcessInput();
        
        if (!isPaused) {
            // به‌روزرسانی فیزیک
            physics->Update(dt);
            
            // به‌روزرسانی موجودیت‌ها
            player->Update(dt);
            UpdateEnemies(dt);
            UpdateCoins(dt);
            
            // به‌روزرسانی سیستم ذرات
            particles->Update(dt);
            
            // بررسی برخوردها
            CheckCollisions();
            
            // به‌روزرسانی مدیریت سطح
            levelManager->Update(dt);
            
            // به‌روزرسانی رابط کاربری
            uiManager->Update(dt);
        }
    }

    void Render() {
        // شروع رندر
        renderer->BeginFrame();
        
        // رندر پس‌زمینه کهکشانی
        RenderGalacticBackground();
        
        if (!isPaused) {
            // رندر موجودیت‌های بازی
            player->Render(renderer.get());
            RenderEnemies();
            RenderCoins();
            
            // رندر سیستم ذرات
            particles->Render(renderer.get());
        }
        
        // رندر رابط کاربری
        uiManager->Render();
        
        // نمایش آمار عملکرد
        RenderPerformanceStats();
        
        // پایان رندر
        renderer->EndFrame();
    }

    void ProcessInput() {
        input->Update();
        
        // کنترل بازیکن
        if (input->IsKeyPressed(SDLK_w) || input->IsKeyPressed(SDLK_UP)) {
            player->MoveForward();
        }
        if (input->IsKeyPressed(SDLK_s) || input->IsKeyPressed(SDLK_DOWN)) {
            player->MoveBackward();
        }
        if (input->IsKeyPressed(SDLK_a) || input->IsKeyPressed(SDLK_LEFT)) {
            player->RotateLeft();
        }
        if (input->IsKeyPressed(SDLK_d) || input->IsKeyPressed(SDLK_RIGHT)) {
            player->RotateRight();
        }
        
        // استفاده از بمب
        if (input->IsKeyPressed(SDLK_SPACE)) {
            player->UseBomb();
            CreateBombExplosion(player->GetPosition());
        }
        
        // مکث بازی
        if (input->IsKeyPressed(SDLK_ESCAPE)) {
            isPaused = !isPaused;
        }
        
        // کنترل‌های لمسی (برای موبایل)
        if (input->GetTouchCount() > 0) {
            auto touchPos = input->GetTouchPosition(0);
            player->MoveTo(touchPos.x, touchPos.y);
        }
    }

    void RenderGalacticBackground() {
        // رندر پس‌زمینه پویای کهکشانی
        renderer->DrawTexture("space_bg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        renderer->DrawTexture("nebula", sin(frameCount * 0.01) * 100, cos(frameCount * 0.008) * 80, 800, 600);
        renderer->DrawTexture("starfield", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // افکت‌های کهکشانی پویا
        for (int i = 0; i < 100; i++) {
            float x = sin(frameCount * 0.02 + i) * SCREEN_WIDTH * 0.3 + SCREEN_WIDTH / 2;
            float y = cos(frameCount * 0.015 + i) * SCREEN_HEIGHT * 0.3 + SCREEN_HEIGHT / 2;
            renderer->DrawParticle(x, y, 2, 255, 255, 255, 150);
        }
    }

    void UpdateEnemies(double dt) {
        for (auto& enemy : enemies) {
            enemy->Update(dt);
            
            // هوش مصنوعی دشمنان
            if (enemy->GetType() == EnemyType::VOLCANO) {
                // تعقیب بازیکن
                auto playerPos = player->GetPosition();
                enemy->MoveToward(playerPos.x, playerPos.y);
            }
        }
        
        // حذف دشمنان نابود شده
        enemies.erase(
            remove_if(enemies.begin(), enemies.end(),
                [](const shared_ptr<Enemy>& e) { return e->IsDestroyed(); }),
            enemies.end()
        );
        
        // تولید دشمنان جدید
        if (enemies.size() < levelManager->GetMaxEnemies()) {
            SpawnEnemyGroup();
        }
    }

    void UpdateCoins(double dt) {
        for (auto& coin : coins) {
            coin->Update(dt);
            
            // چرخش و حرکت سکه‌ها
            coin->Rotate(dt * 2.0);
            coin->FloatAnimation(dt);
        }
        
        // حذف سکه‌های جمع‌آوری شده
        coins.erase(
            remove_if(coins.begin(), coins.end(),
                [](const shared_ptr<Coin>& c) { return c->IsCollected(); }),
            coins.end()
        );
        
        // تولید سکه‌های جدید
        if (coins.size() < levelManager->GetCoinsNeeded()) {
            SpawnCoin();
        }
    }

    void RenderEnemies() {
        for (auto& enemy : enemies) {
            enemy->Render(renderer.get());
        }
    }

    void RenderCoins() {
        for (auto& coin : coins) {
            coin->Render(renderer.get());
        }
    }

    void SpawnEnemyGroup() {
        int groupSize = min(3 + levelManager->GetCurrentLevel() / 5, 7);
        
        for (int i = 0; i < groupSize; i++) {
            auto enemy = make_shared<Enemy>();
            enemy->Initialize(renderer.get());
            
            // موقعیت‌های مختلف برای حمله
            float x, y;
            int spawnSide = rand() % 4;
            
            switch (spawnSide) {
                case 0: // بالا
                    x = rand() % SCREEN_WIDTH;
                    y = -50;
                    break;
                case 1: // راست
                    x = SCREEN_WIDTH + 50;
                    y = rand() % SCREEN_HEIGHT;
                    break;
                case 2: // پایین
                    x = rand() % SCREEN_WIDTH;
                    y = SCREEN_HEIGHT + 50;
                    break;
                case 3: // چپ
                    x = -50;
                    y = rand() % SCREEN_HEIGHT;
                    break;
            }
            
            enemy->SetPosition(x, y);
            enemy->SetSpeed(1.0 + levelManager->GetCurrentLevel() * 0.1);
            enemies.push_back(enemy);
        }
    }

    void SpawnCoin() {
        auto coin = make_shared<Coin>();
        coin->Initialize(renderer.get());
        
        float x = 50 + rand() % (SCREEN_WIDTH - 100);
        float y = 50 + rand() % (SCREEN_HEIGHT - 100);
        
        coin->SetPosition(x, y);
        coin->SetValue(levelManager->GetCurrentLevel());
        coins.push_back(coin);
    }

    void CheckCollisions() {
        // برخورد بازیکن با سکه‌ها
        for (auto& coin : coins) {
            if (!coin->IsCollected() && 
                physics->CheckCollision(player->GetCollider(), coin->GetCollider())) {
                
                coin->Collect();
                gameManager->AddScore(coin->GetValue() * 10);
                gameManager->AddCoins(1);
                player->AddFuel(10);
                
                // افکت جمع‌آوری سکه
                CreateCoinCollectionEffect(coin->GetPosition());
                audio->PlaySound("coin_collect");
            }
        }
        
        // برخورد بازیکن با دشمنان
        for (auto& enemy : enemies) {
            if (physics->CheckCollision(player->GetCollider(), enemy->GetCollider())) {
                if (!player->IsInvulnerable()) {
                    gameManager->TakeDamage(25);
                    CreateExplosionEffect(player->GetPosition());
                    audio->PlaySound("explosion");
                    
                    if (gameManager->GetPlayerHealth() <= 0) {
                        GameOver();
                    }
                }
            }
        }
    }

    void CreateBombExplosion(const Vector2& position) {
        // ایجاد افکت انفجار بمب
        for (int i = 0; i < 50; i++) {
            float angle = (i / 50.0f) * 2 * M_PI;
            float speed = 3.0f + (rand() % 100) / 50.0f;
            particles->CreateParticle(
                position.x, position.y,
                cos(angle) * speed, sin(angle) * speed,
                255, 100, 50, 255, 2.0f
            );
        }
        
        // نابودی تمام دشمنان در شعاع انفجار
        for (auto& enemy : enemies) {
            float distance = physics->Distance(position, enemy->GetPosition());
            if (distance < 300) {
                enemy->Destroy();
                gameManager->AddScore(50);
            }
        }
        
        audio->PlaySound("explosion");
    }

    void CreateCoinCollectionEffect(const Vector2& position) {
        // افکت بصری جمع‌آوری سکه
        for (int i = 0; i < 15; i++) {
            float angle = (i / 15.0f) * 2 * M_PI;
            float speed = 1.5f + (rand() % 100) / 100.0f;
            particles->CreateParticle(
                position.x, position.y,
                cos(angle) * speed, sin(angle) * speed,
                255, 255, 100, 255, 1.5f
            );
        }
    }

    void CreateExplosionEffect(const Vector2& position) {
        // افکت انفجار
        for (int i = 0; i < 30; i++) {
            float angle = (rand() % 360) * M_PI / 180.0f;
            float speed = 2.0f + (rand() % 100) / 50.0f;
            particles->CreateParticle(
                position.x, position.y,
                cos(angle) * speed, sin(angle) * speed,
                255, 50, 50, 255, 1.0f
            );
        }
    }

    void RenderPerformanceStats() {
        if (uiManager->ShowDebugInfo()) {
            string fpsText = "FPS: " + to_string(static_cast<int>(1.0 / deltaTime));
            string frameTime = "Frame: " + to_string(deltaTime * 1000).substr(0, 5) + "ms";
            string entities = "Entities: P:" + to_string(1) + " E:" + to_string(enemies.size()) + " C:" + to_string(coins.size());
            
            renderer->DrawText(fpsText, 10, 10, 20, 255, 255, 255);
            renderer->DrawText(frameTime, 10, 35, 20, 255, 255, 255);
            renderer->DrawText(entities, 10, 60, 20, 255, 255, 255);
        }
    }

    void GameOver() {
        cout << "💀 بازی تمام شد! امتیاز نهایی: " << gameManager->GetScore() << endl;
        
        // نمایش انیمیشن گیم اوور
        CreateGameOverEffect();
        
        // ذخیره امتیاز
        gameManager->SaveHighScore();
        
        // بازگشت به منوی اصلی پس از تاخیر
        this_thread::sleep_for(seconds(3));
        gameRunning = false;
    }

    void CreateGameOverEffect() {
        // افکت سینمایی گیم اوور
        for (int i = 0; i < 100; i++) {
            float angle = (rand() % 360) * M_PI / 180.0f;
            float speed = 1.0f + (rand() % 200) / 100.0f;
            particles->CreateParticle(
                SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                cos(angle) * speed, sin(angle) * speed,
                255, 0, 0, 255, 3.0f
            );
        }
    }

    void Cleanup() {
        cout << "🧹 در حال پاکسازی منابع..." << endl;
        
        // پاکسازی موجودیت‌ها
        enemies.clear();
        coins.clear();
        player.reset();
        
        // پاکسازی سیستم‌ها
        particles.reset();
        uiManager.reset();
        levelManager.reset();
        gameManager.reset();
        
        input.reset();
        audio.reset();
        physics.reset();
        renderer.reset();
        
        cout << "✅ پاکسازی کامل شد" << endl;
    }

public:
    void Run() override {
        cout << "🚀 شروع بازی کهکشانی!" << endl;
        
        while (gameRunning) {
            // محاسبه زمان فریم
            auto currentTime = high_resolution_clock::now();
            deltaTime = duration<double>(currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;
            
            // محدود کردن نرخ فریم
            if (deltaTime < 1.0 / TARGET_FPS) {
                this_thread::sleep_for(milliseconds(1));
                continue;
            }
            
            // به‌روزرسانی و رندر
            Update(deltaTime);
            Render();
            
            // آمار فریم
            frameCount++;
            frameAccumulator += deltaTime;
            if (frameAccumulator >= 1.0) {
                cout << "📊 FPS: " << frameCount << endl;
                frameCount = 0;
                frameAccumulator = 0.0;
            }
        }
        
        cout << "👋 پایان بازی" << endl;
    }
};

// تابع اصلی
int main(int argc, char* argv[]) {
    try {
        cout << "🎮 راه‌اندازی بازی کهکشانی..." << endl;
        
        unique_ptr<GalacticOdyssey> game = make_unique<GalacticOdyssey>();
        game->Run();
        
        return 0;
    }
    catch (const exception& e) {
        cerr << "❌ خطای بحرانی: " << e.what() << endl;
        return -1;
    }
}
