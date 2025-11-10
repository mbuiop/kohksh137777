#include "Coin.h"
#include "Game/Entities/Player.h"
#include "Game/Effects/ParticleSystem.h"
#include <iostream>
#include <algorithm>
#include <random>

namespace GalacticOdyssey {

    // پیاده‌سازی Coin
    Coin::Coin(CoinType type, Rarity rarity)
        : renderSystem_(nullptr), physicsEngine_(nullptr), audioManager_(nullptr),
          type_(type), rarity_(rarity), id_(""), level_(1),
          currentState_(CoinState::SPAWNING), position_(0.0f), velocity_(0.0f),
          rotation_(0.0f), targetPosition_(0.0f),
          physicsBody_(nullptr), collisionRadius_(0.5f),
          coinModel_(nullptr), coinTexture_(nullptr), coinShader_(nullptr),
          modelScale_(1.0f), targetPlayer_(nullptr),
          stateTimer_(0.0f), lifetimeTimer_(0.0f), collectionTimer_(0.0f),
          floatTimer_(0.0f), glowTimer_(0.0f),
          isMagnetized_(false), magnetStrength_(1.0f),
          collectParticles_(nullptr), glowParticles_(nullptr), trailParticles_(nullptr),
          baseColor_(1.0f, 1.0f, 1.0f), rareColor_(1.0f, 1.0f, 1.0f),
          spawnSound_("coin_spawn"), collectSound_("coin_collect"), magnetSound_("coin_magnet"),
          animationTime_(0.0f), floatOffset_(0.0f), isPulsing_(false), pulsePhase_(0.0f)
    {
        // تولید ID منحصربه‌فرد
        static int coinCounter = 0;
        id_ = "coin_" + std::to_string(coinCounter++);
        
        std::cout << "💰 ایجاد سکه: " << id_ << " نوع: " << static_cast<int>(type_) 
                  << " نادرتی: " << static_cast<int>(rarity_) << std::endl;
        
        // تنظیم آمار بر اساس نوع و نادرتی
        SetupFromTypeAndRarity();
        ApplyRarityMultipliers();
        
        lifetimeTimer_ = stats_.lifetime;
    }

    Coin::~Coin()
    {
        Cleanup();
    }

    bool Coin::Initialize(RenderSystem* renderer, PhysicsEngine* physics, 
                         AudioManager* audio, class Player* player)
    {
        std::cout << "🔧 در حال راه‌اندازی سکه " << id_ << "..." << std::endl;
        
        renderSystem_ = renderer;
        physicsEngine_ = physics;
        audioManager_ = audio;
        targetPlayer_ = player;
        
        if (!renderSystem_ || !physicsEngine_) {
            std::cerr << "❌ سیستم‌های وابسته راه‌اندازی نشده‌اند" << std::endl;
            return false;
        }
        
        // ایجاد مدل سکه
        coinModel_ = renderSystem_->CreateModel("coin_" + std::to_string(static_cast<int>(type_)));
        if (!coinModel_) {
            std::cerr << "❌ خطا در ایجاد مدل سکه" << std::endl;
            return false;
        }
        
        // ایجاد بدنه فیزیکی
        physicsBody_ = physicsEngine_->CreateBody(BodyType::DYNAMIC);
        if (!physicsBody_) {
            std::cerr << "❌ خطا در ایجاد بدنه فیزیکی" << std::endl;
            return false;
        }
        
        physicsBody_->SetMass(0.1f);
        physicsBody_->shape = CollisionShape::SPHERE;
        physicsBody_->dimensions = glm::vec3(collisionRadius_);
        physicsBody_->material.density = 0.5f;
        physicsBody_->material.restitution = 0.7f;
        
        // ایجاد سیستم ذرات
        collectParticles_ = new ParticleSystem();
        glowParticles_ = new ParticleSystem();
        trailParticles_ = new ParticleSystem();
        
        // تنظیم موقعیت اولیه
        SetPosition(position_);
        
        // ایجاد افکت ظاهر شدن
        CreateSpawnEffect();
        
        // پخش صدای ظاهر شدن
        if (audioManager_) {
            audioManager_->PlaySound3D(spawnSound_, position_, 0.5f);
        }
        
        std::cout << "✅ سکه " << id_ << " با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void Coin::Cleanup()
    {
        // پاکسازی فیزیک
        if (physicsBody_ && physicsEngine_) {
            physicsEngine_->DestroyBody(physicsBody_);
            physicsBody_ = nullptr;
        }
        
        // پاکسازی ذرات
        if (collectParticles_) {
            delete collectParticles_;
            collectParticles_ = nullptr;
        }
        if (glowParticles_) {
            delete glowParticles_;
            glowParticles_ = nullptr;
        }
        if (trailParticles_) {
            delete trailParticles_;
            trailParticles_ = nullptr;
        }
        
        std::cout << "🧹 سکه " << id_ << " پاکسازی شد" << std::endl;
    }

    void Coin::Update(float deltaTime)
    {
        if (currentState_ == CoinState::COLLECTED || currentState_ == CoinState::DISAPPEARING) {
            return;
        }
        
        // به‌روزرسانی تایمرها
        stateTimer_ -= deltaTime;
        lifetimeTimer_ -= deltaTime;
        collectionTimer_ -= deltaTime;
        floatTimer_ += deltaTime;
        glowTimer_ += deltaTime;
        animationTime_ += deltaTime;
        
        // بررسی انقضای زمان عمر
        if (lifetimeTimer_ <= 0.0f && currentState_ != CoinState::SPAWNING) {
            SetState(CoinState::DISAPPEARING);
            stateTimer_ = 1.0f;
            return;
        }
        
        // وضعیت ظاهر شدن
        if (currentState_ == CoinState::SPAWNING) {
            if (stateTimer_ <= 0.0f) {
                SetState(CoinState::IDLE);
            }
            return;
        }
        
        // وضعیت جمع‌آوری
        if (currentState_ == CoinState::COLLECTING) {
            if (collectionTimer_ <= 0.0f) {
                CompleteCollection();
            }
            return;
        }
        
        // به‌روزرسانی زیرسیستم‌ها
        UpdatePhysics(deltaTime);
        UpdateAnimation(deltaTime);
        UpdateMagnet(deltaTime);
        UpdateParticles(deltaTime);
        UpdateAudio(deltaTime);
        
        // بررسی جذب
        if (ShouldMagnetize() && !isMagnetized_) {
            Magnetize(targetPlayer_);
        }
        
        // محدود کردن به مرزهای بازی
        ClampToBounds();
        
        // به‌روزرسانی وضعیت
        if (stateTimer_ <= 0.0f) {
            switch (currentState_) {
                case CoinState::DISAPPEARING:
                    SetState(CoinState::COLLECTED);
                    break;
                default:
                    break;
            }
        }
    }

    void Coin::Render()
    {
        if (!renderSystem_ || !coinModel_ || 
            currentState_ == CoinState::COLLECTED || 
            currentState_ == CoinState::DISAPPEARING) {
            return;
        }
        
        // محاسبه ترانسفورم برای رندر
        Transform transform;
        transform.position = position_ + floatOffset_;
        transform.rotation = Quaternion::FromEuler(rotation_.x, rotation_.y, rotation_.z);
        transform.scale = modelScale_ * (1.0f + pulsePhase_ * 0.1f);
        
        // اعمال افکت‌های بصری
        if (isMagnetized_) {
            // افزایش درخشش هنگام جذب
            float magnetGlow = (sinf(animationTime_ * 10.0f) + 1.0f) * 0.5f;
            // coinShader_->SetVector3("emissiveColor", baseColor_ * (1.0f + magnetGlow));
        } else {
            // coinShader_->SetVector3("emissiveColor", baseColor_ * stats_.glowIntensity);
        }
        
        if (currentState_ == CoinState::DISAPPEARING) {
            // کاهش آلفا هنگام ناپدید شدن
            float alpha = stateTimer_; // از 1 به 0 می‌رود
            // coinShader_->SetFloat("alphaMultiplier", alpha);
        }
        
        // رندر مدل
        renderSystem_->RenderModel(coinModel_, transform, coinShader_);
        
        // رندر سیستم ذرات
        if (glowParticles_) {
            glowParticles_->Render(*renderSystem_->GetMainCamera());
        }
        
        if (trailParticles_ && isMagnetized_) {
            trailParticles_->Render(*renderSystem_->GetMainCamera());
        }
    }

    void Coin::UpdatePhysics(float deltaTime)
    {
        if (!physicsBody_ || currentState_ == CoinState::MAGNETIZED) return;
        
        // محاسبه حرکت شناوری
        CalculateFloatMotion(deltaTime);
        CalculateRotation(deltaTime);
        
        // اعمال حرکت به فیزیک
        physicsBody_->position = position_ + floatOffset_;
        physicsBody_->velocity = velocity_;
        
        // به‌روزرسانی موقعیت از فیزیک
        position_ = physicsBody_->position - floatOffset_;
        velocity_ = physicsBody_->velocity;
    }

    void Coin::UpdateAnimation(float deltaTime)
    {
        // محاسبه حرکت شناوری
        CalculateFloatMotion(deltaTime);
        CalculateRotation(deltaTime);
        CalculateGlowEffect(deltaTime);
    }

    void Coin::UpdateMagnet(float deltaTime)
    {
        if (!isMagnetized_ || !targetPlayer_) return;
        
        CalculateMagnetForce(deltaTime);
        
        // حرکت به سمت بازیکن
        glm::vec3 direction = targetPlayer_->GetPosition() - position_;
        float distance = glm::length(direction);
        
        if (distance > 0.1f) {
            direction = glm::normalize(direction);
            
            // سرعت بر اساس فاصله (سریع‌تر در فواصل نزدیک)
            float speed = magnetStrength_ * (1.0f + 5.0f / (distance + 0.1f));
            velocity_ = direction * speed;
            
            // اعمال حرکت
            position_ += velocity_ * deltaTime;
            
            // بررسی برخورد با بازیکن
            if (distance < targetPlayer_->GetCollisionRadius() + collisionRadius_) {
                Collect();
            }
        }
    }

    void Coin::CalculateFloatMotion(float deltaTime)
    {
        // حرکت شناوری سینوسی
        float floatOffsetY = sinf(floatTimer_ * stats_.floatFrequency) * stats_.floatAmplitude;
        floatOffset_.y = floatOffsetY;
        
        // حرکت جزئی در محور X و Z برای اثر طبیعی‌تر
        floatOffset_.x = sinf(floatTimer_ * stats_.floatFrequency * 0.7f) * stats_.floatAmplitude * 0.3f;
        floatOffset_.z = cosf(floatTimer_ * stats_.floatFrequency * 0.5f) * stats_.floatAmplitude * 0.3f;
    }

    void Coin::CalculateRotation(float deltaTime)
    {
        // چرخش مداوم
        rotation_.y += stats_.rotationSpeed * deltaTime;
        
        // چرخش جزئی در سایر محورها برای اثر طبیعی‌تر
        rotation_.x = sinf(floatTimer_ * 0.3f) * 15.0f;
        rotation_.z = cosf(floatTimer_ * 0.2f) * 10.0f;
    }

    void Coin::CalculateMagnetForce(float deltaTime)
    {
        // پالس‌های مغناطیسی
        pulsePhase_ = (sinf(animationTime_ * 8.0f) + 1.0f) * 0.5f;
        magnetStrength_ = 3.0f + pulsePhase_ * 2.0f;
    }

    void Coin::CalculateGlowEffect(float deltaTime)
    {
        // درخشش پالسی
        float glow = (sinf(glowTimer_ * 2.0f) + 1.0f) * 0.5f;
        stats_.glowIntensity = 0.5f + glow * 0.5f;
        
        // تغییر رنگ بر اساس نادرتی
        if (rarity_ >= Rarity::RARE) {
            float colorMix = (sinf(glowTimer_ * 3.0f) + 1.0f) * 0.5f;
            stats_.glowColor = glm::mix(baseColor_, rareColor_, colorMix);
        }
    }

    void Coin::Collect()
    {
        if (currentState_ != CoinState::IDLE && currentState_ != CoinState::MAGNETIZED) {
            return;
        }
        
        SetState(CoinState::COLLECTING);
        collectionTimer_ = stats_.collectionTime;
        
        // ایجاد افکت جمع‌آوری
        CreateCollectEffect();
        
        // پخش صدای جمع‌آوری
        if (audioManager_) {
            audioManager_->PlaySound3D(collectSound_, position_, 0.7f);
        }
        
        // اعمال اثرات
        if (effects_.onCollect && targetPlayer_) {
            effects_.onCollect(targetPlayer_);
        }
        
        ApplyTemporaryEffect(targetPlayer_);
    }

    void Coin::Magnetize(class Player* player)
    {
        if (currentState_ != CoinState::IDLE) return;
        
        targetPlayer_ = player;
        isMagnetized_ = true;
        SetState(CoinState::MAGNETIZED);
        
        // ایجاد افکت مغناطیسی
        CreateMagnetEffect();
        
        // پخش صدای جذب
        if (audioManager_) {
            audioManager_->PlaySound3D(magnetSound_, position_, 0.3f);
        }
    }

    void Coin::ApplyTemporaryEffect(class Player* player)
    {
        if (!player) return;
        
        switch (type_) {
            case CoinType::ENERGY_ORB:
                // player->AddEnergy(stats_.value);
                break;
                
            case CoinType::HEALTH_ORB:
                player->Heal(stats_.value);
                break;
                
            case CoinType::EXPERIENCE_ORB:
                player->AddExperience(static_cast<int>(stats_.experience));
                break;
                
            case CoinType::POWER_UP:
                // اعمال قدرت موقت
                break;
                
            default:
                // افزایش سکه‌های بازیکن
                // player->GetInventory()->AddCoins(stats_.value);
                break;
        }
    }

    void Coin::SetState(CoinState newState)
    {
        if (currentState_ == newState) return;
        
        CoinState oldState = currentState_;
        currentState_ = newState;
        stateTimer_ = 0.0f;
        
        // منطق تغییر وضعیت
        switch (newState) {
            case CoinState::IDLE:
                // فعال کردن فیزیک عادی
                if (physicsBody_) {
                    physicsBody_->type = BodyType::DYNAMIC;
                }
                break;
                
            case CoinState::MAGNETIZED:
                // غیرفعال کردن فیزیک عادی
                if (physicsBody_) {
                    physicsBody_->type = BodyType::KINEMATIC;
                }
                velocity_ = glm::vec3(0.0f);
                break;
                
            case CoinState::COLLECTING:
                // غیرفعال کردن تمام فیزیک
                if (physicsBody_) {
                    physicsBody_->type = BodyType::STATIC;
                }
                velocity_ = glm::vec3(0.0f);
                break;
                
            case CoinState::DISAPPEARING:
                // شروع افکت ناپدید شدن
                break;
                
            case CoinState::COLLECTED:
                // آماده‌سازی برای پاکسازی
                break;
        }
        
        // اجرای callback های وضعیت
        switch (newState) {
            case CoinState::SPAWNING:
                if (effects_.onSpawn) {
                    effects_.onSpawn(this);
                }
                break;
                
            case CoinState::DISAPPEARING:
                if (effects_.onDisappear) {
                    effects_.onDisappear(this);
                }
                break;
        }
    }

    void Coin::SetupFromTypeAndRarity()
    {
        // تنظیمات پایه بر اساس نوع سکه
        switch (type_) {
            case CoinType::BRONZE_COIN:
                stats_.value = 1;
                stats_.experience = 1.0f;
                baseColor_ = glm::vec3(0.8f, 0.5f, 0.2f);
                collisionRadius_ = 0.3f;
                break;
                
            case CoinType::SILVER_COIN:
                stats_.value = 5;
                stats_.experience = 3.0f;
                baseColor_ = glm::vec3(0.8f, 0.8f, 0.9f);
                collisionRadius_ = 0.4f;
                break;
                
            case CoinType::GOLD_COIN:
                stats_.value = 10;
                stats_.experience = 8.0f;
                baseColor_ = glm::vec3(1.0f, 0.8f, 0.2f);
                collisionRadius_ = 0.5f;
                break;
                
            case CoinType::PLATINUM_COIN:
                stats_.value = 25;
                stats_.experience = 20.0f;
                baseColor_ = glm::vec3(0.8f, 0.9f, 1.0f);
                collisionRadius_ = 0.6f;
                break;
                
            case CoinType::CRYSTAL:
                stats_.value = 15;
                stats_.experience = 12.0f;
                baseColor_ = glm::vec3(0.2f, 0.8f, 1.0f);
                collisionRadius_ = 0.5f;
                break;
                
            case CoinType::GEM:
                stats_.value = 50;
                stats_.experience = 40.0f;
                baseColor_ = glm::vec3(1.0f, 0.2f, 0.8f);
                collisionRadius_ = 0.4f;
                break;
                
            case CoinType::ENERGY_ORB:
                stats_.value = 0;
                stats_.experience = 5.0f;
                baseColor_ = glm::vec3(0.2f, 1.0f, 0.2f);
                collisionRadius_ = 0.6f;
                break;
                
            case CoinType::HEALTH_ORB:
                stats_.value = 20;
                stats_.experience = 2.0f;
                baseColor_ = glm::vec3(1.0f, 0.2f, 0.2f);
                collisionRadius_ = 0.7f;
                break;
                
            case CoinType::EXPERIENCE_ORB:
                stats_.value = 0;
                stats_.experience = 25.0f;
                baseColor_ = glm::vec3(0.8f, 0.2f, 1.0f);
                collisionRadius_ = 0.5f;
                break;
                
            default:
                stats_.value = 1;
                stats_.experience = 1.0f;
                baseColor_ = glm::vec3(1.0f, 1.0f, 1.0f);
                collisionRadius_ = 0.5f;
                break;
        }
        
        // تنظیم رنگ نادر
        switch (rarity_) {
            case Rarity::COMMON:
                rareColor_ = baseColor_;
                break;
            case Rarity::UNCOMMON:
                rareColor_ = glm::vec3(0.2f, 1.0f, 0.2f);
                break;
            case Rarity::RARE:
                rareColor_ = glm::vec3(0.2f, 0.5f, 1.0f);
                break;
            case Rarity::EPIC:
                rareColor_ = glm::vec3(0.8f, 0.2f, 1.0f);
                break;
            case Rarity::LEGENDARY:
                rareColor_ = glm::vec3(1.0f, 0.8f, 0.2f);
                break;
            case Rarity::MYTHIC:
                rareColor_ = glm::vec3(1.0f, 0.2f, 0.2f);
                break;
        }
    }

    void Coin::ApplyRarityMultipliers()
    {
        // اعمال مضارب بر اساس نادرتی
        switch (rarity_) {
            case Rarity::COMMON:
                // بدون تغییر
                break;
            case Rarity::UNCOMMON:
                stats_.value *= 2;
                stats_.experience *= 1.5f;
                stats_.magnetRange *= 1.2f;
                break;
            case Rarity::RARE:
                stats_.value *= 5;
                stats_.experience *= 3.0f;
                stats_.magnetRange *= 1.5f;
                stats_.glowIntensity *= 1.5f;
                break;
            case Rarity::EPIC:
                stats_.value *= 10;
                stats_.experience *= 6.0f;
                stats_.magnetRange *= 2.0f;
                stats_.glowIntensity *= 2.0f;
                break;
            case Rarity::LEGENDARY:
                stats_.value *= 25;
                stats_.experience *= 15.0f;
                stats_.magnetRange *= 3.0f;
                stats_.glowIntensity *= 3.0f;
                break;
            case Rarity::MYTHIC:
                stats_.value *= 50;
                stats_.experience *= 30.0f;
                stats_.magnetRange *= 5.0f;
                stats_.glowIntensity *= 5.0f;
                break;
        }
    }

    bool Coin::ShouldMagnetize() const
    {
        if (!targetPlayer_ || isMagnetized_) return false;
        
        float distance = DistanceToPlayer();
        return distance <= stats_.magnetRange;
    }

    float Coin::DistanceToPlayer() const
    {
        if (!targetPlayer_) return FLT_MAX;
        return glm::distance(position_, targetPlayer_->GetPosition());
    }

    void Coin::CompleteCollection()
    {
        SetState(CoinState::COLLECTED);
        
        // ایجاد افکت نهایی جمع‌آوری
        if (collectParticles_) {
            collectParticles_->EmitBurst(20, position_, glm::vec4(baseColor_, 1.0f));
        }
    }

    // پیاده‌سازی CoinManager (Singleton)
    CoinManager* CoinManager::instance_ = nullptr;

    CoinManager::CoinManager()
        : renderSystem_(nullptr), physicsEngine_(nullptr), audioManager_(nullptr),
          targetPlayer_(nullptr), totalCoinsCollected_(0), totalValueCollected_(0),
          totalExperienceGained_(0.0f), magnetPower_(1.0f), collectionRange_(5.0f),
          valueMultiplier_(1.0f), experienceMultiplier_(1.0f), autoCollection_(false)
    {
        std::cout << "💰 ایجاد مدیر سکه‌ها" << std::endl;
    }

    CoinManager::~CoinManager()
    {
        Cleanup();
    }

    CoinManager& CoinManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new CoinManager();
        }
        return *instance_;
    }

    void CoinManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool CoinManager::Initialize(RenderSystem* renderer, PhysicsEngine* physics,
                                AudioManager* audio, class Player* player)
    {
        renderSystem_ = renderer;
        physicsEngine_ = physics;
        audioManager_ = audio;
        targetPlayer_ = player;
        
        // ایجاد اسپانر
        spawner_ = std::make_unique<CoinSpawner>();
        if (!spawner_->Initialize(renderer, physics, audio, player)) {
            std::cerr << "❌ خطا در راه‌اندازی اسپانر سکه‌ها" << std::endl;
            return false;
        }
        
        std::cout << "✅ مدیر سکه‌ها با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void CoinManager::Update(float deltaTime)
    {
        if (spawner_) {
            spawner_->Update(deltaTime);
        }
        
        UpdateSpecialCoins(deltaTime);
        UpdateBossDrops(deltaTime);
        
        // جمع‌آوری خودکار
        if (autoCollection_ && targetPlayer_) {
            CollectAllInRange(targetPlayer_->GetPosition(), collectionRange_);
        }
    }

    void CoinManager::OnCoinCollected(Coin* coin)
    {
        totalCoinsCollected_++;
        totalValueCollected_ += coin->GetStats().value;
        totalExperienceGained_ += coin->GetStats().experience;
        
        // آمار بر اساس نوع
        coinsByType_[coin->GetType()]++;
        coinsByRarity_[coin->GetRarity()]++;
        
        // ایجاد افکت جمع‌آوری
        CreateCollectionEffect(coin->GetPosition(), coin->GetType(), coin->GetRarity());
        
        std::cout << "💰 سکه جمع‌آوری شد: ارزش=" << coin->GetStats().value 
                  << " تجربه=" << coin->GetStats().experience << std::endl;
    }

} // namespace GalacticOdyssey
