#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Input/InputHandler.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

namespace GalacticOdyssey {

    // وضعیت‌های مختلف بازیکن
    enum class PlayerState {
        IDLE,           // حالت عادی
        MOVING,         // در حال حرکت
        BOOSTING,       // در حال تقویت سرعت
        DASHING,        // در حال حرکت سریع
        SHOOTING,       // در حال شلیک
        DAMAGED,        // آسیب دیده
        INVULNERABLE,   // حالت مصونیت
        DEAD,           // مرده
        RESPAWNING      // در حال تولد مجدد
    };

    // انواع قدرت‌ها و توانایی‌ها
    enum class PowerUpType {
        SPEED_BOOST,    // افزایش سرعت
        RAPID_FIRE,     // شلیک سریع
        SHIELD,         // محافظ
        MAGNET,         // جذب سکه‌ها
        BOMB,           // بمب
        TIME_SLOW,      // کاهش زمان
        MULTI_SHOT,     // شلیک چندگانه
        HEALTH_BOOST    // افزایش سلامتی
    };

    // ساختار آمار بازیکن
    struct PlayerStats {
        int level;
        int experience;
        int experienceToNextLevel;
        int health;
        int maxHealth;
        int shields;
        int maxShields;
        float speed;
        float rotationSpeed;
        float fireRate;
        int damage;
        int bombCount;
        int coinsCollected;
        int enemiesDestroyed;
        float playTime;
        
        PlayerStats() 
            : level(1), experience(0), experienceToNextLevel(100),
              health(100), maxHealth(100), shields(0), maxShields(100),
              speed(5.0f), rotationSpeed(180.0f), fireRate(0.5f), damage(10),
              bombCount(3), coinsCollected(0), enemiesDestroyed(0), playTime(0.0f) {}
    };

    // ساختار قدرت‌های فعال
    struct ActivePowerUp {
        PowerUpType type;
        float duration;
        float timeRemaining;
        float intensity;
        
        ActivePowerUp(PowerUpType powerType, float dur, float intense = 1.0f)
            : type(powerType), duration(dur), timeRemaining(dur), intensity(intense) {}
    };

    // ساختار سلاح
    struct Weapon {
        std::string name;
        float fireRate;
        float damage;
        float projectileSpeed;
        int projectileCount;
        float spreadAngle;
        bool isHoming;
        float energyCost;
        float cooldown;
        float currentCooldown;
        
        Weapon(const std::string& weaponName = "Laser")
            : name(weaponName), fireRate(0.5f), damage(10.0f), projectileSpeed(15.0f),
              projectileCount(1), spreadAngle(0.0f), isHoming(false),
              energyCost(5.0f), cooldown(0.0f), currentCooldown(0.0f) {}
    };

    // کلاس اصلی بازیکن
    class Player {
    private:
        // سیستم‌های وابسته
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        InputHandler* inputHandler_;
        
        // وضعیت و موقعیت
        PlayerState currentState_;
        PlayerStats stats_;
        glm::vec3 position_;
        glm::vec3 velocity_;
        glm::vec3 acceleration_;
        glm::vec3 rotation_;
        glm::vec3 targetRotation_;
        
        // فیزیک
        RigidBody* physicsBody_;
        float mass_;
        float drag_;
        float angularDrag_;
        
        // گرافیک
        Model3D* shipModel_;
        Texture* shipTexture_;
        Shader* playerShader_;
        glm::vec3 modelScale_;
        
        // سلاح‌ها و قدرت‌ها
        std::vector<Weapon> weapons_;
        int currentWeaponIndex_;
        std::vector<ActivePowerUp> activePowerUps_;
        std::vector<class Projectile*> activeProjectiles_;
        
        // تایمرها و کوoldاون‌ها
        float fireTimer_;
        float dashTimer_;
        float invulnerabilityTimer_;
        float respawnTimer_;
        float boostTimer_;
        
        // ورودی و کنترل
        glm::vec2 inputDirection_;
        glm::vec2 lookDirection_;
        bool isFiring_;
        bool isBoosting_;
        bool isDashing_;
        
        // افکت‌های بصری
        class ParticleSystem* engineParticles_;
        class ParticleSystem* boostParticles_;
        class ParticleSystem* shieldParticles_;
        bool showTrail_;
        float trailTimer_;
        
        // صداها
        std::string engineSound_;
        std::string shootSound_;
        std::string boostSound_;
        std::string damageSound_;
        
        // سیستم انیمیشن
        float animationTime_;
        glm::vec3 wobbleOffset_;
        glm::vec3 engineGlowColor_;
        
    public:
        Player();
        ~Player();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, InputHandler* input);
        void Cleanup();
        
        void Update(float deltaTime);
        void Render();
        
        // حرکت و کنترل
        void Move(const glm::vec2& direction);
        void LookAt(const glm::vec3& target);
        void Boost(bool enable);
        void Dash(const glm::vec3& direction);
        void Stop();
        
        // اقدامات
        void Fire();
        void UseBomb();
        void ActivateShield();
        void CollectCoin(int value = 1);
        void TakeDamage(int damage, const glm::vec3& source = glm::vec3(0.0f));
        void Heal(int amount);
        void AddExperience(int exp);
        
        // قدرت‌ها و ارتقاها
        void AddPowerUp(PowerUpType type, float duration, float intensity = 1.0f);
        void RemovePowerUp(PowerUpType type);
        void UpgradeWeapon();
        void UpgradeShield();
        void UpgradeEngine();
        
        // وضعیت‌ها
        void SetState(PlayerState newState);
        void Respawn();
        void Reset();
        
        // اطلاعات
        const glm::vec3& GetPosition() const { return position_; }
        const glm::vec3& GetVelocity() const { return velocity_; }
        const PlayerStats& GetStats() const { return stats_; }
        PlayerState GetState() const { return currentState_; }
        bool IsAlive() const { return currentState_ != PlayerState::DEAD; }
        bool IsInvulnerable() const { return currentState_ == PlayerState::INVULNERABLE; }
        float GetCollisionRadius() const { return 2.0f; }
        
        // تنظیمات
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& rotation);
        void SetModelScale(const glm::vec3& scale) { modelScale_ = scale; }
        
    private:
        // به‌روزرسانی زیرسیستم‌ها
        void UpdatePhysics(float deltaTime);
        void UpdateInput(float deltaTime);
        void UpdateWeapons(float deltaTime);
        void UpdatePowerUps(float deltaTime);
        void UpdateAnimations(float deltaTime);
        void UpdateParticles(float deltaTime);
        void UpdateAudio(float deltaTime);
        
        // محاسبات
        void CalculateMovement(float deltaTime);
        void CalculateRotation(float deltaTime);
        void CalculateEngineEffects(float deltaTime);
        
        // ایجاد افکت‌ها
        void CreateEngineParticles();
        void CreateBoostParticles();
        void CreateShieldEffect();
        void CreateDamageEffect(const glm::vec3& source);
        void CreateExplosionEffect();
        
        // مدیریت قدرت‌ها
        void ApplyPowerUpEffects();
        void RemoveExpiredPowerUps();
        float GetPowerUpMultiplier(PowerUpType type) const;
        
        // کمک‌کننده‌ها
        bool CanFire() const;
        bool CanDash() const;
        bool CanBoost() const;
        void ClampToBounds();
        void UpdateStatsFromPowerUps();
    };

    // سیستم ارتقا و پیشرفت بازیکن
    class PlayerProgression {
    private:
        struct Upgrade {
            std::string name;
            std::string description;
            int cost;
            int level;
            int maxLevel;
            std::function<void(PlayerStats&)> applyUpgrade;
            
            Upgrade(const std::string& n, const std::string& desc, int c, 
                   std::function<void(PlayerStats&)> apply)
                : name(n), description(desc), cost(c), level(0), maxLevel(5), applyUpgrade(apply) {}
        };
        
        std::vector<Upgrade> availableUpgrades_;
        Player* player_;
        int skillPoints_;
        
    public:
        PlayerProgression(Player* player);
        ~PlayerProgression();
        
        void Initialize();
        void Update(float deltaTime);
        
        void AddSkillPoint(int count = 1);
        bool PurchaseUpgrade(const std::string& upgradeName);
        void ResetUpgrades();
        
        const std::vector<Upgrade>& GetAvailableUpgrades() const { return availableUpgrades_; }
        int GetSkillPoints() const { return skillPoints_; }
        int GetTotalUpgrades() const;
        
    private:
        void CreateDefaultUpgrades();
        Upgrade* FindUpgrade(const std::string& name);
    };

    // سیستم کاستومایزیشن بازیکن
    class PlayerCustomization {
    private:
        struct ShipSkin {
            std::string name;
            std::string modelPath;
            std::string texturePath;
            glm::vec3 baseColor;
            glm::vec3 accentColor;
            bool unlocked;
            int unlockCost;
            
            ShipSkin(const std::string& n, const std::string& model, const std::string& tex)
                : name(n), modelPath(model), texturePath(tex),
                  baseColor(0.2f, 0.4f, 0.8f), accentColor(1.0f, 0.8f, 0.2f),
                  unlocked(false), unlockCost(0) {}
        };
        
        struct TrailEffect {
            std::string name;
            glm::vec3 color;
            float width;
            float lifetime;
            bool unlocked;
            
            TrailEffect(const std::string& n, const glm::vec3& col)
                : name(n), color(col), width(1.0f), lifetime(2.0f), unlocked(false) {}
        };
        
        std::vector<ShipSkin> availableSkins_;
        std::vector<TrailEffect> availableTrails_;
        Player* player_;
        int currentSkinIndex_;
        int currentTrailIndex_;
        
    public:
        PlayerCustomization(Player* player);
        ~PlayerCustomization();
        
        void Initialize();
        void ApplyCustomization();
        
        void UnlockSkin(const std::string& skinName);
        void UnlockTrail(const std::string& trailName);
        void SelectSkin(int index);
        void SelectTrail(int index);
        bool CanAffordSkin(const std::string& skinName) const;
        
        const std::vector<ShipSkin>& GetSkins() const { return availableSkins_; }
        const std::vector<TrailEffect>& GetTrails() const { return availableTrails_; }
        int GetCurrentSkinIndex() const { return currentSkinIndex_; }
        int GetCurrentTrailIndex() const { return currentTrailIndex_; }
        
    private:
        void LoadDefaultSkins();
        void LoadDefaultTrails();
        ShipSkin* FindSkin(const std::string& name);
        TrailEffect* FindTrail(const std::string& name);
    };

    // سیستم دستاوردهای بازیکن
    class PlayerAchievements {
    private:
        struct Achievement {
            std::string id;
            std::string name;
            std::string description;
            bool unlocked;
            float progress;
            float target;
            std::function<bool(const PlayerStats&)> checkCondition;
            
            Achievement(const std::string& id, const std::string& n, const std::string& desc,
                       float t, std::function<bool(const PlayerStats&)> condition)
                : id(id), name(n), description(desc), unlocked(false),
                  progress(0.0f), target(t), checkCondition(condition) {}
        };
        
        std::vector<Achievement> achievements_;
        Player* player_;
        
    public:
        PlayerAchievements(Player* player);
        ~PlayerAchievements();
        
        void Initialize();
        void Update(float deltaTime);
        
        void UnlockAchievement(const std::string& achievementId);
        float GetAchievementProgress(const std::string& achievementId) const;
        bool IsAchievementUnlocked(const std::string& achievementId) const;
        int GetUnlockedCount() const;
        
        const std::vector<Achievement>& GetAllAchievements() const { return achievements_; }
        
    private:
        void CreateDefaultAchievements();
        Achievement* FindAchievement(const std::string& id);
        void CheckAchievements();
    };

    // سیستم موجودی بازیکن
    class PlayerInventory {
    private:
        struct InventoryItem {
            std::string id;
            std::string name;
            std::string description;
            int quantity;
            int maxStack;
            std::function<void(Player*)> useEffect;
            
            InventoryItem(const std::string& id, const std::string& n, const std::string& desc,
                         int maxStack, std::function<void(Player*)> effect)
                : id(id), name(n), description(desc), quantity(0), maxStack(maxStack), useEffect(effect) {}
        };
        
        std::vector<InventoryItem> items_;
        Player* player_;
        int maxSlots_;
        int coins_;
        
    public:
        PlayerInventory(Player* player, int maxSlots = 20);
        ~PlayerInventory();
        
        void Initialize();
        
        bool AddItem(const std::string& itemId, int quantity = 1);
        bool RemoveItem(const std::string& itemId, int quantity = 1);
        bool UseItem(const std::string& itemId);
        bool HasItem(const std::string& itemId) const;
        int GetItemCount(const std::string& itemId) const;
        
        void AddCoins(int amount) { coins_ += amount; }
        bool SpendCoins(int amount);
        int GetCoins() const { return coins_; }
        int GetUsedSlots() const;
        int GetMaxSlots() const { return maxSlots_; }
        
    private:
        void CreateDefaultItems();
        InventoryItem* FindItem(const std::string& id);
        const InventoryItem* FindItem(const std::string& id) const;
    };

    // سیستم ماموریت‌های بازیکن
    class PlayerQuests {
    private:
        struct Quest {
            std::string id;
            std::string title;
            std::string description;
            bool completed;
            bool active;
            std::vector<std::string> objectives;
            std::vector<bool> objectiveCompleted;
            std::function<void(Player*)> reward;
            
            Quest(const std::string& id, const std::string& t, const std::string& desc,
                  std::function<void(Player*)> rewardFunc)
                : id(id), title(t), description(desc), completed(false), active(false), reward(rewardFunc) {}
        };
        
        std::vector<Quest> quests_;
        Player* player_;
        int activeQuestLimit_;
        
    public:
        PlayerQuests(Player* player, int activeLimit = 3);
        ~PlayerQuests();
        
        void Initialize();
        void Update(float deltaTime);
        
        bool StartQuest(const std::string& questId);
        bool CompleteQuest(const std::string& questId);
        bool AbandonQuest(const std::string& questId);
        bool IsQuestActive(const std::string& questId) const;
        bool IsQuestCompleted(const std::string& questId) const;
        
        const std::vector<Quest>& GetAllQuests() const { return quests_; }
        std::vector<Quest> GetActiveQuests() const;
        std::vector<Quest> GetCompletedQuests() const;
        
    private:
        void CreateDefaultQuests();
        Quest* FindQuest(const std::string& id);
        void CheckQuestProgress();
    };

    // مدیر جامع بازیکن
    class PlayerManager {
    private:
        static PlayerManager* instance_;
        
        std::unique_ptr<Player> player_;
        std::unique_ptr<PlayerProgression> progression_;
        std::unique_ptr<PlayerCustomization> customization_;
        std::unique_ptr<PlayerAchievements> achievements_;
        std::unique_ptr<PlayerInventory> inventory_;
        std::unique_ptr<PlayerQuests> quests_;
        
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        InputHandler* inputHandler_;
        
    public:
        PlayerManager();
        ~PlayerManager();
        
        static PlayerManager& GetInstance();
        static void DestroyInstance();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics,
                       AudioManager* audio, InputHandler* input);
        void Cleanup();
        
        void Update(float deltaTime);
        void Render();
        
        // دسترسی به کامپوننت‌ها
        Player* GetPlayer() const { return player_.get(); }
        PlayerProgression* GetProgression() const { return progression_.get(); }
        PlayerCustomization* GetCustomization() const { return customization_.get(); }
        PlayerAchievements* GetAchievements() const { return achievements_.get(); }
        PlayerInventory* GetInventory() const { return inventory_.get(); }
        PlayerQuests* GetQuests() const { return quests_.get(); }
        
        // مدیریت ذخیره و بارگذاری
        bool SavePlayerData(const std::string& filename);
        bool LoadPlayerData(const std::string& filename);
        void ResetPlayerData();
        
    private:
        void InitializeSubsystems();
    };

} // namespace GalacticOdyssey

#endif // PLAYER_H
