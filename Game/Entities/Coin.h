#pragma once
#ifndef COIN_H
#define COIN_H

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Audio/AudioManager.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace GalacticOdyssey {

    // انواع سکه‌ها و منابع
    enum class CoinType {
        BRONZE_COIN,    // سکه برنزی
        SILVER_COIN,    // سکه نقره‌ای
        GOLD_COIN,      // سکه طلایی
        PLATINUM_COIN,  // سکه پلاتینیومی
        CRYSTAL,        // کریستال
        GEM,            // جواهر
        ENERGY_ORB,     // گوی انرژی
        HEALTH_ORB,     // گوی سلامت
        SHIELD_ORB,     // گوی محافظ
        EXPERIENCE_ORB, // گوی تجربه
        POWER_UP,       // قدرت موقت
        COLLECTIBLE,    // کلکسیونی
        TREASURE_CHEST, // صندوق گنج
        BOSS_DROP,      // افتاده از باس
        SECRET          // مخفی
    };

    // وضعیت سکه
    enum class CoinState {
        SPAWNING,       // در حال ظاهر شدن
        IDLE,           // حالت عادی
        COLLECTING,     // در حال جمع‌آوری
        MAGNETIZED,     // جذب شده به بازیکن
        HIDDEN,         // مخفی
        DISAPPEARING,   // در حال ناپدید شدن
        COLLECTED       // جمع‌آوری شده
    };

    // نادرتی سکه‌ها
    enum class Rarity {
        COMMON,         // معمولی
        UNCOMMON,       // غیرمعمول
        RARE,           // نادر
        EPIC,           // اپیک
        LEGENDARY,      // افسانه‌ای
        MYTHIC          // اسطوره‌ای
    };

    // ساختار آمار سکه
    struct CoinStats {
        int value;              // ارزش پایه
        float experience;       // تجربه اعطایی
        float magnetRange;      // شعاع جذب
        float collectionTime;   // زمان جمع‌آوری
        float lifetime;         // طول عمر
        float rotationSpeed;    // سرعت چرخش
        float floatAmplitude;   // دامنه شناوری
        float floatFrequency;   // فرکانس شناوری
        glm::vec3 glowColor;    // رنگ درخشش
        float glowIntensity;    // شدت درخشش
        
        CoinStats() 
            : value(1), experience(0.0f), magnetRange(3.0f),
              collectionTime(0.5f), lifetime(30.0f),
              rotationSpeed(90.0f), floatAmplitude(0.5f),
              floatFrequency(1.0f), glowColor(1.0f, 0.8f, 0.2f),
              glowIntensity(1.0f) {}
    };

    // ساختار اثرات سکه
    struct CoinEffects {
        std::function<void(class Player*)> onCollect;
        std::function<void(class Player*)> whileHeld;
        std::function<void(class Coin*)> onSpawn;
        std::function<void(class Coin*)> onDisappear;
        
        CoinEffects() = default;
    };

    // کلاس اصلی سکه
    class Coin {
    protected:
        // سیستم‌های وابسته
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        
        // هویت و نوع
        CoinType type_;
        Rarity rarity_;
        std::string id_;
        int level_;
        
        // وضعیت و موقعیت
        CoinState currentState_;
        CoinStats stats_;
        glm::vec3 position_;
        glm::vec3 velocity_;
        glm::vec3 rotation_;
        glm::vec3 targetPosition_;
        
        // فیزیک
        RigidBody* physicsBody_;
        float collisionRadius_;
        
        // گرافیک
        Model3D* coinModel_;
        Texture* coinTexture_;
        Shader* coinShader_;
        glm::vec3 modelScale_;
        
        // تایمرها
        float stateTimer_;
        float lifetimeTimer_;
        float collectionTimer_;
        float floatTimer_;
        float glowTimer_;
        
        // جذب و حرکت
        class Player* targetPlayer_;
        bool isMagnetized_;
        float magnetStrength_;
        
        // افکت‌ها
        class ParticleSystem* collectParticles_;
        class ParticleSystem* glowParticles_;
        class ParticleSystem* trailParticles_;
        glm::vec3 baseColor_;
        glm::vec3 rareColor_;
        
        // صداها
        std::string spawnSound_;
        std::string collectSound_;
        std::string magnetSound_;
        
        // انیمیشن
        float animationTime_;
        glm::vec3 floatOffset_;
        bool isPulsing_;
        float pulsePhase_;
        
        // اثرات
        CoinEffects effects_;
        
    public:
        Coin(CoinType type = CoinType::BRONZE_COIN, Rarity rarity = Rarity::COMMON);
        virtual ~Coin();
        
        virtual bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                               AudioManager* audio, class Player* player = nullptr);
        virtual void Cleanup();
        
        virtual void Update(float deltaTime);
        virtual void Render();
        
        // مدیریت وضعیت
        virtual void SetState(CoinState newState);
        virtual void Collect();
        virtual void Magnetize(class Player* player);
        virtual void Hide();
        virtual void Show();
        
        // حرکت و فیزیک
        virtual void MoveTo(const glm::vec3& target);
        virtual void ApplyForce(const glm::vec3& force);
        virtual void Bounce();
        
        // اطلاعات
        const glm::vec3& GetPosition() const { return position_; }
        const CoinStats& GetStats() const { return stats_; }
        CoinType GetType() const { return type_; }
        Rarity GetRarity() const { return rarity_; }
        CoinState GetState() const { return currentState_; }
        bool IsCollectible() const { return currentState_ == CoinState::IDLE || 
                                        currentState_ == CoinState::MAGNETIZED; }
        bool IsCollected() const { return currentState_ == CoinState::COLLECTED; }
        float GetCollisionRadius() const { return collisionRadius_; }
        const std::string& GetId() const { return id_; }
        
        // تنظیمات
        virtual void SetPosition(const glm::vec3& position);
        virtual void SetTargetPlayer(class Player* player) { targetPlayer_ = player; }
        virtual void SetStats(const CoinStats& newStats) { stats_ = newStats; }
        virtual void SetEffects(const CoinEffects& newEffects) { effects_ = newEffects; }
        
        // اثرات ویژه
        virtual void ApplyTemporaryEffect(class Player* player);
        virtual void CreateChainReaction();
        virtual void MultiplyValue(int multiplier);
        
    protected:
        // به‌روزرسانی زیرسیستم‌ها
        virtual void UpdatePhysics(float deltaTime);
        virtual void UpdateAnimation(float deltaTime);
        virtual void UpdateMagnet(float deltaTime);
        virtual void UpdateParticles(float deltaTime);
        virtual void UpdateAudio(float deltaTime);
        
        // محاسبات
        virtual void CalculateFloatMotion(float deltaTime);
        virtual void CalculateRotation(float deltaTime);
        virtual void CalculateMagnetForce(float deltaTime);
        virtual void CalculateGlowEffect(float deltaTime);
        
        // ایجاد افکت‌ها
        virtual void CreateSpawnEffect();
        virtual void CreateCollectEffect();
        virtual void CreateMagnetEffect();
        virtual void CreateGlowEffect();
        virtual void CreatePulseEffect();
        
        // کمک‌کننده‌ها
        virtual void ClampToBounds();
        virtual float DistanceToPlayer() const;
        virtual bool ShouldMagnetize() const;
        
        // تنظیمات بر اساس نوع و نادرتی
        virtual void SetupFromTypeAndRarity();
        virtual void ApplyRarityMultipliers();
    };

    // کلاس سکه پایه
    class BasicCoin : public Coin {
    public:
        BasicCoin(CoinType type = CoinType::BRONZE_COIN, Rarity rarity = Rarity::COMMON);
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        
    protected:
        void SetupFromTypeAndRarity() override;
    };

    // کلاس گوی انرژی
    class EnergyOrb : public Coin {
    private:
        float energyAmount_;
        float rechargeRate_;
        
    public:
        EnergyOrb(Rarity rarity = Rarity::COMMON);
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void ApplyTemporaryEffect(class Player* player) override;
        
    protected:
        void SetupFromTypeAndRarity() override;
    };

    // کلاس گوی سلامت
    class HealthOrb : public Coin {
    private:
        float healAmount_;
        float overhealPercent_;
        
    public:
        HealthOrb(Rarity rarity = Rarity::UNCOMMON);
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void ApplyTemporaryEffect(class Player* player) override;
        
    protected:
        void SetupFromTypeAndRarity() override;
    };

    // کلاس گوی تجربه
    class ExperienceOrb : public Coin {
    private:
        float expMultiplier_;
        float bonusExpChance_;
        
    public:
        ExperienceOrb(Rarity rarity = Rarity::RARE);
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void ApplyTemporaryEffect(class Player* player) override;
        
    protected:
        void SetupFromTypeAndRarity() override;
    };

    // کلاس قدرت موقت
    class PowerUpCoin : public Coin {
    private:
        std::string powerUpType_;
        float duration_;
        float intensity_;
        
    public:
        PowerUpCoin(const std::string& powerType, Rarity rarity = Rarity::EPIC);
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void ApplyTemporaryEffect(class Player* player) override;
        
    protected:
        void SetupFromTypeAndRarity() override;
    };

    // کلاس صندوق گنج
    class TreasureChest : public Coin {
    private:
        std::vector<CoinType> possibleLoot_;
        int minLootCount_;
        int maxLootCount_;
        bool isLocked_;
        float unlockTime_;
        
    public:
        TreasureChest(Rarity rarity = Rarity::LEGENDARY);
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void Update(float deltaTime) override;
        void Collect() override;
        void Unlock();
        void SpawnLoot();
        
    protected:
        void SetupFromTypeAndRarity() override;
        void CreateUnlockEffect();
    };

    // سیستم اسپان سکه‌ها
    class CoinSpawner {
    private:
        struct SpawnRule {
            CoinType type;
            Rarity rarity;
            float probability;
            int minLevel;
            int maxLevel;
            std::vector<glm::vec3> spawnAreas;
            
            SpawnRule(CoinType coinType, Rarity coinRarity, float prob = 0.1f)
                : type(coinType), rarity(coinRarity), probability(prob),
                  minLevel(1), maxLevel(100) {}
        };
        
        struct SpawnEvent {
            glm::vec3 position;
            CoinType type;
            Rarity rarity;
            int count;
            float delay;
            bool completed;
            
            SpawnEvent(const glm::vec3& pos, CoinType coinType, int coinCount = 1)
                : position(pos), type(coinType), rarity(Rarity::COMMON),
                  count(coinCount), delay(0.0f), completed(false) {}
        };
        
        std::vector<SpawnRule> spawnRules_;
        std::vector<SpawnEvent> spawnQueue_;
        std::vector<std::unique_ptr<Coin>> activeCoins_;
        
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        class Player* targetPlayer_;
        
        float spawnTimer_;
        int maxActiveCoins_;
        bool spawningEnabled_;
        float cleanupInterval_;
        
    public:
        CoinSpawner();
        ~CoinSpawner();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player);
        void Cleanup();
        void Update(float deltaTime);
        
        void AddSpawnRule(const SpawnRule& rule);
        void RemoveSpawnRule(CoinType type, Rarity rarity);
        
        void SpawnCoin(CoinType type, const glm::vec3& position, Rarity rarity = Rarity::COMMON);
        void SpawnCoinGroup(CoinType type, int count, const glm::vec3& center, float radius, Rarity rarity = Rarity::COMMON);
        void QueueSpawnEvent(const SpawnEvent& event);
        
        void RemoveCoin(const std::string& coinId);
        void RemoveAllCoins();
        void CleanupCollectedCoins();
        
        const std::vector<std::unique_ptr<Coin>>& GetActiveCoins() const { return activeCoins_; }
        int GetActiveCoinCount() const { return static_cast<int>(activeCoins_.size()); }
        int GetTotalCoinsSpawned() const;
        
        // تنظیمات
        void SetSpawningEnabled(bool enabled) { spawningEnabled_ = enabled; }
        void SetMaxActiveCoins(int max) { maxActiveCoins_ = max; }
        
    private:
        void UpdateSpawning(float deltaTime);
        void ProcessSpawnQueue(float deltaTime);
        void CleanupExpiredCoins();
        CoinType GetRandomCoinType();
        Rarity GetRandomRarityForType(CoinType type);
        glm::vec3 GetRandomSpawnPosition();
        void ApplySpawnRules(Coin* coin);
    };

    // سیستم مدیریت سکه‌ها
    class CoinManager {
    private:
        static CoinManager* instance_;
        
        std::unique_ptr<CoinSpawner> spawner_;
        std::vector<std::unique_ptr<Coin>> specialCoins_;
        std::vector<std::unique_ptr<Coin>> bossDrops_;
        
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        class Player* targetPlayer_;
        
        int totalCoinsCollected_;
        int totalValueCollected_;
        float totalExperienceGained_;
        std::unordered_map<CoinType, int> coinsByType_;
        std::unordered_map<Rarity, int> coinsByRarity_;
        
        // قدرت‌های فعال
        float magnetPower_;
        float collectionRange_;
        float valueMultiplier_;
        float experienceMultiplier_;
        bool autoCollection_;
        
    public:
        CoinManager();
        ~CoinManager();
        
        static CoinManager& GetInstance();
        static void DestroyInstance();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player);
        void Cleanup();
        void Update(float deltaTime);
        void Render();
        
        // مدیریت سکه‌ها
        void SpawnBossDrop(const glm::vec3& position, Rarity rarity = Rarity::EPIC);
        void SpawnTreasureChest(const glm::vec3& position, Rarity rarity = Rarity::LEGENDARY);
        void SpawnSecretCoin(const glm::vec3& position);
        void SpawnCoinRain(int count, const glm::vec3& center, float radius);
        
        // جمع‌آوری
        void CollectAllInRange(const glm::vec3& center, float radius);
        void MagnetizeAllCoins();
        void MultiplyAllValues(float multiplier);
        
        // اطلاعات
        int GetTotalCoinsCollected() const { return totalCoinsCollected_; }
        int GetTotalValueCollected() const { return totalValueCollected_; }
        float GetTotalExperienceGained() const { return totalExperienceGained_; }
        int GetCoinCountByType(CoinType type) const;
        int GetCoinCountByRarity(Rarity rarity) const;
        int GetActiveCoinCount() const;
        
        // قدرت‌ها
        void SetMagnetPower(float power) { magnetPower_ = power; }
        void SetCollectionRange(float range) { collectionRange_ = range; }
        void SetValueMultiplier(float multiplier) { valueMultiplier_ = multiplier; }
        void SetExperienceMultiplier(float multiplier) { experienceMultiplier_ = multiplier; }
        void SetAutoCollection(bool enabled) { autoCollection_ = enabled; }
        
        float GetMagnetPower() const { return magnetPower_; }
        float GetCollectionRange() const { return collectionRange_; }
        float GetValueMultiplier() const { return valueMultiplier_; }
        float GetExperienceMultiplier() const { return experienceMultiplier_; }
        bool GetAutoCollection() const { return autoCollection_; }
        
        // آمار
        struct CoinStatsSummary {
            int totalCoins;
            int totalValue;
            float totalExperience;
            std::unordered_map<CoinType, int> typeBreakdown;
            std::unordered_map<Rarity, int> rarityBreakdown;
            float collectionEfficiency;
        };
        
        CoinStatsSummary GetStatsSummary() const;
        
    private:
        void UpdateSpecialCoins(float deltaTime);
        void UpdateBossDrops(float deltaTime);
        void CleanupAllCoins();
        void OnCoinCollected(Coin* coin);
        void ApplyGlobalEffects(Coin* coin);
        void CreateCollectionEffect(const glm::vec3& position, CoinType type, Rarity rarity);
    };

    // سیستم combo و زنجیره جمع‌آوری
    class CollectionComboSystem {
    private:
        struct Combo {
            int comboCount;
            float comboTime;
            float multiplier;
            bool isActive;
            std::vector<CoinType> collectedTypes;
            
            Combo() : comboCount(0), comboTime(0.0f), multiplier(1.0f), isActive(false) {}
        };
        
        Combo currentCombo_;
        float comboTimeout_;
        float maxComboTime_;
        int maxComboCount_;
        
        CoinManager* coinManager_;
        
    public:
        CollectionComboSystem(CoinManager* manager);
        ~CollectionComboSystem();
        
        void Update(float deltaTime);
        void OnCoinCollected(Coin* coin);
        void BreakCombo();
        void ResetCombo();
        
        int GetCurrentCombo() const { return currentCombo_.comboCount; }
        float GetComboMultiplier() const { return currentCombo_.multiplier; }
        bool IsComboActive() const { return currentCombo_.isActive; }
        float GetComboTimeRemaining() const { return currentCombo_.comboTime; }
        
        // تنظیمات
        void SetComboTimeout(float timeout) { comboTimeout_ = timeout; }
        void SetMaxComboTime(float maxTime) { maxComboTime_ = maxTime; }
        void SetMaxComboCount(int maxCount) { maxComboCount_ = maxCount; }
        
    private:
        void CalculateMultiplier();
        void ApplyComboEffects();
        void CreateComboEffect();
        std::string GetComboRank() const;
    };

    // سیستم افتادن سکه از دشمنان
    class CoinDropSystem {
    private:
        struct DropRule {
            EnemyType enemyType;
            std::vector<std::pair<CoinType, float>> dropChances;
            int minCoins;
            int maxCoins;
            float valueMultiplier;
            
            DropRule(EnemyType enemy) 
                : enemyType(enemy), minCoins(1), maxCoins(3), valueMultiplier(1.0f) {}
        };
        
        std::vector<DropRule> dropRules_;
        CoinManager* coinManager_;
        
    public:
        CoinDropSystem(CoinManager* manager);
        ~CoinDropSystem();
        
        void Initialize();
        void OnEnemyDefeated(class Enemy* enemy);
        
        void AddDropRule(const DropRule& rule);
        void RemoveDropRule(EnemyType enemyType);
        DropRule* GetDropRule(EnemyType enemyType);
        
    private:
        void LoadDefaultDropRules();
        void CalculateDrops(Enemy* enemy, const DropRule& rule);
        glm::vec3 CalculateDropPosition(Enemy* enemy);
    };

    // سیستم افتادن ویژه
    class SpecialDropSystem {
    private:
        struct SpecialDrop {
            std::string trigger;
            CoinType coinType;
            Rarity rarity;
            int count;
            std::function<bool()> condition;
            
            SpecialDrop(const std::string& trig, CoinType type, Rarity rare = Rarity::RARE)
                : trigger(trig), coinType(type), rarity(rare), count(1) {}
        };
        
        std::vector<SpecialDrop> specialDrops_;
        CoinManager* coinManager_;
        
    public:
        SpecialDropSystem(CoinManager* manager);
        ~SpecialDropSystem();
        
        void Initialize();
        void CheckForSpecialDrops(const std::string& trigger, const glm::vec3& position);
        
        void AddSpecialDrop(const SpecialDrop& drop);
        void RemoveSpecialDrop(const std::string& trigger);
        
    private:
        void LoadDefaultSpecialDrops();
        bool CheckDropCondition(const SpecialDrop& drop);
    };

} // namespace GalacticOdyssey

#endif // COIN_H
