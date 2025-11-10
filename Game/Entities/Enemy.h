#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Audio/AudioManager.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace GalacticOdyssey {

    // انواع دشمنان
    enum class EnemyType {
        VOLCANO,        // آتشفشان پایه
        COMET,          // ستاره دنباله‌دار سریع
        ASTEROID,       // سیارک سنگین
        UFO,            // یوفوی هوشمند
        SPACE_SQUID,    // اختاپوس فضایی
        DRONE,          // پهپاد کوچک
        BOSS_CORE,      // هسته باس
        BOSS_TENTACLE,  // بازوی باس
        MINER,          // دشمن معدنچی
        SNIPER,         // تیرانداز از راه دور
        SWARM,          // دشمنان دسته‌ای
        TANK,           // دشمن سنگین
        SUICIDE,        // دشمن انتحاری
        SUPPORT,        // دشمن پشتیبان
        ELITE           // دشمن نخبه
    };

    // رفتارهای دشمن
    enum class EnemyBehavior {
        PATROL,         // گشت‌زنی
        CHASE,          // تعقیب بازیکن
        ATTACK,         // حمله
        FLEE,           // فرار
        EVADE,          // دوری از خطر
        FORMATION,      // حرکت گروهی
        AMBUSH,         // کمین
        SPAWN,          // تولید واحدهای کوچک
        TELEPORT,       // انتقال مکانی
        CHARGE,         // شارژ مستقیم
        ORBIT,          // چرخش دور هدف
        STATIONARY      // ثابت
    };

    // وضعیت دشمن
    enum class EnemyState {
        SPAWNING,       // در حال ظاهر شدن
        ACTIVE,         // فعال
        ATTACKING,      // در حال حمله
        HIT,            // آسیب دیده
        STUNNED,        // گیج شده
        FLEEING,        // در حال فرار
        DYING,          // در حال مرگ
        DEAD,           // مرده
        SPECIAL         // حالت ویژه
    };

    // ساختار آمار دشمن
    struct EnemyStats {
        int health;
        int maxHealth;
        int shield;
        int maxShield;
        float speed;
        float rotationSpeed;
        float acceleration;
        int damage;
        float attackRange;
        float attackRate;
        float detectionRange;
        int experienceValue;
        int coinValue;
        float size;
        
        EnemyStats() 
            : health(50), maxHealth(50), shield(0), maxShield(0),
              speed(3.0f), rotationSpeed(90.0f), acceleration(5.0f),
              damage(10), attackRange(10.0f), attackRate(1.0f),
              detectionRange(20.0f), experienceValue(10), coinValue(5),
              size(1.0f) {}
    };

    // ساختار الگوی حمله
    struct AttackPattern {
        std::string name;
        float windupTime;       // زمان آماده‌سازی
        float activeTime;       // زمان فعال
        float cooldown;         // زمان استراحت
        std::function<void(class Enemy*)> execute;
        std::function<bool(class Enemy*)> canExecute;
        
        AttackPattern(const std::string& n, float windup = 0.5f, float active = 1.0f, float cool = 2.0f)
            : name(n), windupTime(windup), activeTime(active), cooldown(cool) {}
    };

    // ساختار مسیر حرکت
    struct MovementPath {
        std::vector<glm::vec3> points;
        bool loop;
        float speed;
        int currentPoint;
        
        MovementPath() : loop(false), speed(1.0f), currentPoint(0) {}
        
        glm::vec3 GetCurrentTarget() const {
            return points.empty() ? glm::vec3(0.0f) : points[currentPoint];
        }
        
        bool Advance() {
            if (points.empty()) return false;
            
            currentPoint++;
            if (currentPoint >= points.size()) {
                if (loop) {
                    currentPoint = 0;
                } else {
                    currentPoint = static_cast<int>(points.size()) - 1;
                    return false;
                }
            }
            return true;
        }
    };

    // کلاس پایه دشمن
    class Enemy {
    protected:
        // سیستم‌های وابسته
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        
        // هویت و نوع
        EnemyType type_;
        std::string id_;
        int level_;
        
        // وضعیت و موقعیت
        EnemyState currentState_;
        EnemyBehavior currentBehavior_;
        EnemyStats stats_;
        glm::vec3 position_;
        glm::vec3 velocity_;
        glm::vec3 acceleration_;
        glm::vec3 rotation_;
        glm::vec3 targetRotation_;
        
        // فیزیک
        RigidBody* physicsBody_;
        float collisionRadius_;
        
        // گرافیک
        Model3D* enemyModel_;
        Texture* enemyTexture_;
        Shader* enemyShader_;
        glm::vec3 modelScale_;
        
        // هوش مصنوعی
        class Player* targetPlayer_;
        glm::vec3 targetPosition_;
        MovementPath patrolPath_;
        std::vector<AttackPattern> attackPatterns_;
        int currentAttackIndex_;
        
        // تایمرها
        float stateTimer_;
        float attackTimer_;
        float behaviorTimer_;
        float spawnTimer_;
        float hitFlashTimer_;
        float specialAbilityTimer_;
        
        // سلامت و مقاومت
        bool isVulnerable_;
        float vulnerabilityTimer_;
        std::vector<std::string> weaknesses_;
        std::vector<std::string> resistances_;
        
        // افکت‌ها
        class ParticleSystem* deathParticles_;
        class ParticleSystem* trailParticles_;
        class ParticleSystem* hitParticles_;
        glm::vec3 baseColor_;
        glm::vec3 hitColor_;
        
        // صداها
        std::string spawnSound_;
        std::string attackSound_;
        std::string hitSound_;
        std::string deathSound_;
        
        // انیمیشن
        float animationTime_;
        glm::vec3 wobbleOffset_;
        bool isFlashing_;
        
    public:
        Enemy(EnemyType type = EnemyType::VOLCANO);
        virtual ~Enemy();
        
        virtual bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                               AudioManager* audio, class Player* player = nullptr);
        virtual void Cleanup();
        
        virtual void Update(float deltaTime);
        virtual void Render();
        
        // مدیریت وضعیت
        virtual void SetState(EnemyState newState);
        virtual void SetBehavior(EnemyBehavior newBehavior);
        virtual void TakeDamage(int damage, const glm::vec3& source = glm::vec3(0.0f));
        virtual void Heal(int amount);
        virtual void Stun(float duration);
        
        // حرکت و ناوبری
        virtual void MoveTo(const glm::vec3& target);
        virtual void FollowPlayer();
        virtual void Patrol();
        virtual void Evade(const glm::vec3& danger);
        virtual void Charge();
        
        // حمله و اقدامات
        virtual void Attack();
        virtual void UseSpecialAbility();
        virtual void SpawnMinions();
        
        // اطلاعات
        const glm::vec3& GetPosition() const { return position_; }
        const EnemyStats& GetStats() const { return stats_; }
        EnemyType GetType() const { return type_; }
        EnemyState GetState() const { return currentState_; }
        bool IsAlive() const { return currentState_ != EnemyState::DEAD; }
        bool IsActive() const { return currentState_ == EnemyState::ACTIVE; }
        float GetCollisionRadius() const { return collisionRadius_; }
        const std::string& GetId() const { return id_; }
        
        // تنظیمات
        virtual void SetPosition(const glm::vec3& position);
        virtual void SetTargetPlayer(class Player* player) { targetPlayer_ = player; }
        virtual void SetPatrolPath(const MovementPath& path) { patrolPath_ = path; }
        virtual void SetStats(const EnemyStats& newStats) { stats_ = newStats; }
        
    protected:
        // به‌روزرسانی زیرسیستم‌ها
        virtual void UpdateAI(float deltaTime);
        virtual void UpdatePhysics(float deltaTime);
        virtual void UpdateAttacks(float deltaTime);
        virtual void UpdateAnimations(float deltaTime);
        virtual void UpdateParticles(float deltaTime);
        virtual void UpdateAudio(float deltaTime);
        
        // محاسبات هوش مصنوعی
        virtual void CalculateMovement(float deltaTime);
        virtual void CalculateRotation(float deltaTime);
        virtual void DecideNextAction();
        virtual bool CanSeePlayer() const;
        virtual bool IsPlayerInRange() const;
        
        // ایجاد افکت‌ها
        virtual void CreateSpawnEffect();
        virtual void CreateDeathEffect();
        virtual void CreateHitEffect(const glm::vec3& source);
        virtual void CreateAttackEffect();
        virtual void CreateTrailEffect();
        
        // کمک‌کننده‌ها
        virtual void ClampToBounds();
        virtual void FaceTarget(const glm::vec3& target);
        virtual float DistanceToPlayer() const;
        virtual glm::vec3 DirectionToPlayer() const;
        
        // رفتارهای خاص
        virtual void UpdateVolcanoBehavior(float deltaTime);
        virtual void UpdateUFOBehavior(float deltaTime);
        virtual void UpdateAsteroidBehavior(float deltaTime);
        virtual void UpdateBossBehavior(float deltaTime);
    };

    // کلاس دشمن آتشفشان
    class VolcanoEnemy : public Enemy {
    private:
        float eruptionTimer_;
        float lavaProjectileSpeed_;
        int lavaProjectileCount_;
        
    public:
        VolcanoEnemy();
        ~VolcanoEnemy();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void Update(float deltaTime) override;
        
        void Erupt();
        void SpawnLavaProjectiles();
        
    protected:
        void UpdateVolcanoBehavior(float deltaTime) override;
        void CreateEruptionEffect();
    };

    // کلاس دشمن یوفو
    class UFOEnemy : public Enemy {
    private:
        float beamTimer_;
        float teleportTimer_;
        bool isChargingBeam_;
        glm::vec3 beamTarget_;
        
    public:
        UFOEnemy();
        ~UFOEnemy();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void Update(float deltaTime) override;
        
        void ChargeBeam();
        void FireBeam();
        void Teleport();
        void SpawnDrones();
        
    protected:
        void UpdateUFOBehavior(float deltaTime) override;
        void CreateBeamEffect();
        void CreateTeleportEffect();
    };

    // کلاس دشمن سیارک
    class AsteroidEnemy : public Enemy {
    private:
        float breakThreshold_;
        std::vector<glm::vec3> fragmentDirections_;
        bool canSplit_;
        
    public:
        AsteroidEnemy();
        ~AsteroidEnemy();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void Update(float deltaTime) override;
        void TakeDamage(int damage, const glm::vec3& source = glm::vec3(0.0f)) override;
        
        void Split();
        void Spin();
        
    protected:
        void UpdateAsteroidBehavior(float deltaTime) override;
        void CreateBreakEffect();
    };

    // کلاس باس نهایی
    class BossEnemy : public Enemy {
    private:
        struct BossPhase {
            int healthThreshold;
            EnemyBehavior behavior;
            std::vector<AttackPattern> attacks;
            float movementSpeed;
            std::string phaseName;
        };
        
        std::vector<BossPhase> phases_;
        int currentPhase_;
        std::vector<class Enemy*> minions_;
        std::vector<glm::vec3> weakPoints_;
        std::vector<bool> weakPointActive_;
        
        float enrageTimer_;
        bool isEnraged_;
        float arenaRadius_;
        
    public:
        BossEnemy();
        ~BossEnemy();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player = nullptr) override;
        void Update(float deltaTime) override;
        void TakeDamage(int damage, const glm::vec3& source = glm::vec3(0.0f)) override;
        void Render() override;
        
        void StartBattle();
        void TransitionPhase();
        void ActivateWeakPoint(int index);
        void Enrage();
        void SummonMinions();
        void PerformUltimateAttack();
        
        int GetCurrentPhase() const { return currentPhase_; }
        bool IsEnraged() const { return isEnraged_; }
        
    protected:
        void UpdateBossBehavior(float deltaTime) override;
        void InitializePhases();
        void UpdateWeakPoints();
        void CreatePhaseTransitionEffect();
        void CreateEnrageEffect();
    };

    // سیستم اسپان دشمنان
    class EnemySpawner {
    private:
        struct SpawnPoint {
            glm::vec3 position;
            float radius;
            bool active;
            float cooldown;
            std::vector<EnemyType> allowedTypes;
            
            SpawnPoint(const glm::vec3& pos, float rad = 5.0f)
                : position(pos), radius(rad), active(true), cooldown(0.0f) {}
        };
        
        struct Wave {
            int waveNumber;
            std::vector<std::pair<EnemyType, int>> enemyComposition;
            float spawnInterval;
            float waveDuration;
            bool completed;
            
            Wave(int number) 
                : waveNumber(number), spawnInterval(2.0f), waveDuration(60.0f), completed(false) {}
        };
        
        std::vector<SpawnPoint> spawnPoints_;
        std::vector<Wave> waves_;
        std::vector<std::unique_ptr<Enemy>> activeEnemies_;
        int currentWave_;
        float waveTimer_;
        float spawnTimer_;
        
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        class Player* targetPlayer_;
        
        int maxActiveEnemies_;
        bool spawningEnabled_;
        
    public:
        EnemySpawner();
        ~EnemySpawner();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player);
        void Cleanup();
        void Update(float deltaTime);
        
        void AddSpawnPoint(const glm::vec3& position, float radius = 5.0f);
        void AddWave(const Wave& wave);
        void StartNextWave();
        void StopSpawning();
        
        void SpawnEnemy(EnemyType type, const glm::vec3& position);
        void SpawnEnemyGroup(EnemyType type, int count, const glm::vec3& center, float radius);
        
        void RemoveEnemy(const std::string& enemyId);
        void RemoveAllEnemies();
        
        const std::vector<std::unique_ptr<Enemy>>& GetActiveEnemies() const { return activeEnemies_; }
        int GetActiveEnemyCount() const { return static_cast<int>(activeEnemies_.size()); }
        int GetCurrentWave() const { return currentWave_; }
        int GetTotalWaves() const { return static_cast<int>(waves_.size()); }
        bool IsWaveComplete() const;
        
    private:
        void UpdateSpawning(float deltaTime);
        void UpdateWaves(float deltaTime);
        void CleanupDeadEnemies();
        glm::vec3 GetRandomSpawnPosition();
        EnemyType GetRandomEnemyTypeForWave();
        void CreateWaveCompletionReward();
    };

    // سیستم مدیریت دشمنان
    class EnemyManager {
    private:
        static EnemyManager* instance_;
        
        std::unique_ptr<EnemySpawner> spawner_;
        std::vector<std::unique_ptr<Enemy>> specialEnemies_;
        std::vector<std::unique_ptr<Enemy>> bossEnemies_;
        
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        class Player* targetPlayer_;
        
        int totalEnemiesKilled_;
        int totalDamageDealt_;
        float gameTime_;
        bool bossActive_;
        
    public:
        EnemyManager();
        ~EnemyManager();
        
        static EnemyManager& GetInstance();
        static void DestroyInstance();
        
        bool Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                       AudioManager* audio, class Player* player);
        void Cleanup();
        void Update(float deltaTime);
        void Render();
        
        // مدیریت دشمنان
        void SpawnBoss(const glm::vec3& position);
        void SpawnEliteEnemy(EnemyType type, const glm::vec3& position);
        void SpawnEnemySwarm(EnemyType type, int count, const glm::vec3& center);
        
        // اطلاعات
        int GetTotalEnemiesKilled() const { return totalEnemiesKilled_; }
        int GetActiveEnemyCount() const;
        int GetBossCount() const { return static_cast<int>(bossEnemies_.size()); }
        bool IsBossActive() const { return bossActive_; }
        float GetGameTime() const { return gameTime_; }
        
        // تنظیمات
        void SetSpawningEnabled(bool enabled);
        void StartWaveSystem();
        void StopWaveSystem();
        
        // آمار
        struct EnemyStatsSummary {
            int volcanoesKilled;
            int ufosKilled;
            int asteroidsKilled;
            int bossesKilled;
            float averageTimeToKill;
            int totalCoinsFromEnemies;
        };
        
        EnemyStatsSummary GetStatsSummary() const;
        
    private:
        void UpdateSpecialEnemies(float deltaTime);
        void UpdateBossEnemies(float deltaTime);
        void CleanupAllEnemies();
        void OnEnemyKilled(Enemy* enemy);
        void CreateKillEffect(const glm::vec3& position);
    };

    // سیستم رفتار گروهی دشمنان
    class SwarmBehavior {
    private:
        struct SwarmMember {
            Enemy* enemy;
            glm::vec3 separation;
            glm::vec3 alignment;
            glm::vec3 cohesion;
        };
        
        std::vector<SwarmMember> swarmMembers_;
        glm::vec3 swarmCenter_;
        glm::vec3 swarmVelocity_;
        
        float separationWeight_;
        float alignmentWeight_;
        float cohesionWeight_;
        float desiredSeparation_;
        float neighborDistance_;
        
    public:
        SwarmBehavior();
        ~SwarmBehavior();
        
        void AddMember(Enemy* enemy);
        void RemoveMember(Enemy* enemy);
        void Update(float deltaTime);
        
        void SetWeights(float separation, float alignment, float cohesion);
        void SetDistances(float separation, float neighbor);
        
        const glm::vec3& GetSwarmCenter() const { return swarmCenter_; }
        const glm::vec3& GetSwarmVelocity() const { return swarmVelocity_; }
        int GetMemberCount() const { return static_cast<int>(swarmMembers_.size()); }
        
    private:
        void CalculateSwarmCenter();
        void CalculateSwarmVelocity();
        void ApplySwarmBehavior();
        glm::vec3 CalculateSeparation(SwarmMember& member);
        glm::vec3 CalculateAlignment(SwarmMember& member);
        glm::vec3 CalculateCohesion(SwarmMember& member);
    };

    // سیستم تولید مثل دشمنان
    class EnemyFactory {
    private:
        struct EnemyTemplate {
            EnemyType type;
            EnemyStats baseStats;
            std::string modelPath;
            std::string texturePath;
            std::vector<AttackPattern> attacks;
            std::vector<std::string> abilities;
        };
        
        std::unordered_map<EnemyType, EnemyTemplate> enemyTemplates_;
        RenderSystem* renderSystem_;
        PhysicsEngine* physicsEngine_;
        AudioManager* audioManager_;
        
    public:
        EnemyFactory(RenderSystem* renderer, PhysicsEngine* physics, AudioManager* audio);
        ~EnemyFactory();
        
        void Initialize();
        std::unique_ptr<Enemy> CreateEnemy(EnemyType type, class Player* targetPlayer = nullptr);
        std::unique_ptr<Enemy> CreateEnemyWithLevel(EnemyType type, int level, class Player* targetPlayer = nullptr);
        
        void RegisterEnemyTemplate(EnemyType type, const EnemyTemplate& template);
        EnemyTemplate* GetEnemyTemplate(EnemyType type);
        
    private:
        void LoadDefaultTemplates();
        void ScaleStatsForLevel(EnemyStats& stats, int level);
        void SetupEnemyBehavior(Enemy* enemy, EnemyType type);
    };

} // namespace GalacticOdyssey

#endif // ENEMY_H
