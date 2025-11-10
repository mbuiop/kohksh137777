#include "Enemy.h"
#include "Game/Entities/Player.h"
#include "Game/Entities/Projectile.h"
#include "Game/Effects/ParticleSystem.h"
#include <iostream>
#include <algorithm>
#include <random>

namespace GalacticOdyssey {

    // پیاده‌سازی Enemy
    Enemy::Enemy(EnemyType type)
        : renderSystem_(nullptr), physicsEngine_(nullptr), audioManager_(nullptr),
          type_(type), id_(""), level_(1),
          currentState_(EnemyState::SPAWNING), currentBehavior_(EnemyBehavior::PATROL),
          position_(0.0f), velocity_(0.0f), acceleration_(0.0f),
          rotation_(0.0f), targetRotation_(0.0f),
          physicsBody_(nullptr), collisionRadius_(1.0f),
          enemyModel_(nullptr), enemyTexture_(nullptr), enemyShader_(nullptr),
          modelScale_(1.0f), targetPlayer_(nullptr),
          currentAttackIndex_(0),
          stateTimer_(0.0f), attackTimer_(0.0f), behaviorTimer_(0.0f),
          spawnTimer_(1.0f), hitFlashTimer_(0.0f), specialAbilityTimer_(0.0f),
          isVulnerable_(true), vulnerabilityTimer_(0.0f),
          deathParticles_(nullptr), trailParticles_(nullptr), hitParticles_(nullptr),
          baseColor_(1.0f, 0.3f, 0.2f), hitColor_(1.0f, 1.0f, 1.0f),
          spawnSound_("enemy_spawn"), attackSound_("enemy_attack"),
          hitSound_("enemy_hit"), deathSound_("enemy_death"),
          animationTime_(0.0f), wobbleOffset_(0.0f), isFlashing_(false)
    {
        // تولید ID منحصربه‌فرد
        static int enemyCounter = 0;
        id_ = "enemy_" + std::to_string(enemyCounter++);
        
        std::cout << "👹 ایجاد دشمن: " << id_ << " نوع: " << static_cast<int>(type_) << std::endl;
        
        // تنظیم آمار بر اساس نوع
        switch (type_) {
            case EnemyType::VOLCANO:
                stats_.health = 100;
                stats_.maxHealth = 100;
                stats_.speed = 2.0f;
                stats_.damage = 20;
                stats_.attackRange = 8.0f;
                stats_.size = 1.5f;
                baseColor_ = glm::vec3(0.8f, 0.2f, 0.1f);
                break;
                
            case EnemyType::UFO:
                stats_.health = 80;
                stats_.maxHealth = 80;
                stats_.shield = 50;
                stats_.maxShield = 50;
                stats_.speed = 4.0f;
                stats_.damage = 15;
                stats_.attackRange = 12.0f;
                baseColor_ = glm::vec3(0.2f, 0.8f, 0.2f);
                break;
                
            case EnemyType::ASTEROID:
                stats_.health = 150;
                stats_.maxHealth = 150;
                stats_.speed = 1.5f;
                stats_.damage = 30;
                stats_.attackRange = 3.0f;
                stats_.size = 2.0f;
                baseColor_ = glm::vec3(0.5f, 0.5f, 0.5f);
                break;
                
            default:
                break;
        }
        
        collisionRadius_ = stats_.size;
    }

    Enemy::~Enemy()
    {
        Cleanup();
    }

    bool Enemy::Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                          AudioManager* audio, class Player* player)
    {
        std::cout << "🔧 در حال راه‌اندازی دشمن " << id_ << "..." << std::endl;
        
        renderSystem_ = renderer;
        physicsEngine_ = physics;
        audioManager_ = audio;
        targetPlayer_ = player;
        
        if (!renderSystem_ || !physicsEngine_) {
            std::cerr << "❌ سیستم‌های وابسته راه‌اندازی نشده‌اند" << std::endl;
            return false;
        }
        
        // ایجاد مدل دشمن
        enemyModel_ = renderSystem_->CreateModel("enemy_" + std::to_string(static_cast<int>(type_)));
        if (!enemyModel_) {
            std::cerr << "❌ خطا در ایجاد مدل دشمن" << std::endl;
            return false;
        }
        
        // ایجاد بدنه فیزیکی
        physicsBody_ = physicsEngine_->CreateBody(BodyType::DYNAMIC);
        if (!physicsBody_) {
            std::cerr << "❌ خطا در ایجاد بدنه فیزیکی" << std::endl;
            return false;
        }
        
        physicsBody_->SetMass(stats_.size * 10.0f);
        physicsBody_->shape = CollisionShape::SPHERE;
        physicsBody_->dimensions = glm::vec3(collisionRadius_);
        physicsBody_->material.density = 2.0f;
        physicsBody_->material.restitution = 0.1f;
        
        // ایجاد سیستم ذرات
        deathParticles_ = new ParticleSystem();
        trailParticles_ = new ParticleSystem();
        hitParticles_ = new ParticleSystem();
        
        // ایجاد الگوهای حمله
        InitializeAttackPatterns();
        
        // تنظیم موقعیت اولیه
        SetPosition(position_);
        
        // ایجاد افکت ظاهر شدن
        CreateSpawnEffect();
        
        std::cout << "✅ دشمن " << id_ << " با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void Enemy::Cleanup()
    {
        // پاکسازی فیزیک
        if (physicsBody_ && physicsEngine_) {
            physicsEngine_->DestroyBody(physicsBody_);
            physicsBody_ = nullptr;
        }
        
        // پاکسازی ذرات
        if (deathParticles_) {
            delete deathParticles_;
            deathParticles_ = nullptr;
        }
        if (trailParticles_) {
            delete trailParticles_;
            trailParticles_ = nullptr;
        }
        if (hitParticles_) {
            delete hitParticles_;
            hitParticles_ = nullptr;
        }
        
        std::cout << "🧹 دشمن " << id_ << " پاکسازی شد" << std::endl;
    }

    void Enemy::Update(float deltaTime)
    {
        if (currentState_ == EnemyState::DEAD) return;
        
        // به‌روزرسانی تایمرها
        stateTimer_ -= deltaTime;
        attackTimer_ -= deltaTime;
        behaviorTimer_ -= deltaTime;
        spawnTimer_ -= deltaTime;
        hitFlashTimer_ -= deltaTime;
        specialAbilityTimer_ -= deltaTime;
        vulnerabilityTimer_ -= deltaTime;
        animationTime_ += deltaTime;
        
        // وضعیت ظاهر شدن
        if (currentState_ == EnemyState::SPAWNING) {
            if (spawnTimer_ <= 0.0f) {
                SetState(EnemyState::ACTIVE);
            }
            return;
        }
        
        // به‌روزرسانی زیرسیستم‌ها
        UpdateAI(deltaTime);
        UpdatePhysics(deltaTime);
        UpdateAttacks(deltaTime);
        UpdateAnimations(deltaTime);
        UpdateParticles(deltaTime);
        UpdateAudio(deltaTime);
        
        // محدود کردن به مرزهای بازی
        ClampToBounds();
        
        // به‌روزرسانی وضعیت
        if (stateTimer_ <= 0.0f) {
            switch (currentState_) {
                case EnemyState::HIT:
                case EnemyState::STUNNED:
                    SetState(EnemyState::ACTIVE);
                    break;
                case EnemyState::DYING:
                    SetState(EnemyState::DEAD);
                    CreateDeathEffect();
                    break;
                default:
                    break;
            }
        }
        
        // به‌روزرسانی آسیب‌پذیری
        if (vulnerabilityTimer_ <= 0.0f && !isVulnerable_) {
            isVulnerable_ = true;
        }
    }

    void Enemy::Render()
    {
        if (!renderSystem_ || !enemyModel_ || currentState_ == EnemyState::DEAD) return;
        
        // محاسبه ترانسفورم برای رندر
        Transform transform;
        transform.position = position_ + wobbleOffset_;
        transform.rotation = Quaternion::FromEuler(rotation_.x, rotation_.y, rotation_.z);
        transform.scale = modelScale_ * stats_.size;
        
        // اعمال افکت‌های بصری
        if (isFlashing_ && hitFlashTimer_ > 0.0f) {
            float flashIntensity = (sinf(hitFlashTimer_ * 30.0f) + 1.0f) * 0.5f;
            // enemyShader_->SetVector3("emissiveColor", hitColor_ * flashIntensity);
        } else {
            // enemyShader_->SetVector3("emissiveColor", baseColor_ * 0.3f);
        }
        
        if (!isVulnerable_) {
            float blink = sinf(animationTime_ * 10.0f) * 0.3f + 0.7f;
            // enemyShader_->SetFloat("alphaMultiplier", blink);
        }
        
        // رندر مدل
        renderSystem_->RenderModel(enemyModel_, transform, enemyShader_);
        
        // رندر سیستم ذرات
        if (trailParticles_) {
            trailParticles_->Render(*renderSystem_->GetMainCamera());
        }
        
        if (hitParticles_ && isFlashing_) {
            hitParticles_->Render(*renderSystem_->GetMainCamera());
        }
    }

    void Enemy::UpdateAI(float deltaTime)
    {
        if (currentState_ != EnemyState::ACTIVE) return;
        
        // تصمیم‌گیری برای اقدام بعدی
        DecideNextAction();
        
        // اجرای رفتار فعلی
        switch (currentBehavior_) {
            case EnemyBehavior::PATROL:
                Patrol();
                break;
            case EnemyBehavior::CHASE:
                FollowPlayer();
                break;
            case EnemyBehavior::ATTACK:
                Attack();
                break;
            case EnemyBehavior::FLEE:
                Evade(targetPlayer_ ? targetPlayer_->GetPosition() : glm::vec3(0.0f));
                break;
            default:
                break;
        }
        
        // به‌روزرسانی رفتار بر اساس نوع دشمن
        switch (type_) {
            case EnemyType::VOLCANO:
                UpdateVolcanoBehavior(deltaTime);
                break;
            case EnemyType::UFO:
                UpdateUFOBehavior(deltaTime);
                break;
            case EnemyType::ASTEROID:
                UpdateAsteroidBehavior(deltaTime);
                break;
            case EnemyType::BOSS_CORE:
                UpdateBossBehavior(deltaTime);
                break;
            default:
                break;
        }
        
        behaviorTimer_ -= deltaTime;
        if (behaviorTimer_ <= 0.0f) {
            // تغییر رفتار تصادفی
            std::vector<EnemyBehavior> possibleBehaviors = {
                EnemyBehavior::PATROL, EnemyBehavior::CHASE, EnemyBehavior::ATTACK
            };
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, possibleBehaviors.size() - 1);
            
            SetBehavior(possibleBehaviors[dis(gen)]);
            behaviorTimer_ = 3.0f + (rand() % 100) / 100.0f * 5.0f;
        }
    }

    void Enemy::UpdatePhysics(float deltaTime)
    {
        if (!physicsBody_) return;
        
        // محاسبه حرکت
        CalculateMovement(deltaTime);
        CalculateRotation(deltaTime);
        
        // اعمال حرکت به فیزیک
        physicsBody_->position = position_;
        physicsBody_->velocity = velocity_;
        
        // به‌روزرسانی موقعیت از فیزیک
        position_ = physicsBody_->position;
        velocity_ = physicsBody_->velocity;
    }

    void Enemy::CalculateMovement(float deltaTime)
    {
        glm::vec3 targetVelocity = velocity_;
        
        switch (currentBehavior_) {
            case EnemyBehavior::CHASE:
                if (targetPlayer_ && DistanceToPlayer() > stats_.attackRange * 0.8f) {
                    glm::vec3 direction = DirectionToPlayer();
                    targetVelocity = direction * stats_.speed;
                }
                break;
                
            case EnemyBehavior::PATROL:
                if (!patrolPath_.points.empty()) {
                    glm::vec3 target = patrolPath_.GetCurrentTarget();
                    glm::vec3 direction = glm::normalize(target - position_);
                    targetVelocity = direction * stats_.speed * 0.5f;
                    
                    if (glm::distance(position_, target) < 1.0f) {
                        patrolPath_.Advance();
                    }
                }
                break;
                
            case EnemyBehavior::FLEE:
                if (targetPlayer_) {
                    glm::vec3 direction = position_ - targetPlayer_->GetPosition();
                    if (glm::length(direction) > 0.1f) {
                        direction = glm::normalize(direction);
                        targetVelocity = direction * stats_.speed * 1.2f;
                    }
                }
                break;
                
            default:
                targetVelocity = glm::vec3(0.0f);
                break;
        }
        
        // محاسبه شتاب
        acceleration_ = (targetVelocity - velocity_) * stats_.acceleration;
        
        // اعمال درگ
        acceleration_ -= velocity_ * 0.5f;
        
        // یکپارچه‌سازی سرعت
        velocity_ += acceleration_ * deltaTime;
        
        // اعمال حرکت
        position_ += velocity_ * deltaTime;
    }

    void Enemy::CalculateRotation(float deltaTime)
    {
        glm::vec3 targetDirection = velocity_;
        if (glm::length(targetDirection) > 0.1f) {
            targetDirection = glm::normalize(targetDirection);
            
            // محاسبه زوایای یاو و پیچ
            targetRotation_.y = atan2f(targetDirection.x, targetDirection.z);
            targetRotation_.x = -asinf(targetDirection.y);
        }
        
        if (targetPlayer_ && currentBehavior_ == EnemyBehavior::ATTACK) {
            // نگاه کردن به بازیکن هنگام حمله
            FaceTarget(targetPlayer_->GetPosition());
        }
        
        // اینترپولیشن نرم چرخش
        rotation_ = glm::mix(rotation_, targetRotation_, stats_.rotationSpeed * deltaTime);
    }

    void Enemy::DecideNextAction()
    {
        if (!targetPlayer_) return;
        
        float distance = DistanceToPlayer();
        
        if (distance <= stats_.attackRange && attackTimer_ <= 0.0f) {
            SetBehavior(EnemyBehavior::ATTACK);
        } else if (distance <= stats_.detectionRange) {
            SetBehavior(EnemyBehavior::CHASE);
        } else {
            SetBehavior(EnemyBehavior::PATROL);
        }
        
        // فرار در صورت سلامت کم
        if (stats_.health < stats_.maxHealth * 0.3f) {
            SetBehavior(EnemyBehavior::FLEE);
        }
    }

    void Enemy::TakeDamage(int damage, const glm::vec3& source)
    {
        if (!isVulnerable_ || currentState_ == EnemyState::DEAD) return;
        
        // محاسبه آسیب با در نظر گرفتن مقاومت‌ها
        int actualDamage = damage;
        
        // کسر از محافظ اول
        if (stats_.shield > 0) {
            int shieldDamage = std::min(stats_.shield, actualDamage);
            stats_.shield -= shieldDamage;
            actualDamage -= shieldDamage;
        }
        
        // اعمال آسیب به سلامت
        if (actualDamage > 0) {
            stats_.health -= actualDamage;
        }
        
        // ایجاد افکت آسیب
        CreateHitEffect(source);
        
        // پخش صدای آسیب
        if (audioManager_) {
            audioManager_->PlaySound3D(hitSound_, position_, 0.8f);
        }
        
        // فلش بصری
        isFlashing_ = true;
        hitFlashTimer_ = 0.3f;
        
        if (stats_.health <= 0) {
            // دشمن مرده
            SetState(EnemyState::DYING);
            stats_.health = 0;
            stateTimer_ = 0.5f; // زمان قبل از ناپدید شدن
        } else {
            // حالت آسیب دیده
            SetState(EnemyState::HIT);
            stateTimer_ = 0.2f;
            
            // مصونیت موقت
            isVulnerable_ = false;
            vulnerabilityTimer_ = 0.5f;
        }
    }

    void Enemy::Attack()
    {
        if (attackTimer_ > 0.0f || currentState_ != EnemyState::ACTIVE) return;
        
        if (!attackPatterns_.empty()) {
            AttackPattern& pattern = attackPatterns_[currentAttackIndex_];
            
            if (pattern.canExecute && pattern.canExecute(this)) {
                pattern.execute(this);
                
                // پخش صدای حمله
                if (audioManager_) {
                    audioManager_->PlaySound3D(attackSound_, position_, 0.7f);
                }
                
                attackTimer_ = pattern.cooldown;
                currentAttackIndex_ = (currentAttackIndex_ + 1) % attackPatterns_.size();
                
                SetState(EnemyState::ATTACKING);
                stateTimer_ = pattern.windupTime + pattern.activeTime;
            }
        }
    }

    void Enemy::SetState(EnemyState newState)
    {
        if (currentState_ == newState) return;
        
        EnemyState oldState = currentState_;
        currentState_ = newState;
        stateTimer_ = 0.0f;
        
        // منطق تغییر وضعیت
        switch (newState) {
            case EnemyState::ACTIVE:
                // فعال کردن AI
                break;
                
            case EnemyState::ATTACKING:
                // آماده‌سازی برای حمله
                break;
                
            case EnemyState::DYING:
                // غیرفعال کردن فیزیک و AI
                if (physicsBody_) {
                    physicsBody_->type = BodyType::STATIC;
                }
                velocity_ = glm::vec3(0.0f);
                acceleration_ = glm::vec3(0.0f);
                break;
                
            case EnemyState::DEAD:
                // پاکسازی منابع
                break;
        }
    }

    void Enemy::SetBehavior(EnemyBehavior newBehavior)
    {
        if (currentBehavior_ == newBehavior) return;
        
        currentBehavior_ = newBehavior;
        behaviorTimer_ = 5.0f; // مدت زمان رفتار
        
        // منطق تغییر رفتار
        switch (newBehavior) {
            case EnemyBehavior::CHASE:
                // افزایش سرعت برای تعقیب
                stats_.speed *= 1.2f;
                break;
                
            case EnemyBehavior::FLEE:
                // افزایش سرعت برای فرار
                stats_.speed *= 1.5f;
                break;
                
            default:
                // بازگشت به سرعت عادی
                switch (type_) {
                    case EnemyType::VOLCANO:
                        stats_.speed = 2.0f;
                        break;
                    case EnemyType::UFO:
                        stats_.speed = 4.0f;
                        break;
                    case EnemyType::ASTEROID:
                        stats_.speed = 1.5f;
                        break;
                    default:
                        stats_.speed = 3.0f;
                        break;
                }
                break;
        }
    }

    bool Enemy::CanSeePlayer() const
    {
        if (!targetPlayer_) return false;
        
        // بررسی دید مستقیم (ساده‌سازی)
        float distance = DistanceToPlayer();
        return distance <= stats_.detectionRange;
    }

    bool Enemy::IsPlayerInRange() const
    {
        if (!targetPlayer_) return false;
        return DistanceToPlayer() <= stats_.attackRange;
    }

    float Enemy::DistanceToPlayer() const
    {
        if (!targetPlayer_) return FLT_MAX;
        return glm::distance(position_, targetPlayer_->GetPosition());
    }

    glm::vec3 Enemy::DirectionToPlayer() const
    {
        if (!targetPlayer_) return glm::vec3(0.0f, 0.0f, -1.0f);
        return glm::normalize(targetPlayer_->GetPosition() - position_);
    }

    void Enemy::FaceTarget(const glm::vec3& target)
    {
        glm::vec3 direction = glm::normalize(target - position_);
        targetRotation_.y = atan2f(direction.x, direction.z);
        targetRotation_.x = -asinf(direction.y);
    }

    void Enemy::ClampToBounds()
    {
        // محدود کردن به مرزهای بازی
        const float BOUNDS = 100.0f;
        
        position_.x = glm::clamp(position_.x, -BOUNDS, BOUNDS);
        position_.y = glm::clamp(position_.y, -BOUNDS, BOUNDS);
        position_.z = glm::clamp(position_.z, -BOUNDS, BOUNDS);
    }

    void Enemy::InitializeAttackPatterns()
    {
        // الگوی حمله پایه
        AttackPattern basicAttack("Basic Attack", 0.3f, 0.5f, 2.0f);
        basicAttack.canExecute = [this](Enemy* enemy) {
            return this->IsPlayerInRange();
        };
        basicAttack.execute = [this](Enemy* enemy) {
            // ایجاد پرتابه یا آسیب مستقیم
            if (targetPlayer_) {
                // targetPlayer_->TakeDamage(stats_.damage, position_);
            }
        };
        
        attackPatterns_.push_back(basicAttack);
    }

    void Enemy::UpdateVolcanoBehavior(float deltaTime)
    {
        // رفتار ویژه آتشفشان
        specialAbilityTimer_ -= deltaTime;
        
        if (specialAbilityTimer_ <= 0.0f && IsPlayerInRange()) {
            // فوران دوره‌ای
            // dynamic_cast<VolcanoEnemy*>(this)->Erupt();
            specialAbilityTimer_ = 5.0f;
        }
    }

    void Enemy::UpdateUFOBehavior(float deltaTime)
    {
        // رفتار ویژه یوفو
        specialAbilityTimer_ -= deltaTime;
        
        if (specialAbilityTimer_ <= 0.0f && DistanceToPlayer() < 15.0f) {
            // تله‌پورت دوره‌ای
            // dynamic_cast<UFOEnemy*>(this)->Teleport();
            specialAbilityTimer_ = 8.0f;
        }
    }

    // پیاده‌سازی EnemyManager (Singleton)
    EnemyManager* EnemyManager::instance_ = nullptr;

    EnemyManager::EnemyManager()
        : renderSystem_(nullptr), physicsEngine_(nullptr), audioManager_(nullptr),
          targetPlayer_(nullptr), totalEnemiesKilled_(0), totalDamageDealt_(0),
          gameTime_(0.0f), bossActive_(false)
    {
        std::cout << "👹 ایجاد مدیر دشمنان" << std::endl;
    }

    EnemyManager::~EnemyManager()
    {
        Cleanup();
    }

    EnemyManager& EnemyManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new EnemyManager();
        }
        return *instance_;
    }

    void EnemyManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool EnemyManager::Initialize(RenderSystem* renderer, PhysicsEngine* physics,
                                 AudioManager* audio, class Player* player)
    {
        renderSystem_ = renderer;
        physicsEngine_ = physics;
        audioManager_ = audio;
        targetPlayer_ = player;
        
        // ایجاد اسپانر
        spawner_ = std::make_unique<EnemySpawner>();
        if (!spawner_->Initialize(renderer, physics, audio, player)) {
            std::cerr << "❌ خطا در راه‌اندازی اسپانر دشمنان" << std::endl;
            return false;
        }
        
        std::cout << "✅ مدیر دشمنان با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void EnemyManager::Update(float deltaTime)
    {
        gameTime_ += deltaTime;
        
        if (spawner_) {
            spawner_->Update(deltaTime);
        }
        
        UpdateSpecialEnemies(deltaTime);
        UpdateBossEnemies(deltaTime);
    }

    void EnemyManager::Render()
    {
        if (spawner_) {
            // اسپانر خودش دشمنان را رندر می‌کند
        }
        
        // رندر دشمنان ویژه
        for (auto& enemy : specialEnemies_) {
            if (enemy->IsAlive()) {
                enemy->Render();
            }
        }
        
        // رندر باس‌ها
        for (auto& boss : bossEnemies_) {
            if (boss->IsAlive()) {
                boss->Render();
            }
        }
    }

} // namespace GalacticOdyssey
