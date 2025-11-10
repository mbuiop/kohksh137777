#pragma once
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Engine/Core/GameEngine.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Input/InputHandler.h"
#include "Game/Entities/Player.h"
#include "Game/Entities/Enemy.h"
#include "Game/Entities/Coin.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace GalacticOdyssey {

    // وضعیت‌های مختلف بازی
    enum class GameState {
        BOOT,           // راه‌اندازی
        MAIN_MENU,      // منوی اصلی
        LOADING,        // در حال بارگذاری
        PLAYING,        // در حال بازی
        PAUSED,         // مکث شده
        LEVEL_COMPLETE, // مرحله کامل شده
        GAME_OVER,      // بازی تمام شده
        UPGRADE_SHOP,   // فروشگاه ارتقا
        LEADERBOARD,    // جدول امتیازات
        SETTINGS,       // تنظیمات
        QUITTING        // در حال خروج
    };

    // حالت‌های بازی
    enum class GameMode {
        CAMPAIGN,       // حالت کمپین
        SURVIVAL,       // حالت بقا
        ENDLESS,        // حالت بی‌پایان
        BOSS_RUSH,      // حالت حمله باس
        TIME_ATTACK,    // حالت ضد زمان
        CHALLENGE,      // حالت چالشی
        FREE_PLAY       // حالت آزاد
    };

    // سطح دشواری
    enum class Difficulty {
        EASY,           // آسان
        NORMAL,         // معمولی
        HARD,           // سخت
        EXPERT,         // حرفه‌ای
        IMPOSSIBLE      // غیرممکن
    };

    // ساختار تنظیمات بازی
    struct GameSettings {
        Difficulty difficulty;
        bool fullscreen;
        int resolutionWidth;
        int resolutionHeight;
        float masterVolume;
        float musicVolume;
        float effectsVolume;
        bool vsyncEnabled;
        bool showFPS;
        bool motionBlur;
        bool bloomEnabled;
        int textureQuality;
        int shadowQuality;
        int antiAliasing;
        std::string language;
        
        GameSettings() 
            : difficulty(Difficulty::NORMAL), fullscreen(false),
              resolutionWidth(1920), resolutionHeight(1080),
              masterVolume(1.0f), musicVolume(0.7f), effectsVolume(0.8f),
              vsyncEnabled(true), showFPS(true), motionBlur(false),
              bloomEnabled(true), textureQuality(2), shadowQuality(2),
              antiAliasing(2), language("en") {}
    };

    // ساختار آمار بازی
    struct GameStats {
        int totalPlayTime;          // کل زمان بازی (ثانیه)
        int gamesPlayed;            // تعداد بازی‌های انجام شده
        int gamesCompleted;         // تعداد بازی‌های کامل شده
        int totalScore;             // مجموع امتیازات
        int highScore;              // بالاترین امتیاز
        int enemiesDestroyed;       // دشمنان نابود شده
        int coinsCollected;         // سکه‌های جمع‌آوری شده
        int powerUpsUsed;           // قدرت‌های استفاده شده
        int deaths;                 // تعداد مرگ‌ها
        float accuracy;             // دقت شلیک
        int maxCombo;               // بیشترین کامبو
        int secretsFound;           // اسرار پیدا شده
        
        GameStats() 
            : totalPlayTime(0), gamesPlayed(0), gamesCompleted(0),
              totalScore(0), highScore(0), enemiesDestroyed(0),
              coinsCollected(0), powerUpsUsed(0), deaths(0),
              accuracy(0.0f), maxCombo(0), secretsFound(0) {}
    };

    // ساختار پیشرفت بازی
    struct GameProgress {
        int currentLevel;
        int maxLevelReached;
        std::vector<bool> levelsCompleted;
        std::vector<bool> achievementsUnlocked;
        std::vector<std::string> unlockedShips;
        std::vector<std::string> unlockedWeapons;
        int totalStarsEarned;
        std::unordered_map<std::string, int> collectiblesFound;
        
        GameProgress() 
            : currentLevel(1), maxLevelReached(1), totalStarsEarned(0) {}
    };

    // کلاس مدیریت اصلی بازی
    class GameManager {
    private:
        static GameManager* instance_;
        
        // موتور و سیستم‌ها
        std::unique_ptr<GameEngine> gameEngine_;
        std::unique_ptr<RenderSystem> renderSystem_;
        std::unique_ptr<PhysicsEngine> physicsEngine_;
        std::unique_ptr<AudioManager> audioManager_;
        std::unique_ptr<InputHandler> inputHandler_;
        
        // موجودیت‌های بازی
        std::unique_ptr<PlayerManager> playerManager_;
        std::unique_ptr<EnemyManager> enemyManager_;
        std::unique_ptr<CoinManager> coinManager_;
        
        // وضعیت بازی
        GameState currentState_;
        GameState previousState_;
        GameMode currentMode_;
        GameSettings settings_;
        GameStats stats_;
        GameProgress progress_;
        
        // زمان‌سنج‌ها
        float gameTime_;
        float stateTime_;
        float frameTime_;
        float deltaTime_;
        
        // مدیریت سطح
        int currentLevel_;
        int currentWave_;
        float levelTime_;
        float waveTimer_;
        bool levelComplete_;
        bool bossSpawned_;
        
        // امتیاز و منابع
        int currentScore_;
        int multiplier_;
        int comboCount_;
        float comboTime_;
        int lives_;
        int continues_;
        
        // رویدادها و callback ها
        std::unordered_map<std::string, std::function<void()>> eventCallbacks_;
        std::vector<std::function<void(GameState, GameState)>> stateChangeCallbacks_;
        
        // ذخیره‌سازی و تنظیمات
        std::string saveFilePath_;
        bool isSavedGame_;
        bool autoSaveEnabled_;
        
    public:
        GameManager();
        ~GameManager();
        
        static GameManager& GetInstance();
        static void DestroyInstance();
        
        // مدیریت چرخه حیات
        bool Initialize();
        void Run();
        void Cleanup();
        void Shutdown();
        
        // مدیریت وضعیت
        void SetState(GameState newState);
        void PushState(GameState newState);
        void PopState();
        GameState GetCurrentState() const { return currentState_; }
        GameState GetPreviousState() const { return previousState_; }
        
        // مدیریت سطح
        void LoadLevel(int level);
        void RestartLevel();
        void CompleteLevel();
        void FailLevel();
        void NextLevel();
        
        // مدیریت بازی
        void StartGame(GameMode mode);
        void PauseGame();
        void ResumeGame();
        void EndGame(bool victory);
        void AddScore(int points);
        void AddCombo();
        void ResetCombo();
        
        // مدیریت منابع
        void AddLife();
        void RemoveLife();
        void AddContinue();
        void UseContinue();
        
        // رویدادها
        void RegisterEvent(const std::string& eventName, std::function<void()> callback);
        void TriggerEvent(const std::string& eventName);
        void RegisterStateChangeCallback(std::function<void(GameState, GameState)> callback);
        
        // ذخیره‌سازی و بارگذاری
        bool SaveGame(const std::string& slot = "auto");
        bool LoadGame(const std::string& slot = "auto");
        bool DeleteSave(const std::string& slot);
        bool HasSaveGame(const std::string& slot = "auto") const;
        
        // تنظیمات
        void ApplySettings(const GameSettings& newSettings);
        const GameSettings& GetSettings() const { return settings_; }
        GameSettings& GetMutableSettings() { return settings_; }
        
        // آمار
        const GameStats& GetStats() const { return stats_; }
        GameStats& GetMutableStats() { return stats_; }
        const GameProgress& GetProgress() const { return progress_; }
        GameProgress& GetMutableProgress() { return progress_; }
        
        // اطلاعات بازی
        int GetCurrentScore() const { return currentScore_; }
        int GetCurrentLevel() const { return currentLevel_; }
        int GetCurrentWave() const { return currentWave_; }
        int GetLives() const { return lives_; }
        int GetContinues() const { return continues_; }
        int GetMultiplier() const { return multiplier_; }
        int GetComboCount() const { return comboCount_; }
        float GetGameTime() const { return gameTime_; }
        float GetLevelTime() const { return levelTime_; }
        bool IsLevelComplete() const { return levelComplete_; }
        bool IsBossSpawned() const { return bossSpawned_; }
        
        // دسترسی به مدیران
        PlayerManager* GetPlayerManager() const { return playerManager_.get(); }
        EnemyManager* GetEnemyManager() const { return enemyManager_.get(); }
        CoinManager* GetCoinManager() const { return coinManager_.get(); }
        GameEngine* GetGameEngine() const { return gameEngine_.get(); }
        RenderSystem* GetRenderSystem() const { return renderSystem_.get(); }
        PhysicsEngine* GetPhysicsEngine() const { return physicsEngine_.get(); }
        AudioManager* GetAudioManager() const { return audioManager_.get(); }
        InputHandler* GetInputHandler() const { return inputHandler_.get(); }
        
    private:
        // به‌روزرسانی بر اساس وضعیت
        void UpdateBoot(float deltaTime);
        void UpdateMainMenu(float deltaTime);
        void UpdateLoading(float deltaTime);
        void UpdatePlaying(float deltaTime);
        void UpdatePaused(float deltaTime);
        void UpdateLevelComplete(float deltaTime);
        void UpdateGameOver(float deltaTime);
        void UpdateUpgradeShop(float deltaTime);
        
        // رندر بر اساس وضعیت
        void RenderBoot();
        void RenderMainMenu();
        void RenderLoading();
        void RenderPlaying();
        void RenderPaused();
        void RenderLevelComplete();
        void RenderGameOver();
        void RenderUpgradeShop();
        
        // مدیریت وضعیت
        void InitializeState(GameState state);
        void CleanupState(GameState state);
        void OnStateEnter(GameState newState);
        void OnStateExit(GameState oldState);
        
        // مدیریت سطح
        void SetupLevel(int level);
        void CleanupLevel();
        void SpawnWave(int wave);
        void CheckWaveCompletion();
        void SpawnBoss();
        
        // امتیاز و کامبو
        void UpdateCombo(float deltaTime);
        void CalculateMultiplier();
        void ApplyScoreEffects();
        
        // ذخیره‌سازی
        void InitializeSaveSystem();
        std::string GetSaveFilePath(const std::string& slot) const;
        bool BackupSave(const std::string& slot);
        
        // رویدادهای سیستمی
        void OnPlayerDeath();
        void OnBossDefeated();
        void OnSecretFound();
        void OnAchievementUnlocked(const std::string& achievementId);
    };

    // سیستم دستاوردها
    class AchievementSystem {
    private:
        struct Achievement {
            std::string id;
            std::string name;
            std::string description;
            std::string iconPath;
            bool unlocked;
            float progress;
            float target;
            std::function<bool(const GameStats&, const GameProgress&)> checkCondition;
            std::function<void()> onUnlock;
            
            Achievement(const std::string& id, const std::string& n, const std::string& desc,
                       float t, std::function<bool(const GameStats&, const GameProgress&)> condition)
                : id(id), name(n), description(desc), unlocked(false),
                  progress(0.0f), target(t), checkCondition(condition) {}
        };
        
        std::vector<Achievement> achievements_;
        GameManager* gameManager_;
        
    public:
        AchievementSystem(GameManager* manager);
        ~AchievementSystem();
        
        void Initialize();
        void Update(float deltaTime);
        
        void UnlockAchievement(const std::string& achievementId);
        void ResetAchievement(const std::string& achievementId);
        void ResetAllAchievements();
        
        bool IsAchievementUnlocked(const std::string& achievementId) const;
        float GetAchievementProgress(const std::string& achievementId) const;
        int GetUnlockedCount() const;
        int GetTotalCount() const;
        
        const std::vector<Achievement>& GetAllAchievements() const { return achievements_; }
        std::vector<Achievement> GetUnlockedAchievements() const;
        std::vector<Achievement> GetLockedAchievements() const;
        
    private:
        void LoadAchievements();
        void CheckAchievements();
        Achievement* FindAchievement(const std::string& id);
        const Achievement* FindAchievement(const std::string& id) const;
        void CreateUnlockEffect(const Achievement& achievement);
    };

    // سیستم چالش‌ها
    class ChallengeSystem {
    private:
        struct Challenge {
            std::string id;
            std::string name;
            std::string description;
            std::string difficulty;
            bool completed;
            float progress;
            float target;
            std::function<bool(const GameStats&)> checkCompletion;
            std::function<void()> reward;
            time_t startTime;
            time_t endTime;
            
            Challenge(const std::string& id, const std::string& n, const std::string& desc,
                     float t, std::function<bool(const GameStats&)> condition)
                : id(id), name(n), description(desc), completed(false),
                  progress(0.0f), target(t), checkCompletion(condition),
                  startTime(0), endTime(0) {}
        };
        
        std::vector<Challenge> activeChallenges_;
        std::vector<Challenge> completedChallenges_;
        GameManager* gameManager_;
        
    public:
        ChallengeSystem(GameManager* manager);
        ~ChallengeSystem();
        
        void Initialize();
        void Update(float deltaTime);
        
        void AddChallenge(const Challenge& challenge);
        void RemoveChallenge(const std::string& challengeId);
        void CompleteChallenge(const std::string& challengeId);
        
        bool IsChallengeActive(const std::string& challengeId) const;
        bool IsChallengeCompleted(const std::string& challengeId) const;
        float GetChallengeProgress(const std::string& challengeId) const;
        
        const std::vector<Challenge>& GetActiveChallenges() const { return activeChallenges_; }
        const std::vector<Challenge>& GetCompletedChallenges() const { return completedChallenges_; }
        int GetActiveCount() const { return static_cast<int>(activeChallenges_.size()); }
        int GetCompletedCount() const { return static_cast<int>(completedChallenges_.size()); }
        
    private:
        void LoadDailyChallenges();
        void LoadWeeklyChallenges();
        void CheckChallenges();
        Challenge* FindActiveChallenge(const std::string& id);
        void CreateCompletionEffect(const Challenge& challenge);
    };

    // سیستم جدول امتیازات
    class LeaderboardSystem {
    private:
        struct LeaderboardEntry {
            std::string playerName;
            int score;
            int level;
            GameMode mode;
            Difficulty difficulty;
            time_t timestamp;
            std::string replayData;
            
            LeaderboardEntry(const std::string& name, int s, GameMode m, Difficulty d)
                : playerName(name), score(s), level(1), mode(m), difficulty(d),
                  timestamp(time(nullptr)) {}
            
            bool operator<(const LeaderboardEntry& other) const {
                return score > other.score; // مرتب سازی نزولی
            }
        };
        
        std::vector<LeaderboardEntry> leaderboards_[6]; // برای هر حالت بازی
        GameManager* gameManager_;
        std::string leaderboardFile_;
        
    public:
        LeaderboardSystem(GameManager* manager);
        ~LeaderboardSystem();
        
        bool Initialize();
        void Cleanup();
        
        void SubmitScore(const std::string& playerName, int score, GameMode mode, Difficulty difficulty);
        std::vector<LeaderboardEntry> GetLeaderboard(GameMode mode, int maxEntries = 10) const;
        int GetPlayerRank(const std::string& playerName, GameMode mode) const;
        LeaderboardEntry GetPlayerBest(GameMode mode) const;
        
        bool SaveLeaderboards();
        bool LoadLeaderboards();
        void ClearLeaderboards();
        
        int GetTotalEntries() const;
        int GetEntriesCount(GameMode mode) const;
        
    private:
        int GetLeaderboardIndex(GameMode mode) const;
        void SortLeaderboard(int index);
        bool ValidateEntry(const LeaderboardEntry& entry) const;
        void CreateNewEntryEffect(const LeaderboardEntry& entry);
    };

    // سیستم تحلیل و آمار
    class AnalyticsSystem {
    private:
        struct GameSession {
            std::string sessionId;
            time_t startTime;
            time_t endTime;
            GameMode mode;
            Difficulty difficulty;
            int finalScore;
            int levelReached;
            int enemiesKilled;
            int coinsCollected;
            float playTime;
            std::string endReason;
            
            GameSession(GameMode m, Difficulty d)
                : sessionId(""), startTime(time(nullptr)), endTime(0),
                  mode(m), difficulty(d), finalScore(0), levelReached(1),
                  enemiesKilled(0), coinsCollected(0), playTime(0.0f),
                  endReason("unknown") {}
        };
        
        std::vector<GameSession> sessionHistory_;
        GameSession* currentSession_;
        GameManager* gameManager_;
        std::string analyticsFile_;
        
    public:
        AnalyticsSystem(GameManager* manager);
        ~AnalyticsSystem();
        
        void Initialize();
        void Cleanup();
        
        void StartSession(GameMode mode, Difficulty difficulty);
        void EndSession(const std::string& reason = "normal");
        void UpdateSession(float deltaTime);
        
        void RecordEvent(const std::string& eventName, const std::unordered_map<std::string, std::string>& properties = {});
        void RecordDeath(const glm::vec3& position, const std::string& cause);
        void RecordPurchase(const std::string& item, int cost);
        
        const GameSession* GetCurrentSession() const { return currentSession_; }
        std::vector<GameSession> GetSessionHistory(int maxSessions = 50) const;
        GameSession GetBestSession() const;
        
        // آمار تحلیلی
        float GetAveragePlayTime() const;
        float GetAverageScore() const;
        float GetCompletionRate() const;
        std::string GetMostPlayedMode() const;
        std::string GetMostCommonDeathCause() const;
        
        bool SaveAnalytics();
        bool LoadAnalytics();
        
    private:
        std::string GenerateSessionId() const;
        void CalculateSessionStats(GameSession& session);
        void SendAnalyticsData(const GameSession& session);
    };

    // سیستم رویدادهای فصلی
    class SeasonalEventSystem {
    private:
        struct SeasonalEvent {
            std::string id;
            std::string name;
            std::string description;
            time_t startDate;
            time_t endDate;
            bool active;
            std::unordered_map<std::string, int> rewards;
            std::function<bool()> completionCondition;
            
            SeasonalEvent(const std::string& id, const std::string& n, const std::string& desc,
                         time_t start, time_t end)
                : id(id), name(n), description(desc), startDate(start), endDate(end),
                  active(false) {}
        };
        
        std::vector<SeasonalEvent> seasonalEvents_;
        GameManager* gameManager_;
        
    public:
        SeasonalEventSystem(GameManager* manager);
        ~SeasonalEventSystem();
        
        void Initialize();
        void Update(float deltaTime);
        
        void AddEvent(const SeasonalEvent& event);
        void RemoveEvent(const std::string& eventId);
        void CompleteEvent(const std::string& eventId);
        
        bool IsEventActive(const std::string& eventId) const;
        bool IsEventAvailable(const std::string& eventId) const;
        time_t GetEventTimeRemaining(const std::string& eventId) const;
        
        const std::vector<SeasonalEvent>& GetAllEvents() const { return seasonalEvents_; }
        std::vector<SeasonalEvent> GetActiveEvents() const;
        std::vector<SeasonalEvent> GetUpcomingEvents() const;
        
    private:
        void CheckEventStatus();
        void LoadHolidayEvents();
        SeasonalEvent* FindEvent(const std::string& id);
        void ActivateEvent(SeasonalEvent& event);
        void DeactivateEvent(SeasonalEvent& event);
        void CreateEventNotification(const SeasonalEvent& event);
    };

    // سیستم نوتیفیکیشن
    class NotificationSystem {
    private:
        struct Notification {
            std::string id;
            std::string title;
            std::string message;
            std::string icon;
            float duration;
            float timer;
            bool important;
            std::function<void()> onClick;
            
            Notification(const std::string& id, const std::string& t, const std::string& msg,
                        float dur = 5.0f)
                : id(id), title(t), message(msg), duration(dur), timer(dur),
                  important(false) {}
        };
        
        std::vector<Notification> activeNotifications_;
        std::vector<Notification> notificationQueue_;
        GameManager* gameManager_;
        
    public:
        NotificationSystem(GameManager* manager);
        ~NotificationSystem();
        
        void Update(float deltaTime);
        void Render();
        
        void ShowNotification(const std::string& title, const std::string& message, 
                            float duration = 5.0f, bool important = false);
        void ShowAchievementNotification(const std::string& achievementName, 
                                       const std::string& achievementDesc);
        void ShowLevelUpNotification(int newLevel);
        void ShowRewardNotification(const std::string& reward, int amount);
        
        void ClearNotifications();
        void ClearNonImportantNotifications();
        int GetActiveNotificationCount() const { return static_cast<int>(activeNotifications_.size()); }
        
    private:
        void ProcessQueue();
        void UpdateNotifications(float deltaTime);
        void RenderNotification(const Notification& notification, int index);
        Notification* FindNotification(const std::string& id);
    };

} // namespace GalacticOdyssey

#endif // GAME_MANAGER_H
