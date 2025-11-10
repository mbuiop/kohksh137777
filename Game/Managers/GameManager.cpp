#include "GameManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace GalacticOdyssey {

    // پیاده‌سازی GameManager (Singleton)
    GameManager* GameManager::instance_ = nullptr;

    GameManager::GameManager()
        : currentState_(GameState::BOOT), previousState_(GameState::BOOT),
          currentMode_(GameMode::CAMPAIGN), currentLevel_(1), currentWave_(1),
          gameTime_(0.0f), stateTime_(0.0f), frameTime_(0.0f), deltaTime_(0.0f),
          levelTime_(0.0f), waveTimer_(0.0f), levelComplete_(false), bossSpawned_(false),
          currentScore_(0), multiplier_(1), comboCount_(0), comboTime_(0.0f),
          lives_(3), continues_(3), isSavedGame_(false), autoSaveEnabled_(true),
          saveFilePath_("saves/")
    {
        std::cout << "🎮 ایجاد مدیر بازی" << std::endl;
        
        // ایجاد پوشه ذخیره‌سازی
        #ifdef _WIN32
            system("mkdir saves 2>nul");
        #else
            system("mkdir -p saves 2>/dev/null");
        #endif
    }

    GameManager::~GameManager()
    {
        Cleanup();
    }

    GameManager& GameManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new GameManager();
        }
        return *instance_;
    }

    void GameManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool GameManager::Initialize()
    {
        std::cout << "🔧 در حال راه‌اندازی مدیر بازی..." << std::endl;
        
        try {
            // ایجاد موتور بازی
            gameEngine_ = std::make_unique<GameEngine>();
            if (!gameEngine_->Initialize()) {
                throw std::runtime_error("Failed to initialize GameEngine");
            }
            
            // ایجاد سیستم‌ها
            renderSystem_ = std::make_unique<RenderSystem>(gameEngine_.get());
            physicsEngine_ = std::make_unique<PhysicsEngine>();
            audioManager_ = std::make_unique<AudioManager>();
            inputHandler_ = std::make_unique<InputHandler>();
            
            // راه‌اندازی سیستم‌ها
            if (!renderSystem_->Initialize()) {
                throw std::runtime_error("Failed to initialize RenderSystem");
            }
            
            if (!physicsEngine_->Initialize()) {
                throw std::runtime_error("Failed to initialize PhysicsEngine");
            }
            
            if (!audioManager_->Initialize()) {
                throw std::runtime_error("Failed to initialize AudioManager");
            }
            
            if (!inputHandler_->Initialize()) {
                throw std::runtime_error("Failed to initialize InputHandler");
            }
            
            // ایجاد مدیران موجودیت‌ها
            playerManager_ = std::make_unique<PlayerManager>();
            enemyManager_ = std::make_unique<EnemyManager>();
            coinManager_ = std::make_unique<CoinManager>();
            
            // راه‌اندازی مدیران موجودیت‌ها
            if (!playerManager_->Initialize(renderSystem_.get(), physicsEngine_.get(), 
                                          audioManager_.get(), inputHandler_.get())) {
                throw std::runtime_error("Failed to initialize PlayerManager");
            }
            
            if (!enemyManager_->Initialize(renderSystem_.get(), physicsEngine_.get(),
                                         audioManager_.get(), playerManager_->GetPlayer())) {
                throw std::runtime_error("Failed to initialize EnemyManager");
            }
            
            if (!coinManager_->Initialize(renderSystem_.get(), physicsEngine_.get(),
                                        audioManager_.get(), playerManager_->GetPlayer())) {
                throw std::runtime_error("Failed to initialize CoinManager");
            }
            
            // بارگذاری تنظیمات
            LoadGame("settings");
            
            // اعمال تنظیمات
            ApplySettings(settings_);
            
            // ثبت رویدادها
            RegisterEvent("player_death", [this]() { OnPlayerDeath(); });
            RegisterEvent("boss_defeated", [this]() { OnBossDefeated(); });
            RegisterEvent("secret_found", [this]() { OnSecretFound(); });
            
            std::cout << "✅ مدیر بازی با موفقیت راه‌اندازی شد" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ خطا در راه‌اندازی مدیر بازی: " << e.what() << std::endl;
            Cleanup();
            return false;
        }
    }

    void GameManager::Run()
    {
        std::cout << "🚀 شروع اجرای بازی..." << std::endl;
        
        if (!gameEngine_) {
            std::cerr << "❌ موتور بازی راه‌اندازی نشده است" << std::endl;
            return;
        }
        
        // حلقه اصلی بازی
        while (currentState_ != GameState::QUITTING) {
            // محاسبه زمان فریم
            float currentTime = gameEngine_->GetGameTime();
            deltaTime_ = currentTime - frameTime_;
            frameTime_ = currentTime;
            gameTime_ += deltaTime_;
            stateTime_ += deltaTime_;
            
            // به‌روزرسانی بر اساس وضعیت
            switch (currentState_) {
                case GameState::BOOT:
                    UpdateBoot(deltaTime_);
                    break;
                case GameState::MAIN_MENU:
                    UpdateMainMenu(deltaTime_);
                    break;
                case GameState::LOADING:
                    UpdateLoading(deltaTime_);
                    break;
                case GameState::PLAYING:
                    UpdatePlaying(deltaTime_);
                    break;
                case GameState::PAUSED:
                    UpdatePaused(deltaTime_);
                    break;
                case GameState::LEVEL_COMPLETE:
                    UpdateLevelComplete(deltaTime_);
                    break;
                case GameState::GAME_OVER:
                    UpdateGameOver(deltaTime_);
                    break;
                case GameState::UPGRADE_SHOP:
                    UpdateUpgradeShop(deltaTime_);
                    break;
                default:
                    break;
            }
            
            // رندر بر اساس وضعیت
            renderSystem_->BeginFrame();
            
            switch (currentState_) {
                case GameState::BOOT:
                    RenderBoot();
                    break;
                case GameState::MAIN_MENU:
                    RenderMainMenu();
                    break;
                case GameState::LOADING:
                    RenderLoading();
                    break;
                case GameState::PLAYING:
                    RenderPlaying();
                    break;
                case GameState::PAUSED:
                    RenderPaused();
                    break;
                case GameState::LEVEL_COMPLETE:
                    RenderLevelComplete();
                    break;
                case GameState::GAME_OVER:
                    RenderGameOver();
                    break;
                case GameState::UPGRADE_SHOP:
                    RenderUpgradeShop();
                    break;
                default:
                    break;
            }
            
            renderSystem_->EndFrame();
            
            // ذخیره‌سازی خودکار
            if (autoSaveEnabled_ && currentState_ == GameState::PLAYING) {
                static float lastAutoSave = 0.0f;
                if (gameTime_ - lastAutoSave > 60.0f) { // هر 60 ثانیه
                    SaveGame("autosave");
                    lastAutoSave = gameTime_;
                }
            }
        }
        
        std::cout << "🛑 پایان اجرای بازی" << std::endl;
    }

    void GameManager::UpdatePlaying(float deltaTime)
    {
        // به‌روزرسانی مدیران موجودیت‌ها
        playerManager_->Update(deltaTime);
        enemyManager_->Update(deltaTime);
        coinManager_->Update(deltaTime);
        
        // به‌روزرسانی زمان سطح
        levelTime_ += deltaTime;
        waveTimer_ -= deltaTime;
        
        // به‌روزرسانی کامبو
        UpdateCombo(deltaTime);
        
        // بررسی کامل شدن موج
        CheckWaveCompletion();
        
        // بررسی اسپان باس
        if (!bossSpawned_ && currentWave_ >= 5) { // بعد از 5 موج
            SpawnBoss();
        }
        
        // بررسی پایان سطح
        if (levelComplete_ && enemyManager_->GetActiveEnemyCount() == 0) {
            CompleteLevel();
        }
        
        // بررسی مرگ بازیکن
        if (playerManager_->GetPlayer() && !playerManager_->GetPlayer()->IsAlive()) {
            RemoveLife();
            if (lives_ <= 0) {
                EndGame(false);
            } else {
                playerManager_->GetPlayer()->Respawn();
            }
        }
        
        // کنترل‌های ورودی
        if (inputHandler_->IsKeyJustPressed(KeyCode::ESCAPE)) {
            PauseGame();
        }
        
        if (inputHandler_->IsKeyJustPressed(KeyCode::F1)) {
            // تقلب - اضافه کردن امتیاز
            AddScore(1000);
        }
    }

    void GameManager::RenderPlaying()
    {
        // رندر پس‌زمینه
        renderSystem_->SetClearColor(glm::vec3(0.1f, 0.1f, 0.2f));
        
        // رندر موجودیت‌ها
        playerManager_->Render();
        enemyManager_->Render();
        coinManager_->Render();
        
        // رندر رابط کاربری
        RenderHUD();
    }

    void GameManager::RenderHUD()
    {
        // نمایش امتیاز
        std::string scoreText = "امتیاز: " + std::to_string(currentScore_);
        renderSystem_->RenderText(scoreText, 20, 20, 24, glm::vec3(1.0f, 1.0f, 1.0f));
        
        // نمایش کامبو
        if (comboCount_ > 1) {
            std::string comboText = "کامبو: " + std::to_string(comboCount_) + "x";
            renderSystem_->RenderText(comboText, 20, 50, 20, glm::vec3(1.0f, 0.8f, 0.2f));
        }
        
        // نمایش سلامت بازیکن
        if (playerManager_->GetPlayer()) {
            auto& playerStats = playerManager_->GetPlayer()->GetStats();
            std::string healthText = "سلامت: " + std::to_string(playerStats.health) + "/" + std::to_string(playerStats.maxHealth);
            renderSystem_->RenderText(healthText, 20, 80, 20, glm::vec3(0.2f, 1.0f, 0.2f));
        }
        
        // نمایش موج و سطح
        std::string levelText = "مرحله: " + std::to_string(currentLevel_) + " - موج: " + std::to_string(currentWave_);
        renderSystem_->RenderText(levelText, 20, 110, 20, glm::vec3(0.8f, 0.8f, 1.0f));
        
        // نمایش زمان
        int minutes = static_cast<int>(levelTime_) / 60;
        int seconds = static_cast<int>(levelTime_) % 60;
        std::string timeText = "زمان: " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
        renderSystem_->RenderText(timeText, 20, 140, 20, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    void GameManager::SetState(GameState newState)
    {
        if (currentState_ == newState) return;
        
        std::cout << "🔄 تغییر وضعیت بازی از " << static_cast<int>(currentState_) 
                  << " به " << static_cast<int>(newState) << std::endl;
        
        OnStateExit(currentState_);
        previousState_ = currentState_;
        currentState_ = newState;
        stateTime_ = 0.0f;
        OnStateEnter(newState);
        
        // اطلاع‌رسانی به callback ها
        for (auto& callback : stateChangeCallbacks_) {
            callback(previousState_, newState);
        }
    }

    void GameManager::OnStateEnter(GameState state)
    {
        switch (state) {
            case GameState::MAIN_MENU:
                audioManager_->PlayMusic("main_theme", true);
                break;
                
            case GameState::PLAYING:
                audioManager_->PlayMusic("gameplay_theme", true);
                if (playerManager_->GetPlayer()) {
                    playerManager_->GetPlayer()->SetState(PlayerState::IDLE);
                }
                break;
                
            case GameState::PAUSED:
                audioManager_->SetPaused(true);
                break;
                
            case GameState::LEVEL_COMPLETE:
                audioManager_->PlaySound2D("level_complete", 1.0f);
                stats_.gamesCompleted++;
                break;
                
            case GameState::GAME_OVER:
                audioManager_->PlaySound2D("game_over", 1.0f);
                stats_.deaths++;
                break;
                
            default:
                break;
        }
    }

    void GameManager::OnStateExit(GameState state)
    {
        switch (state) {
            case GameState::PAUSED:
                audioManager_->SetPaused(false);
                break;
                
            default:
                break;
        }
    }

    void GameManager::StartGame(GameMode mode)
    {
        currentMode_ = mode;
        currentLevel_ = 1;
        currentWave_ = 1;
        currentScore_ = 0;
        lives_ = 3;
        continues_ = 3;
        levelComplete_ = false;
        bossSpawned_ = false;
        levelTime_ = 0.0f;
        
        stats_.gamesPlayed++;
        
        LoadLevel(currentLevel_);
        SetState(GameState::PLAYING);
        
        std::cout << "🎮 شروع بازی جدید - حالت: " << static_cast<int>(mode) 
                  << " سطح: " << currentLevel_ << std::endl;
    }

    void GameManager::LoadLevel(int level)
    {
        std::cout << "📦 در حال بارگذاری سطح " << level << "..." << std::endl;
        
        SetState(GameState::LOADING);
        
        // پاکسازی سطح قبلی
        CleanupLevel();
        
        // تنظیم سطح جدید
        currentLevel_ = level;
        currentWave_ = 1;
        levelComplete_ = false;
        bossSpawned_ = false;
        levelTime_ = 0.0f;
        waveTimer_ = 0.0f;
        
        // راه‌اندازی سطح
        SetupLevel(level);
        
        // بارگذاری منابع سطح
        // (در پیاده‌سازی کامل اینجا منابع بارگذاری می‌شوند)
        
        SetState(GameState::PLAYING);
        
        std::cout << "✅ سطح " << level << " بارگذاری شد" << std::endl;
    }

    void GameManager::SetupLevel(int level)
    {
        // تنظیمات سطح بر اساس شماره سطح
        switch (level) {
            case 1:
                // سطح 1 - مقدماتی
                enemyManager_->GetEnemySpawner()->AddWave(/* ... */);
                break;
                
            case 2:
                // سطح 2 - متوسط
                enemyManager_->GetEnemySpawner()->AddWave(/* ... */);
                break;
                
            case 3:
                // سطح 3 - پیشرفته
                enemyManager_->GetEnemySpawner()->AddWave(/* ... */);
                break;
                
            default:
                // سطوح تصادفی برای حالت بقا
                enemyManager_->GetEnemySpawner()->AddWave(/* ... */);
                break;
        }
        
        // شروع اولین موج
        SpawnWave(currentWave_);
        
        // تنظیم موقعیت بازیکن
        if (playerManager_->GetPlayer()) {
            playerManager_->GetPlayer()->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            playerManager_->GetPlayer()->Reset();
        }
    }

    void GameManager::SpawnWave(int wave)
    {
        std::cout << "🌊 اسپان موج " << wave << std::endl;
        
        currentWave_ = wave;
        waveTimer_ = 30.0f; // 30 ثانیه برای کامل کردن موج
        
        // اسپان دشمنان بر اساس موج
        int enemyCount = 5 + wave * 2;
        float spawnRadius = 15.0f + wave * 2.0f;
        
        for (int i = 0; i < enemyCount; i++) {
            float angle = (i / static_cast<float>(enemyCount)) * 2 * 3.14159f;
            glm::vec3 spawnPos = glm::vec3(
                cos(angle) * spawnRadius,
                sin(angle) * spawnRadius,
                0.0f
            );
            
            EnemyType type = EnemyType::VOLCANO;
            if (wave >= 3) type = EnemyType::UFO;
            if (wave >= 5) type = EnemyType::ASTEROID;
            
            enemyManager_->GetEnemySpawner()->SpawnEnemy(type, spawnPos);
        }
    }

    void GameManager::CheckWaveCompletion()
    {
        if (waveTimer_ <= 0.0f || enemyManager_->GetActiveEnemyCount() == 0) {
            if (currentWave_ < 10) { // حداکثر 10 موج در هر سطح
                SpawnWave(currentWave_ + 1);
            } else {
                levelComplete_ = true;
            }
        }
    }

    void GameManager::SpawnBoss()
    {
        if (bossSpawned_) return;
        
        std::cout << "👹 اسپان باس در سطح " << currentLevel_ << std::endl;
        
        bossSpawned_ = true;
        enemyManager_->SpawnBoss(glm::vec3(0.0f, 0.0f, -20.0f));
        
        // پخش موسیقی باس
        audioManager_->PlayMusic("boss_theme", true);
    }

    void GameManager::CompleteLevel()
    {
        std::cout << "🎉 سطح " << currentLevel_ << " کامل شد!" << std::endl;
        
        // محاسبه امتیاز پایانی
        int timeBonus = static_cast<int>(300.0f - levelTime_); // امتیاز زمان
        int healthBonus = 0;
        
        if (playerManager_->GetPlayer()) {
            auto& stats = playerManager_->GetPlayer()->GetStats();
            healthBonus = stats.health * 10; // امتیاز سلامت باقی‌مانده
        }
        
        int levelBonus = currentLevel_ * 1000; // امتیاز سطح
        int totalBonus = timeBonus + healthBonus + levelBonus;
        
        AddScore(totalBonus);
        
        // به‌روزرسانی پیشرفت
        progress_.maxLevelReached = std::max(progress_.maxLevelReached, currentLevel_);
        if (currentLevel_ < progress_.levelsCompleted.size()) {
            progress_.levelsCompleted[currentLevel_] = true;
        }
        
        SetState(GameState::LEVEL_COMPLETE);
    }

    void GameManager::AddScore(int points)
    {
        int actualPoints = points * multiplier_;
        currentScore_ += actualPoints;
        stats_.totalScore += actualPoints;
        stats_.highScore = std::max(stats_.highScore, currentScore_);
        
        // ایجاد افکت امتیاز
        if (actualPoints >= 100) {
            // افکت امتیاز بزرگ
        }
    }

    void GameManager::AddCombo()
    {
        comboCount_++;
        comboTime_ = 3.0f; // 3 ثانیه برای حفظ کامبو
        CalculateMultiplier();
        
        // ایجاد افکت کامبو
        if (comboCount_ % 10 == 0) {
            // افکت کامبو ویژه
        }
    }

    void GameManager::UpdateCombo(float deltaTime)
    {
        if (comboCount_ > 0) {
            comboTime_ -= deltaTime;
            if (comboTime_ <= 0.0f) {
                ResetCombo();
            }
        }
    }

    void GameManager::CalculateMultiplier()
    {
        multiplier_ = 1 + (comboCount_ / 10);
        multiplier_ = std::min(multiplier_, 10); // حداکثر 10x
    }

    void GameManager::OnPlayerDeath()
    {
        std::cout << "💀 بازیکن مرد" << std::endl;
        ResetCombo();
        
        // ایجاد افکت مرگ
        if (playerManager_->GetPlayer()) {
            // playerManager_->GetPlayer()->CreateDeathEffect();
        }
    }

    void GameManager::OnBossDefeated()
    {
        std::cout << "🎊 باس شکست خورد!" << std::endl;
        AddScore(5000); // امتیاز اضافی برای شکست باس
        levelComplete_ = true;
        
        // پخش موسیقی پیروزی
        audioManager_->PlaySound2D("boss_defeated", 1.0f);
    }

    bool GameManager::SaveGame(const std::string& slot)
    {
        std::string filePath = GetSaveFilePath(slot);
        
        try {
            std::ofstream file(filePath, std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "❌ خطا در باز کردن فایل ذخیره: " << filePath << std::endl;
                return false;
            }
            
            // ایجاد بک‌آپ
            BackupSave(slot);
            
            // ذخیره‌سازی داده‌ها
            // (در پیاده‌سازی کامل اینجا داده‌ها سریالایز می‌شوند)
            
            file.close();
            std::cout << "💾 بازی ذخیره شد: " << filePath << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ خطا در ذخیره‌سازی بازی: " << e.what() << std::endl;
            return false;
        }
    }

    // پیاده‌سازی AchievementSystem
    AchievementSystem::AchievementSystem(GameManager* manager)
        : gameManager_(manager)
    {
        std::cout << "🏆 ایجاد سیستم دستاوردها" << std::endl;
    }

    void AchievementSystem::Initialize()
    {
        LoadAchievements();
        std::cout << "✅ سیستم دستاوردها راه‌اندازی شد" << std::endl;
    }

    void AchievementSystem::LoadAchievements()
    {
        // دستاوردهای پایه
        achievements_.emplace_back(
            "first_blood", "نخستین خون", "اولین دشمن را نابود کن",
            1.0f, [](const GameStats& stats, const GameProgress& progress) {
                return stats.enemiesDestroyed >= 1;
            }
        );
        
        achievements_.emplace_back(
            "coin_collector", "جمع‌آوری سکه", "100 سکه جمع‌آوری کن",
            100.0f, [](const GameStats& stats, const GameProgress& progress) {
                return stats.coinsCollected >= 100;
            }
        );
        
        achievements_.emplace_back(
            "survivor", "بازمانده", "10 بازی را کامل کن",
            10.0f, [](const GameStats& stats, const GameProgress& progress) {
                return stats.gamesCompleted >= 10;
            }
        );
        
        // دستاوردهای پیشرفته
        achievements_.emplace_back(
            "combo_master", "استاد کامبو", "کامبو 50 تایی انجام بده",
            50.0f, [](const GameStats& stats, const GameProgress& progress) {
                return stats.maxCombo >= 50;
            }
        );
        
        achievements_.emplace_back(
            "perfectionist", "کمال‌گرا", "یک سطح را بدون آسیب دیدن کامل کن",
            1.0f, [](const GameStats& stats, const GameProgress& progress) {
                // منطق پیچیده‌تر در پیاده‌سازی کامل
                return false;
            }
        );
        
        std::cout << "📋 " << achievements_.size() << " دستاورد بارگذاری شد" << std::endl;
    }

} // namespace GalacticOdyssey
