#include "Player.h"
#include "Game/Entities/Projectile.h"
#include "Game/Effects/ParticleSystem.h"
#include <iostream>
#include <algorithm>

namespace GalacticOdyssey {

    // پیاده‌سازی Player
    Player::Player()
        : renderSystem_(nullptr), physicsEngine_(nullptr), audioManager_(nullptr), inputHandler_(nullptr),
          currentState_(PlayerState::IDLE), position_(0.0f), velocity_(0.0f), acceleration_(0.0f),
          rotation_(0.0f), targetRotation_(0.0f), physicsBody_(nullptr),
          mass_(1.0f), drag_(0.5f), angularDrag_(2.0f),
          shipModel_(nullptr), shipTexture_(nullptr), playerShader_(nullptr),
          modelScale_(1.0f), currentWeaponIndex_(0),
          fireTimer_(0.0f), dashTimer_(0.0f), invulnerabilityTimer_(0.0f),
          respawnTimer_(0.0f), boostTimer_(0.0f),
          inputDirection_(0.0f), lookDirection_(0.0f),
          isFiring_(false), isBoosting_(false), isDashing_(false),
          engineParticles_(nullptr), boostParticles_(nullptr), shieldParticles_(nullptr),
          showTrail_(true), trailTimer_(0.0f),
          engineSound_(""), shootSound_(""), boostSound_(""), damageSound_(""),
          animationTime_(0.0f), wobbleOffset_(0.0f), engineGlowColor_(0.0f, 0.5f, 1.0f)
    {
        std::cout << "👤 ایجاد بازیکن جدید" << std::endl;
        
        // ایجاد سلاح پیش‌فرض
        weapons_.emplace_back("Basic Laser");
        stats_ = PlayerStats();
    }

    Player::~Player()
    {
        Cleanup();
    }

    bool Player::Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                           AudioManager* audio, InputHandler* input)
    {
        std::cout << "🔧 در حال راه‌اندازی بازیکن..." << std::endl;
        
        renderSystem_ = renderer;
        physicsEngine_ = physics;
        audioManager_ = audio;
        inputHandler_ = input;
        
        if (!renderSystem_ || !physicsEngine_) {
            std::cerr << "❌ سیستم‌های وابسته راه‌اندازی نشده‌اند" << std::endl;
            return false;
        }
        
        // ایجاد مدل سفینه
        shipModel_ = renderSystem_->CreateModel("player_ship");
        if (!shipModel_) {
            std::cerr << "❌ خطا در ایجاد مدل سفینه" << std::endl;
            return false;
        }
        
        // ایجاد بدنه فیزیکی
        physicsBody_ = physicsEngine_->CreateBody(BodyType::DYNAMIC);
        if (!physicsBody_) {
            std::cerr << "❌ خطا در ایجاد بدنه فیزیکی" << std::endl;
            return false;
        }
        
        physicsBody_->SetMass(mass_);
        physicsBody_->shape = CollisionShape::SPHERE;
        physicsBody_->dimensions = glm::vec3(GetCollisionRadius());
        physicsBody_->material.density = 1.0f;
        physicsBody_->material.restitution = 0.3f;
        
        // ایجاد سیستم ذرات
        engineParticles_ = new ParticleSystem();
        boostParticles_ = new ParticleSystem();
        shieldParticles_ = new ParticleSystem();
        
        // بارگذاری صداها
        engineSound_ = "engine_hum";
        shootSound_ = "laser_shoot";
        boostSound_ = "boost";
        damageSound_ = "player_hit";
        
        // تنظیم موقعیت اولیه
        SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        
        std::cout << "✅ بازیکن با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void Player::Cleanup()
    {
        std::cout << "🧹 پاکسازی بازیکن..." << std::endl;
        
        // پاکسازی فیزیک
        if (physicsBody_ && physicsEngine_) {
            physicsEngine_->DestroyBody(physicsBody_);
            physicsBody_ = nullptr;
        }
        
        // پاکسازی ذرات
        if (engineParticles_) {
            delete engineParticles_;
            engineParticles_ = nullptr;
        }
        if (boostParticles_) {
            delete boostParticles_;
            boostParticles_ = nullptr;
        }
        if (shieldParticles_) {
            delete shieldParticles_;
            shieldParticles_ = nullptr;
        }
        
        // پاکسازی پرتابه‌ها
        for (auto* projectile : activeProjectiles_) {
            delete projectile;
        }
        activeProjectiles_.clear();
        
        std::cout << "✅ بازیکن پاکسازی شد" << std::endl;
    }

    void Player::Update(float deltaTime)
    {
        if (currentState_ == PlayerState::DEAD) {
            respawnTimer_ -= deltaTime;
            if (respawnTimer_ <= 0.0f) {
                Respawn();
            }
            return;
        }
        
        // به‌روزرسانی تایمرها
        stats_.playTime += deltaTime;
        fireTimer_ -= deltaTime;
        dashTimer_ -= deltaTime;
        invulnerabilityTimer_ -= deltaTime;
        boostTimer_ -= deltaTime;
        trailTimer_ -= deltaTime;
        animationTime_ += deltaTime;
        
        // به‌روزرسانی زیرسیستم‌ها
        UpdateInput(deltaTime);
        UpdatePhysics(deltaTime);
        UpdateWeapons(deltaTime);
        UpdatePowerUps(deltaTime);
        UpdateAnimations(deltaTime);
        UpdateParticles(deltaTime);
        UpdateAudio(deltaTime);
        
        // اعمال قدرت‌ها
        ApplyPowerUpEffects();
        RemoveExpiredPowerUps();
        
        // محدود کردن به مرزهای بازی
        ClampToBounds();
        
        // به‌روزرسانی وضعیت
        if (invulnerabilityTimer_ <= 0.0f && currentState_ == PlayerState::INVULNERABLE) {
            SetState(PlayerState::IDLE);
        }
        
        if (dashTimer_ <= 0.0f && currentState_ == PlayerState::DASHING) {
            SetState(PlayerState::IDLE);
        }
        
        if (boostTimer_ <= 0.0f && currentState_ == PlayerState::BOOSTING) {
            SetState(PlayerState::IDLE);
        }
    }

    void Player::Render()
    {
        if (!renderSystem_ || !shipModel_) return;
        
        // محاسبه ترانسفورم برای رندر
        Transform transform;
        transform.position = position_;
        transform.rotation = Quaternion::FromEuler(rotation_.x, rotation_.y, rotation_.z);
        transform.scale = modelScale_;
        
        // اعمال افکت‌های بصری
        if (currentState_ == PlayerState::INVULNERABLE) {
            float blink = sinf(animationTime_ * 10.0f) * 0.5f + 0.5f;
            // playerShader_->SetFloat("blinkFactor", blink);
        }
        
        if (currentState_ == PlayerState::DASHING) {
            // playerShader_->SetVector3("emissiveColor", glm::vec3(1.0f, 0.5f, 0.2f));
        }
        
        // رندر مدل
        renderSystem_->RenderModel(shipModel_, transform, playerShader_);
        
        // رندر سیستم ذرات
        if (engineParticles_) {
            engineParticles_->Render(*renderSystem_->GetMainCamera());
        }
        
        if (boostParticles_ && isBoosting_) {
            boostParticles_->Render(*renderSystem_->GetMainCamera());
        }
        
        if (shieldParticles_ && stats_.shields > 0) {
            shieldParticles_->Render(*renderSystem_->GetMainCamera());
        }
    }

    void Player::UpdateInput(float deltaTime)
    {
        if (!inputHandler_) return;
        
        // دریافت ورودی حرکت
        inputDirection_.x = inputHandler_->GetAxis("MoveHorizontal");
        inputDirection_.y = inputHandler_->GetAxis("MoveVertical");
        
        // دریافت ورودی نگاه کردن
        lookDirection_.x = inputHandler_->GetAxis("LookHorizontal");
        lookDirection_.y = inputHandler_->GetAxis("LookVertical");
        
        // دریافت اقدامات
        isFiring_ = inputHandler_->GetAction("Fire");
        bool boostPressed = inputHandler_->GetAction("Boost");
        bool dashPressed = inputHandler_->GetAction("Dash");
        
        // کنترل تقویت سرعت
        if (boostPressed && CanBoost()) {
            Boost(true);
        } else if (!boostPressed && isBoosting_) {
            Boost(false);
        }
        
        // کنترل حرکت سریع
        if (dashPressed && CanDash()) {
            Dash(glm::vec3(inputDirection_.x, inputDirection_.y, 0.0f));
        }
        
        // کنترل شلیک
        if (isFiring_ && CanFire()) {
            Fire();
        }
    }

    void Player::UpdatePhysics(float deltaTime)
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

    void Player::UpdateWeapons(float deltaTime)
    {
        Weapon& currentWeapon = weapons_[currentWeaponIndex_];
        
        // به‌روزرسانی کوoldاون سلاح
        if (currentWeapon.currentCooldown > 0.0f) {
            currentWeapon.currentCooldown -= deltaTime;
        }
        
        // به‌روزرسانی پرتابه‌های فعال
        for (auto it = activeProjectiles_.begin(); it != activeProjectiles_.end();) {
            Projectile* projectile = *it;
            projectile->Update(deltaTime);
            
            if (projectile->ShouldDestroy()) {
                delete projectile;
                it = activeProjectiles_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Player::UpdatePowerUps(float deltaTime)
    {
        for (auto& powerUp : activePowerUps_) {
            powerUp.timeRemaining -= deltaTime;
        }
    }

    void Player::UpdateAnimations(float deltaTime)
    {
        // انیمیشن لرزش موتور
        float wobbleSpeed = isBoosting_ ? 20.0f : 5.0f;
        float wobbleAmount = isBoosting_ ? 0.1f : 0.05f;
        
        wobbleOffset_.x = sinf(animationTime_ * wobbleSpeed) * wobbleAmount;
        wobbleOffset_.y = cosf(animationTime_ * wobbleSpeed * 0.7f) * wobbleAmount;
        wobbleOffset_.z = sinf(animationTime_ * wobbleSpeed * 0.3f) * wobbleAmount;
        
        // انیمیشن رنگ موتور
        if (isBoosting_) {
            engineGlowColor_ = glm::mix(engineGlowColor_, glm::vec3(1.0f, 0.5f, 0.2f), deltaTime * 5.0f);
        } else {
            engineGlowColor_ = glm::mix(engineGlowColor_, glm::vec3(0.0f, 0.5f, 1.0f), deltaTime * 2.0f);
        }
    }

    void Player::UpdateParticles(float deltaTime)
    {
        if (engineParticles_) {
            // ذرات موتور عادی
            glm::vec3 engineDirection = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 leftEnginePos = position_ + glm::vec3(-0.5f, 0.0f, -1.0f);
            glm::vec3 rightEnginePos = position_ + glm::vec3(0.5f, 0.0f, -1.0f);
            
            for (int i = 0; i < 2; i++) {
                glm::vec3 enginePos = (i == 0) ? leftEnginePos : rightEnginePos;
                
                Particle particle;
                particle.position = enginePos;
                particle.velocity = engineDirection * -5.0f + velocity_ * 0.5f;
                particle.color = glm::vec4(engineGlowColor_, 0.8f);
                particle.size = 0.5f;
                particle.life = 0.0f;
                particle.maxLife = 1.0f;
                
                engineParticles_->EmitParticle(particle);
            }
            
            engineParticles_->Update(deltaTime);
        }
        
        if (isBoosting_ && boostParticles_) {
            // ذرات تقویت کننده
            glm::vec3 boostDirection = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 boostPos = position_ + glm::vec3(0.0f, 0.0f, -1.5f);
            
            for (int i = 0; i < 5; i++) {
                Particle particle;
                particle.position = boostPos;
                particle.velocity = boostDirection * -15.0f + 
                                   glm::vec3((rand() % 100 - 50) / 50.0f,
                                            (rand() % 100 - 50) / 50.0f,
                                            0.0f) * 2.0f;
                particle.color = glm::vec4(1.0f, 0.3f, 0.1f, 1.0f);
                particle.size = 1.0f + (rand() % 100) / 100.0f;
                particle.life = 0.0f;
                particle.maxLife = 0.5f;
                
                boostParticles_->EmitParticle(particle);
            }
            
            boostParticles_->Update(deltaTime);
        }
    }

    void Player::UpdateAudio(float deltaTime)
    {
        if (!audioManager_) return;
        
        // صدای موتور
        auto engineSound = audioManager_->GetSound(engineSound_);
        if (engineSound) {
            float enginePitch = 0.5f + glm::length(velocity_) * 0.1f;
            if (isBoosting_) {
                enginePitch += 0.3f;
            }
            engineSound->SetPitch(enginePitch);
            
            if (engineSound->GetState() != AudioState::PLAYING) {
                engineSound->Play();
            }
        }
    }

    void Player::CalculateMovement(float deltaTime)
    {
        glm::vec3 targetVelocity = glm::vec3(inputDirection_.x, inputDirection_.y, 0.0f) * stats_.speed;
        
        // اعمال تقویت سرعت
        if (isBoosting_) {
            targetVelocity *= 2.0f;
        }
        
        // اعمال حرکت سریع
        if (currentState_ == PlayerState::DASHING) {
            targetVelocity *= 3.0f;
        }
        
        // محاسبه شتاب
        acceleration_ = (targetVelocity - velocity_) * 10.0f;
        
        // اعمال درگ
        acceleration_ -= velocity_ * drag_;
        
        // یکپارچه‌سازی سرعت
        velocity_ += acceleration_ * deltaTime;
        
        // اعمال حرکت
        position_ += velocity_ * deltaTime;
    }

    void Player::CalculateRotation(float deltaTime)
    {
        if (glm::length(inputDirection_) > 0.1f) {
            // چرخش به سمت جهت حرکت
            float targetAngle = atan2f(-inputDirection_.x, inputDirection_.y);
            targetRotation_.z = targetAngle;
        }
        
        if (glm::length(lookDirection_) > 0.1f) {
            // چرخش به سمت جهت نگاه
            targetRotation_.x = lookDirection_.y * 0.5f;
            targetRotation_.y = lookDirection_.x * 0.5f;
        }
        
        // اینترپولیشن نرم چرخش
        rotation_ = glm::mix(rotation_, targetRotation_, stats_.rotationSpeed * deltaTime);
    }

    void Player::Move(const glm::vec2& direction)
    {
        inputDirection_ = direction;
        
        if (glm::length(direction) > 0.1f) {
            SetState(PlayerState::MOVING);
        } else {
            SetState(PlayerState::IDLE);
        }
    }

    void Player::Boost(bool enable)
    {
        isBoosting_ = enable;
        
        if (enable && CanBoost()) {
            SetState(PlayerState::BOOSTING);
            boostTimer_ = 0.5f; // مدت زمان تقویت
            
            if (audioManager_) {
                audioManager_->PlaySound3D(boostSound_, position_, 1.0f);
            }
            
            CreateBoostParticles();
        } else {
            if (currentState_ == PlayerState::BOOSTING) {
                SetState(PlayerState::IDLE);
            }
        }
    }

    void Player::Dash(const glm::vec3& direction)
    {
        if (!CanDash()) return;
        
        SetState(PlayerState::DASHING);
        dashTimer_ = 0.2f; // مدت زمان حرکت سریع
        
        // اعمال سرعت ناگهانی
        glm::vec3 dashDirection = glm::length(direction) > 0.1f ? 
                                 glm::normalize(direction) : 
                                 glm::vec3(0.0f, 1.0f, 0.0f);
        
        velocity_ += dashDirection * 20.0f;
        
        // ایجاد افکت بصری
        CreateEngineParticles();
    }

    void Player::Fire()
    {
        if (!CanFire()) return;
        
        Weapon& currentWeapon = weapons_[currentWeaponIndex_];
        
        // ریست تایمر شلیک
        fireTimer_ = currentWeapon.fireRate;
        currentWeapon.currentCooldown = currentWeapon.cooldown;
        
        // ایجاد پرتابه
        glm::vec3 fireDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 firePosition = position_ + fireDirection * 1.5f;
        
        auto projectile = new Projectile();
        projectile->Initialize(renderSystem_, physicsEngine_);
        projectile->SetPosition(firePosition);
        projectile->SetVelocity(fireDirection * currentWeapon.projectileSpeed);
        projectile->SetDamage(currentWeapon.damage);
        
        activeProjectiles_.push_back(projectile);
        
        // پخش صدای شلیک
        if (audioManager_) {
            audioManager_->PlaySound3D(shootSound_, position_, 0.7f);
        }
        
        SetState(PlayerState::SHOOTING);
    }

    void Player::TakeDamage(int damage, const glm::vec3& source)
    {
        if (currentState_ == PlayerState::INVULNERABLE || currentState_ == PlayerState::DEAD) {
            return;
        }
        
        // محاسبه آسیب با در نظر گرفتن محافظ
        int actualDamage = damage;
        if (stats_.shields > 0) {
            int shieldDamage = std::min(stats_.shields, damage);
            stats_.shields -= shieldDamage;
            actualDamage -= shieldDamage;
        }
        
        if (actualDamage > 0) {
            stats_.health -= actualDamage;
        }
        
        // ایجاد افکت آسیب
        CreateDamageEffect(source);
        
        // پخش صدای آسیب
        if (audioManager_) {
            audioManager_->PlaySound3D(damageSound_, position_, 1.0f);
        }
        
        if (stats_.health <= 0) {
            // بازیکن مرده
            SetState(PlayerState::DEAD);
            stats_.health = 0;
            CreateExplosionEffect();
            respawnTimer_ = 3.0f; // 3 ثانیه تا تولد مجدد
        } else {
            // حالت مصونیت موقت
            SetState(PlayerState::DAMAGED);
            invulnerabilityTimer_ = 1.5f;
            SetState(PlayerState::INVULNERABLE);
        }
    }

    void Player::Heal(int amount)
    {
        stats_.health = std::min(stats_.health + amount, stats_.maxHealth);
    }

    void Player::AddPowerUp(PowerUpType type, float duration, float intensity)
    {
        // بررسی آیا قدرت از قبل فعال است
        for (auto& powerUp : activePowerUps_) {
            if (powerUp.type == type) {
                powerUp.timeRemaining = duration;
                powerUp.intensity = intensity;
                return;
            }
        }
        
        // افزودن قدرت جدید
        activePowerUps_.emplace_back(type, duration, intensity);
        UpdateStatsFromPowerUps();
    }

    void Player::ApplyPowerUpEffects()
    {
        for (auto& powerUp : activePowerUps_) {
            switch (powerUp.type) {
                case PowerUpType::SPEED_BOOST:
                    stats_.speed = 5.0f * (1.0f + powerUp.intensity * 0.5f);
                    break;
                    
                case PowerUpType::RAPID_FIRE:
                    weapons_[currentWeaponIndex_].fireRate = 0.5f / (1.0f + powerUp.intensity);
                    break;
                    
                case PowerUpType::SHIELD:
                    stats_.shields = static_cast<int>(stats_.maxShields * powerUp.intensity);
                    break;
                    
                case PowerUpType::MAGNET:
                    // جذب سکه‌ها در منطق بازی پیاده‌سازی می‌شود
                    break;
                    
                default:
                    break;
            }
        }
    }

    bool Player::CanFire() const
    {
        return fireTimer_ <= 0.0f && 
               currentWeaponIndex_ < weapons_.size() &&
               weapons_[currentWeaponIndex_].currentCooldown <= 0.0f;
    }

    bool Player::CanDash() const
    {
        return dashTimer_ <= 0.0f && currentState_ != PlayerState::DASHING;
    }

    bool Player::CanBoost() const
    {
        return boostTimer_ <= 0.0f && currentState_ != PlayerState::BOOSTING;
    }

    void Player::ClampToBounds()
    {
        // محدود کردن به مرزهای بازی (می‌تواند تنظیم شود)
        const float BOUNDS = 50.0f;
        
        position_.x = glm::clamp(position_.x, -BOUNDS, BOUNDS);
        position_.y = glm::clamp(position_.y, -BOUNDS, BOUNDS);
        position_.z = glm::clamp(position_.z, -BOUNDS, BOUNDS);
    }

    void Player::SetState(PlayerState newState)
    {
        if (currentState_ == newState) return;
        
        PlayerState oldState = currentState_;
        currentState_ = newState;
        
        // منطق تغییر وضعیت
        switch (newState) {
            case PlayerState::IDLE:
                // توقف افکت‌های ویژه
                break;
                
            case PlayerState::MOVING:
                // شروع افکت‌های حرکت
                break;
                
            case PlayerState::DASHING:
                // فعال کردن افکت‌های حرکت سریع
                break;
                
            case PlayerState::INVULNERABLE:
                // فعال کردن افکت‌های مصونیت
                CreateShieldEffect();
                break;
                
            case PlayerState::DEAD:
                // غیرفعال کردن کنترل‌ها
                velocity_ = glm::vec3(0.0f);
                acceleration_ = glm::vec3(0.0f);
                break;
        }
    }

    void Player::Respawn()
    {
        stats_.health = stats_.maxHealth;
        stats_.shields = 0;
        SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        velocity_ = glm::vec3(0.0f);
        acceleration_ = glm::vec3(0.0f);
        SetState(PlayerState::IDLE);
    }

    // پیاده‌سازی PlayerManager (Singleton)
    PlayerManager* PlayerManager::instance_ = nullptr;

    PlayerManager::PlayerManager()
        : renderSystem_(nullptr), physicsEngine_(nullptr), 
          audioManager_(nullptr), inputHandler_(nullptr) {}

    PlayerManager::~PlayerManager()
    {
        Cleanup();
    }

    PlayerManager& PlayerManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new PlayerManager();
        }
        return *instance_;
    }

    void PlayerManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool PlayerManager::Initialize(RenderSystem* renderer, PhysicsEngine* physics,
                                  AudioManager* audio, InputHandler* input)
    {
        renderSystem_ = renderer;
        physicsEngine_ = physics;
        audioManager_ = audio;
        inputHandler_ = input;
        
        // ایجاد بازیکن اصلی
        player_ = std::make_unique<Player>();
        if (!player_->Initialize(renderer, physics, audio, input)) {
            std::cerr << "❌ خطا در راه‌اندازی بازیکن" << std::endl;
            return false;
        }
        
        // ایجاد زیرسیستم‌ها
        InitializeSubsystems();
        
        std::cout << "✅ مدیر بازیکن با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void PlayerManager::InitializeSubsystems()
    {
        progression_ = std::make_unique<PlayerProgression>(player_.get());
        customization_ = std::make_unique<PlayerCustomization>(player_.get());
        achievements_ = std::make_unique<PlayerAchievements>(player_.get());
        inventory_ = std::make_unique<PlayerInventory>(player_.get());
        quests_ = std::make_unique<PlayerQuests>(player_.get());
        
        progression_->Initialize();
        customization_->Initialize();
        achievements_->Initialize();
        inventory_->Initialize();
        quests_->Initialize();
    }

    void PlayerManager::Update(float deltaTime)
    {
        if (player_) {
            player_->Update(deltaTime);
        }
        
        if (progression_) {
            progression_->Update(deltaTime);
        }
        
        if (achievements_) {
            achievements_->Update(deltaTime);
        }
        
        if (quests_) {
            quests_->Update(deltaTime);
        }
    }

    void PlayerManager::Render()
    {
        if (player_) {
            player_->Render();
        }
    }

} // namespace GalacticOdyssey
