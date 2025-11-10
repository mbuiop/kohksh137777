#include "PhysicsEngine.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace GalacticOdyssey {

    // پیاده‌سازی PhysicsEngine
    PhysicsEngine::PhysicsEngine()
        : gravity_(0.0f, -9.81f, 0.0f), airDensity_(1.2f), timeScale_(1.0f),
          iterations_(10), spatialHash_(nullptr),
          framesPerSecond_(0), bodyCount_(0), collisionCount_(0) 
    {
        std::cout << "🔧 ایجاد موتور فیزیک" << std::endl;
    }

    PhysicsEngine::~PhysicsEngine()
    {
        Cleanup();
    }

    bool PhysicsEngine::Initialize()
    {
        std::cout << "🔧 در حال راه‌اندازی موتور فیزیک..." << std::endl;
        
        spatialHash_ = new SpatialHash(2.0f, 1000);
        
        std::cout << "✅ موتور فیزیک با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void PhysicsEngine::Cleanup()
    {
        std::cout << "🧹 پاکسازی موتور فیزیک..." << std::endl;
        
        bodies_.clear();
        bodyMap_.clear();
        contacts_.clear();
        
        if (spatialHash_) {
            delete spatialHash_;
            spatialHash_ = nullptr;
        }
        
        std::cout << "✅ موتور فیزیک پاکسازی شد" << std::endl;
    }

    RigidBody* PhysicsEngine::CreateBody(BodyType type)
    {
        auto body = std::make_unique<RigidBody>();
        body->type = type;
        
        if (type == BodyType::STATIC) {
            body->SetMass(0.0f); // جرم بی‌نهایت
        }
        
        RigidBody* result = body.get();
        uint32_t id = static_cast<uint32_t>(bodies_.size());
        bodies_.push_back(std::move(body));
        bodyMap_[id] = result;
        
        bodyCount_++;
        return result;
    }

    void PhysicsEngine::DestroyBody(RigidBody* body)
    {
        auto it = std::find_if(bodies_.begin(), bodies_.end(),
            [body](const std::unique_ptr<RigidBody>& b) { return b.get() == body; });
        
        if (it != bodies_.end()) {
            if (spatialHash_) {
                spatialHash_->Remove(body);
            }
            bodies_.erase(it);
            bodyCount_--;
        }
    }

    void PhysicsEngine::Update(float deltaTime)
    {
        if (deltaTime <= 0.0f) return;
        
        // اعمال مقیاس زمان
        float scaledDeltaTime = deltaTime * timeScale_;
        
        // به‌روزرسانی هش فضایی
        if (spatialHash_) {
            spatialHash_->Clear();
            for (auto& body : bodies_) {
                if (body->isAwake) {
                    spatialHash_->Insert(body.get());
                }
            }
        }
        
        // تشخیص برخورد
        DetectCollisions();
        
        // حل فیزیک در چند مرحله برای پایداری
        int substeps = 3;
        float substepDelta = scaledDeltaTime / substeps;
        
        for (int i = 0; i < substeps; i++) {
            StepSimulation(substepDelta);
        }
        
        collisionCount_ = static_cast<int>(contacts_.size());
    }

    void PhysicsEngine::StepSimulation(float deltaTime)
    {
        // مرحله 1: یکپارچه‌سازی نیروها
        IntegrateForces(deltaTime);
        
        // مرحله 2: حل برخوردها
        ResolveCollisions(deltaTime);
        
        // مرحله 3: یکپارچه‌سازی سرعت‌ها
        IntegrateVelocities(deltaTime);
        
        // مرحله 4: به‌روزرسانی وضعیت خواب
        UpdateSleepState(deltaTime);
        
        // پاکسازی نیروهای فریم جاری
        for (auto& body : bodies_) {
            body->force = glm::vec3(0.0f);
            body->torque = glm::vec3(0.0f);
        }
        
        contacts_.clear();
    }

    void PhysicsEngine::IntegrateForces(float deltaTime)
    {
        for (auto& body : bodies_) {
            if (body->type != BodyType::DYNAMIC || !body->isAwake) continue;
            
            // اعمال گرانش
            body->force += gravity_ * body->mass;
            
            // اعمال مقاومت هوا
            if (airDensity_ > 0.0f && glm::length(body->velocity) > 0.1f) {
                glm::vec3 dragForce = -0.5f * airDensity_ * body->velocity * glm::length(body->velocity);
                body->force += dragForce;
            }
            
            // شتاب از F = ma
            body->acceleration = body->force * body->inverseMass;
            
            // سرعت زاویه‌ای از τ = Iα
            if (body->inverseMass > 0.0f) {
                glm::vec3 angularAcceleration = body->inverseInertiaTensor * body->torque;
                body->angularVelocity += angularAcceleration * deltaTime;
                
                // میرایی زاویه‌ای
                body->angularVelocity *= (1.0f - body->material.damping * deltaTime);
            }
        }
    }

    void PhysicsEngine::IntegrateVelocities(float deltaTime)
    {
        for (auto& body : bodies_) {
            if (body->type != BodyType::DYNAMIC || !body->isAwake) continue;
            
            // یکپارچه‌سازی سرعت
            body->velocity += body->acceleration * deltaTime;
            
            // میرایی خطی
            body->velocity *= (1.0f - body->material.damping * deltaTime);
            
            // یکپارچه‌سازی موقعیت
            body->position += body->velocity * deltaTime;
            
            // یکپارچه‌سازی چرخش (ساده‌سازی)
            if (glm::length(body->angularVelocity) > 0.001f) {
                // در پیاده‌سازی کامل از کواترنیون استفاده می‌شود
            }
        }
    }

    void PhysicsEngine::DetectCollisions()
    {
        BroadPhase();
        NarrowPhase();
    }

    void PhysicsEngine::BroadPhase()
    {
        // در این فاز از هش فضایی برای پیدا کردن جفت‌های بالقوه برخورد استفاده می‌کنیم
        contacts_.clear();
        
        if (!spatialHash_) return;
        
        std::vector<std::pair<RigidBody*, RigidBody*>> potentialPairs;
        
        // برای هر بدنه، همسایه‌های بالقوه را پیدا کن
        for (auto& body : bodies_) {
            if (!body->isAwake) continue;
            
            float checkRadius = 0.0f;
            switch (body->shape) {
                case CollisionShape::SPHERE:
                    checkRadius = body->dimensions.x;
                    break;
                case CollisionShape::BOX:
                    checkRadius = glm::length(body->dimensions) * 0.5f;
                    break;
                default:
                    checkRadius = 1.0f;
            }
            
            auto neighbors = spatialHash_->Query(body->position, checkRadius * 2.0f);
            
            for (auto* neighbor : neighbors) {
                if (neighbor == body.get()) continue;
                
                // جلوگیری از بررسی تکراری
                if (body.get() < neighbor) {
                    potentialPairs.emplace_back(body.get(), neighbor);
                }
            }
        }
        
        // فاز Narrow را روی جفت‌های بالقوه اجرا کن
        for (auto& pair : potentialPairs) {
            Contact contact;
            if (CheckCollision(pair.first, pair.second, contact)) {
                contacts_.push_back(contact);
                
                // فراخوانی callback
                if (collisionCallback_) {
                    collisionCallback_(pair.first, pair.second, contact);
                }
            }
        }
    }

    void PhysicsEngine::NarrowPhase()
    {
        // در این فاز برخوردهای دقیق تشخیص داده می‌شوند
        // این کار در BroadPhase انجام شده است
    }

    bool PhysicsEngine::CheckCollision(RigidBody* a, RigidBody* b, Contact& contact)
    {
        contact.bodyA = a;
        contact.bodyB = b;
        
        // ترکیب اشکال مختلف
        if (a->shape == CollisionShape::SPHERE && b->shape == CollisionShape::SPHERE) {
            return SphereSphereCollision(a, b, contact);
        }
        else if (a->shape == CollisionShape::BOX && b->shape == CollisionShape::BOX) {
            return BoxBoxCollision(a, b, contact);
        }
        else if ((a->shape == CollisionShape::SPHERE && b->shape == CollisionShape::BOX) ||
                 (a->shape == CollisionShape::BOX && b->shape == CollisionShape::SPHERE)) {
            return SphereBoxCollision(
                a->shape == CollisionShape::SPHERE ? a : b,
                a->shape == CollisionShape::BOX ? a : b,
                contact
            );
        }
        
        return false;
    }

    bool PhysicsEngine::SphereSphereCollision(RigidBody* a, RigidBody* b, Contact& contact)
    {
        glm::vec3 delta = b->position - a->position;
        float distance = glm::length(delta);
        float radiusSum = a->dimensions.x + b->dimensions.x;
        
        if (distance < radiusSum && distance > 0.0f) {
            contact.penetration = radiusSum - distance;
            contact.normal = glm::normalize(delta);
            contact.point = a->position + contact.normal * a->dimensions.x;
            contact.restitution = std::min(a->material.restitution, b->material.restitution);
            contact.friction = std::min(a->material.friction, b->material.friction);
            
            // محاسبه سرعت نسبی
            glm::vec3 rv = b->velocity - a->velocity;
            contact.relativeVelocity = rv;
            
            return true;
        }
        
        return false;
    }

    bool PhysicsEngine::BoxBoxCollision(RigidBody* a, RigidBody* b, Contact& contact)
    {
        // ساده‌سازی - استفاده از AABB
        glm::vec3 aMin = a->position - a->dimensions * 0.5f;
        glm::vec3 aMax = a->position + a->dimensions * 0.5f;
        glm::vec3 bMin = b->position - b->dimensions * 0.5f;
        glm::vec3 bMax = b->position + b->dimensions * 0.5f;
        
        if (aMax.x > bMin.x && aMin.x < bMax.x &&
            aMax.y > bMin.y && aMin.y < bMax.y &&
            aMax.z > bMin.z && aMin.z < bMax.z) {
            
            // محاسبه عمق نفوذ در هر محور
            glm::vec3 overlaps(
                std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x),
                std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y),
                std::min(aMax.z, bMax.z) - std::max(aMin.z, bMin.z)
            );
            
            // پیدا کردن محور با کمترین نفوذ
            if (overlaps.x < overlaps.y && overlaps.x < overlaps.z) {
                contact.normal = glm::vec3(a->position.x < b->position.x ? -1.0f : 1.0f, 0.0f, 0.0f);
                contact.penetration = overlaps.x;
            }
            else if (overlaps.y < overlaps.z) {
                contact.normal = glm::vec3(0.0f, a->position.y < b->position.y ? -1.0f : 1.0f, 0.0f);
                contact.penetration = overlaps.y;
            }
            else {
                contact.normal = glm::vec3(0.0f, 0.0f, a->position.z < b->position.z ? -1.0f : 1.0f);
                contact.penetration = overlaps.z;
            }
            
            contact.point = a->position;
            contact.restitution = std::min(a->material.restitution, b->material.restitution);
            contact.friction = std::min(a->material.friction, b->material.friction);
            
            return true;
        }
        
        return false;
    }

    bool PhysicsEngine::SphereBoxCollision(RigidBody* sphere, RigidBody* box, Contact& contact)
    {
        // پیدا کردن نزدیک‌ترین نقطه روی باکس به کره
        glm::vec3 boxMin = box->position - box->dimensions * 0.5f;
        glm::vec3 boxMax = box->position + box->dimensions * 0.5f;
        
        glm::vec3 closestPoint;
        closestPoint.x = std::max(boxMin.x, std::min(sphere->position.x, boxMax.x));
        closestPoint.y = std::max(boxMin.y, std::min(sphere->position.y, boxMax.y));
        closestPoint.z = std::max(boxMin.z, std::min(sphere->position.z, boxMax.z));
        
        float distance = glm::length(sphere->position - closestPoint);
        float sphereRadius = sphere->dimensions.x;
        
        if (distance < sphereRadius) {
            contact.normal = glm::normalize(sphere->position - closestPoint);
            contact.penetration = sphereRadius - distance;
            contact.point = closestPoint;
            contact.restitution = std::min(sphere->material.restitution, box->material.restitution);
            contact.friction = std::min(sphere->material.friction, box->material.friction);
            return true;
        }
        
        return false;
    }

    void PhysicsEngine::ResolveCollisions(float deltaTime)
    {
        for (auto& contact : contacts_) {
            ResolveContact(contact, deltaTime);
        }
    }

    void PhysicsEngine::ResolveContact(Contact& contact, float deltaTime)
    {
        RigidBody* a = contact.bodyA;
        RigidBody* b = contact.bodyB;
        
        if (!a->isAwake && !b->isAwake) return;
        
        // محاسبه سرعت نسبی در جهت نرمال
        glm::vec3 relativeVelocity = b->velocity - a->velocity;
        float velocityAlongNormal = glm::dot(relativeVelocity, contact.normal);
        
        // اگر اجسام در حال دور شدن هستند، برخورد را حل نکن
        if (velocityAlongNormal > 0) return;
        
        // ضریب ارتجاع
        float e = contact.restitution;
        
        // ضریب impulse scalar
        float j = -(1 + e) * velocityAlongNormal;
        j /= a->inverseMass + b->inverseMass;
        
        // اعمال impulse
        glm::vec3 impulse = j * contact.normal;
        a->velocity -= impulse * a->inverseMass;
        b->velocity += impulse * b->inverseMass;
        
        // اصطکاک
        ApplyFriction(contact, deltaTime);
        
        // تصحیح موقعیت برای جلوگیری از نفوذ
        const float percent = 0.2f; // 20% correction
        const float slop = 0.01f;   // 1cm slop
        float correction = std::max(contact.penetration - slop, 0.0f) / (a->inverseMass + b->inverseMass) * percent;
        glm::vec3 correctionVector = correction * contact.normal;
        
        a->position -= correctionVector * a->inverseMass;
        b->position += correctionVector * b->inverseMass;
        
        // بیدار کردن اجسام
        a->WakeUp();
        b->WakeUp();
    }

    void PhysicsEngine::ApplyFriction(Contact& contact, float deltaTime)
    {
        RigidBody* a = contact.bodyA;
        RigidBody* b = contact.bodyB;
        
        glm::vec3 relativeVelocity = b->velocity - a->velocity;
        glm::vec3 tangent = relativeVelocity - glm::dot(relativeVelocity, contact.normal) * contact.normal;
        
        if (glm::length(tangent) > 0.001f) {
            tangent = glm::normalize(tangent);
            
            // magnitude به دلیل اصطکاک
            float jt = -glm::dot(relativeVelocity, tangent);
            jt /= a->inverseMass + b->inverseMass;
            
            // قانون کولن برای اصطکاک
            float mu = contact.friction;
            glm::vec3 frictionImpulse;
            if (std::abs(jt) < jt * mu) {
                frictionImpulse = jt * tangent;
            } else {
                frictionImpulse = -jt * tangent * mu;
            }
            
            // اعمال اصطکاک
            a->velocity -= frictionImpulse * a->inverseMass;
            b->velocity += frictionImpulse * b->inverseMass;
        }
    }

    void PhysicsEngine::UpdateSleepState(float deltaTime)
    {
        const float sleepThreshold = 0.1f;
        const float sleepTime = 2.0f; // 2 seconds
        
        for (auto& body : bodies_) {
            if (body->type != BodyType::DYNAMIC) continue;
            
            float velocity = glm::length(body->velocity);
            float angularVelocity = glm::length(body->angularVelocity);
            
            if (velocity < sleepThreshold && angularVelocity < sleepThreshold) {
                body->sleepTimer += deltaTime;
                if (body->sleepTimer >= sleepTime) {
                    body->isAwake = false;
                    body->velocity = glm::vec3(0.0f);
                    body->angularVelocity = glm::vec3(0.0f);
                }
            } else {
                body->sleepTimer = 0.0f;
                body->isAwake = true;
            }
        }
    }

    RaycastResult PhysicsEngine::Raycast(const Ray& ray, uint32_t layerMask)
    {
        RaycastResult result;
        float closestDistance = ray.maxDistance;
        
        for (auto& body : bodies_) {
            if (!body->isAwake) continue;
            
            // بررسی برخورد بر اساس شکل
            switch (body->shape) {
                case CollisionShape::SPHERE: {
                    glm::vec3 toSphere = body->position - ray.origin;
                    float t = glm::dot(toSphere, ray.direction);
                    glm::vec3 closestPoint = ray.origin + ray.direction * t;
                    float distance = glm::length(closestPoint - body->position);
                    
                    if (distance <= body->dimensions.x && t >= 0 && t < closestDistance) {
                        result.hit = true;
                        result.point = closestPoint;
                        result.normal = glm::normalize(closestPoint - body->position);
                        result.distance = t;
                        result.body = body.get();
                        closestDistance = t;
                    }
                    break;
                }
                case CollisionShape::BOX: {
                    // ساده‌سازی - AABB raycast
                    glm::vec3 boxMin = body->position - body->dimensions * 0.5f;
                    glm::vec3 boxMax = body->position + body->dimensions * 0.5f;
                    
                    glm::vec3 t1 = (boxMin - ray.origin) / ray.direction;
                    glm::vec3 t2 = (boxMax - ray.origin) / ray.direction;
                    
                    glm::vec3 tmin = glm::min(t1, t2);
                    glm::vec3 tmax = glm::max(t1, t2);
                    
                    float tminVal = std::max(std::max(tmin.x, tmin.y), tmin.z);
                    float tmaxVal = std::min(std::min(tmax.x, tmax.y), tmax.z);
                    
                    if (tmaxVal >= tminVal && tminVal < closestDistance && tminVal >= 0) {
                        result.hit = true;
                        result.point = ray.origin + ray.direction * tminVal;
                        result.distance = tminVal;
                        result.body = body.get();
                        
                        // محاسبه نرمال
                        if (tminVal == tmin.x) result.normal = glm::vec3(-1, 0, 0);
                        else if (tminVal == tmin.y) result.normal = glm::vec3(0, -1, 0);
                        else result.normal = glm::vec3(0, 0, -1);
                        
                        closestDistance = tminVal;
                    }
                    break;
                }
            }
        }
        
        return result;
    }

    // پیاده‌سازی SpatialHash
    SpatialHash::SpatialHash(float cellSize, int gridSize)
        : cellSize_(cellSize), gridSize_(gridSize) {}

    SpatialHash::~SpatialHash()
    {
        Clear();
    }

    void SpatialHash::Clear()
    {
        grid_.clear();
    }

    void SpatialHash::Insert(RigidBody* body)
    {
        int minX, maxX, minY, maxY, minZ, maxZ;
        GetCellBounds(body->position, body->dimensions.x, minX, maxX, minY, maxY, minZ, maxZ);
        
        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                for (int z = minZ; z <= maxZ; z++) {
                    uint64_t hash = Hash(x, y, z);
                    grid_[hash].bodies.push_back(body);
                }
            }
        }
    }

    void SpatialHash::Remove(RigidBody* body)
    {
        // برای سادگی، کل grid پاک می‌شود و دوباره پر می‌شود
        // در پیاده‌سازی بهینه‌تر، body از سلول‌های خاص حذف می‌شود
    }

    std::vector<RigidBody*> SpatialHash::Query(const glm::vec3& position, float radius)
    {
        std::vector<RigidBody*> results;
        int minX, maxX, minY, maxY, minZ, maxZ;
        
        GetCellBounds(position, radius, minX, maxX, minY, maxY, minZ, maxZ);
        
        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                for (int z = minZ; z <= maxZ; z++) {
                    uint64_t hash = Hash(x, y, z);
                    auto it = grid_.find(hash);
                    if (it != grid_.end()) {
                        for (auto* body : it->second.bodies) {
                            if (std::find(results.begin(), results.end(), body) == results.end()) {
                                results.push_back(body);
                            }
                        }
                    }
                }
            }
        }
        
        return results;
    }

    uint64_t SpatialHash::Hash(int x, int y, int z) const
    {
        // هش ساده برای مختصات سلول
        const uint64_t p1 = 73856093;
        const uint64_t p2 = 19349663;
        const uint64_t p3 = 83492791;
        
        return (x * p1) ^ (y * p2) ^ (z * p3);
    }

    void SpatialHash::GetCellBounds(const glm::vec3& position, float radius,
                                   int& minX, int& maxX, int& minY, int& maxY, int& minZ, int& maxZ) const
    {
        minX = static_cast<int>((position.x - radius) / cellSize_);
        maxX = static_cast<int>((position.x + radius) / cellSize_);
        minY = static_cast<int>((position.y - radius) / cellSize_);
        maxY = static_cast<int>((position.y + radius) / cellSize_);
        minZ = static_cast<int>((position.z - radius) / cellSize_);
        maxZ = static_cast<int>((position.z + radius) / cellSize_);
    }

    // پیاده‌سازی stub برای سایر کلاس‌ها
    ParticlePhysics::ParticlePhysics(int maxParticles) 
        : globalForce_(0.0f), activeParticles_(0)
    {
        particles_.resize(maxParticles);
    }

    ParticlePhysics::~ParticlePhysics() {}

    void ParticlePhysics::Update(float deltaTime)
    {
        for (auto& particle : particles_) {
            if (!particle.active) continue;
            
            IntegrateParticle(particle, deltaTime);
        }
    }

    void ParticlePhysics::IntegrateParticle(PhysicsParticle& particle, float deltaTime)
    {
        if (!particle.active) return;
        
        // اعمال نیروها
        particle.velocity += (particle.acceleration + globalForce_) * deltaTime;
        
        // یکپارچه‌سازی موقعیت
        particle.position += particle.velocity * deltaTime;
        
        // به‌روزرسانی طول عمر
        particle.lifetime += deltaTime;
        if (particle.lifetime >= particle.maxLifetime) {
            particle.active = false;
            activeParticles_--;
        }
    }

} // namespace GalacticOdyssey
