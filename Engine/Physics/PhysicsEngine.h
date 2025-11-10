#pragma once
#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace GalacticOdyssey {

    // انواع بدنه فیزیکی
    enum class BodyType {
        STATIC,     // جسم ثابت (زمین، دیوار)
        DYNAMIC,    // جسم متحرک (بازیکن، سکه)
        KINEMATIC   // جسم کنترل شده (پلتفرم متحرک)
    };

    // اشکال هندسی برای برخورد
    enum class CollisionShape {
        SPHERE,
        BOX,
        CAPSULE,
        CYLINDER,
        MESH,
        PLANE
    };

    // ساختار ماده فیزیکی
    struct Material {
        float density;          // چگالی
        float friction;         // اصطکاک
        float restitution;      // ارتجاع
        float damping;          // میرایی
        
        Material(float density = 1.0f, float friction = 0.5f, 
                float restitution = 0.3f, float damping = 0.1f)
            : density(density), friction(friction), 
              restitution(restitution), damping(damping) {}
    };

    // ساختار بدنه فیزیکی
    struct RigidBody {
        glm::vec3 position;     // موقعیت
        glm::vec3 velocity;     // سرعت
        glm::vec3 acceleration; // شتاب
        glm::vec3 force;        // نیروی اعمالی
        
        glm::vec3 angularVelocity;  // سرعت زاویه‌ای
        glm::vec3 torque;           // گشتاور
        
        float mass;             // جرم
        float inverseMass;      // معکوس جرم
        glm::mat3 inertiaTensor;    // تانسور اینرسی
        glm::mat3 inverseInertiaTensor; // معکوس تانسور اینرسی
        
        BodyType type;          // نوع بدنه
        CollisionShape shape;   // شکل برخورد
        glm::vec3 dimensions;   // ابعاد (برای باکس: طول، عرض، ارتفاع)
        
        Material material;      // ماده
        bool isAwake;           // وضعیت فعال
        float sleepTimer;       // تایمر خواب
        
        // برای تشخیص برخورد
        bool isColliding;
        glm::vec3 collisionNormal;
        float penetrationDepth;
        
        RigidBody() 
            : position(0.0f), velocity(0.0f), acceleration(0.0f), force(0.0f),
              angularVelocity(0.0f), torque(0.0f),
              mass(1.0f), inverseMass(1.0f),
              type(BodyType::DYNAMIC), shape(CollisionShape::SPHERE), dimensions(1.0f),
              isAwake(true), sleepTimer(0.0f),
              isColliding(false), collisionNormal(0.0f), penetrationDepth(0.0f) 
        {
            inertiaTensor = glm::mat3(1.0f);
            inverseInertiaTensor = glm::mat3(1.0f);
        }
        
        void SetMass(float newMass) {
            mass = newMass;
            inverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
            CalculateInertiaTensor();
        }
        
        void CalculateInertiaTensor() {
            switch (shape) {
                case CollisionShape::SPHERE: {
                    float radius = dimensions.x;
                    float I = 0.4f * mass * radius * radius;
                    inertiaTensor = glm::mat3(I);
                    inverseInertiaTensor = glm::mat3(1.0f / I);
                    break;
                }
                case CollisionShape::BOX: {
                    float w2 = dimensions.x * dimensions.x;
                    float h2 = dimensions.y * dimensions.y;
                    float d2 = dimensions.z * dimensions.z;
                    float Ixx = (1.0f/12.0f) * mass * (h2 + d2);
                    float Iyy = (1.0f/12.0f) * mass * (w2 + d2);
                    float Izz = (1.0f/12.0f) * mass * (w2 + h2);
                    inertiaTensor = glm::mat3(Ixx, 0, 0, 0, Iyy, 0, 0, 0, Izz);
                    inverseInertiaTensor = glm::mat3(1.0f/Ixx, 0, 0, 0, 1.0f/Iyy, 0, 0, 0, 1.0f/Izz);
                    break;
                }
                default:
                    inertiaTensor = glm::mat3(1.0f);
                    inverseInertiaTensor = glm::mat3(1.0f);
            }
        }
        
        void ApplyForce(const glm::vec3& forceVec, const glm::vec3& point = glm::vec3(0.0f)) {
            force += forceVec;
            if (point != glm::vec3(0.0f)) {
                glm::vec3 r = point - position;
                torque += glm::cross(r, forceVec);
            }
            WakeUp();
        }
        
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& point = glm::vec3(0.0f)) {
            velocity += impulse * inverseMass;
            if (point != glm::vec3(0.0f)) {
                glm::vec3 r = point - position;
                angularVelocity += inverseInertiaTensor * glm::cross(r, impulse);
            }
            WakeUp();
        }
        
        void WakeUp() {
            isAwake = true;
            sleepTimer = 0.0f;
        }
    };

    // ساختار برخورد
    struct Contact {
        RigidBody* bodyA;
        RigidBody* bodyB;
        glm::vec3 point;            // نقطه برخورد
        glm::vec3 normal;           // نرمال برخورد
        float penetration;          // عمق نفوذ
        glm::vec3 relativeVelocity; // سرعت نسبی
        float restitution;          // ضریب ارتجاع
        float friction;             // ضریب اصطکاک
        
        Contact() 
            : bodyA(nullptr), bodyB(nullptr), point(0.0f), normal(0.0f),
              penetration(0.0f), relativeVelocity(0.0f),
              restitution(0.3f), friction(0.5f) {}
    };

    // ساختار Ray برای Raycasting
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
        float maxDistance;
        
        Ray(const glm::vec3& origin = glm::vec3(0.0f), 
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
            float maxDistance = 1000.0f)
            : origin(origin), direction(glm::normalize(direction)), 
              maxDistance(maxDistance) {}
    };

    // ساختار نتیجه Raycast
    struct RaycastResult {
        bool hit;                   // آیا برخورد رخ داده؟
        glm::vec3 point;            // نقطه برخورد
        glm::vec3 normal;           // نرمال سطح
        float distance;             // فاصله تا نقطه برخورد
        RigidBody* body;            // جسم برخورد کرده
        
        RaycastResult() 
            : hit(false), point(0.0f), normal(0.0f), 
              distance(0.0f), body(nullptr) {}
    };

    // کلاس اصلی موتور فیزیک
    class PhysicsEngine {
    private:
        // بدنه‌های فیزیکی
        std::vector<std::unique_ptr<RigidBody>> bodies_;
        std::unordered_map<uint32_t, RigidBody*> bodyMap_;
        
        // برخوردها
        std::vector<Contact> contacts_;
        
        // تنظیمات جهانی
        glm::vec3 gravity_;
        float airDensity_;
        float timeScale_;
        int iterations_;        // تعداد تکرارهای حل کننده
        
        // مدیریت فضایی
        class SpatialHash* spatialHash_;
        
        // Callback برای رویدادهای برخورد
        std::function<void(RigidBody*, RigidBody*, const Contact&)> collisionCallback_;
        
        // آمار
        int framesPerSecond_;
        int bodyCount_;
        int collisionCount_;
        
    public:
        PhysicsEngine();
        ~PhysicsEngine();
        
        bool Initialize();
        void Cleanup();
        
        // مدیریت بدنه‌ها
        RigidBody* CreateBody(BodyType type = BodyType::DYNAMIC);
        void DestroyBody(RigidBody* body);
        RigidBody* GetBody(uint32_t id) const;
        
        // به‌روزرسانی فیزیک
        void Update(float deltaTime);
        void StepSimulation(float deltaTime);
        
        // Raycasting
        RaycastResult Raycast(const Ray& ray, uint32_t layerMask = 0xFFFFFFFF);
        std::vector<RaycastResult> RaycastAll(const Ray& ray, uint32_t layerMask = 0xFFFFFFFF);
        
        // پرس‌وجوهای فضایی
        std::vector<RigidBody*> OverlapSphere(const glm::vec3& center, float radius, uint32_t layerMask = 0xFFFFFFFF);
        std::vector<RigidBody*> OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents, uint32_t layerMask = 0xFFFFFFFF);
        
        // تنظیمات
        void SetGravity(const glm::vec3& gravity) { gravity_ = gravity; }
        void SetAirDensity(float density) { airDensity_ = density; }
        void SetTimeScale(float scale) { timeScale_ = scale; }
        void SetIterations(int iterations) { iterations_ = iterations; }
        
        const glm::vec3& GetGravity() const { return gravity_; }
        float GetAirDensity() const { return airDensity_; }
        float GetTimeScale() const { return timeScale_; }
        
        // مدیریت رویدادها
        void SetCollisionCallback(std::function<void(RigidBody*, RigidBody*, const Contact&)> callback) {
            collisionCallback_ = callback;
        }
        
        // آمار
        int GetBodyCount() const { return bodyCount_; }
        int GetCollisionCount() const { return collisionCount_; }
        int GetFPS() const { return framesPerSecond_; }
        
    private:
        // مراحل شبیه‌سازی
        void IntegrateForces(float deltaTime);
        void DetectCollisions();
        void ResolveCollisions(float deltaTime);
        void IntegrateVelocities(float deltaTime);
        void UpdateSleepState(float deltaTime);
        
        // تشخیص برخورد
        bool CheckCollision(RigidBody* a, RigidBody* b, Contact& contact);
        bool SphereSphereCollision(RigidBody* a, RigidBody* b, Contact& contact);
        bool SphereBoxCollision(RigidBody* sphere, RigidBody* box, Contact& contact);
        bool BoxBoxCollision(RigidBody* a, RigidBody* b, Contact& contact);
        
        // کمک‌کننده‌های ریاضی
        glm::vec3 CalculateSupport(const RigidBody* body, const glm::vec3& direction);
        bool SATTest(const RigidBody* a, const RigidBody* b, Contact& contact);
        
        // محاسبات پاسخ برخورد
        void ResolveContact(Contact& contact, float deltaTime);
        void ApplyImpulse(Contact& contact);
        void ApplyFriction(Contact& contact, float deltaTime);
        
        // محاسبات اینرسی
        void UpdateInertiaTensor(RigidBody* body);
        
        // بهینه‌سازی
        void BroadPhase();
        void NarrowPhase();
    };

    // سیستم ذرات فیزیکی برای افکت‌ها
    class ParticlePhysics {
    private:
        struct PhysicsParticle {
            glm::vec3 position;
            glm::vec3 velocity;
            glm::vec3 acceleration;
            float mass;
            float lifetime;
            float maxLifetime;
            bool active;
            
            PhysicsParticle() 
                : position(0.0f), velocity(0.0f), acceleration(0.0f),
                  mass(1.0f), lifetime(0.0f), maxLifetime(1.0f), active(false) {}
        };
        
        std::vector<PhysicsParticle> particles_;
        glm::vec3 globalForce_;
        int activeParticles_;
        
    public:
        ParticlePhysics(int maxParticles = 1000);
        ~ParticlePhysics();
        
        void Update(float deltaTime);
        void EmitParticle(const glm::vec3& position, const glm::vec3& velocity, 
                         float lifetime = 1.0f, float mass = 1.0f);
        void ApplyForce(const glm::vec3& force);
        void SetGlobalForce(const glm::vec3& force) { globalForce_ = force; }
        
        const std::vector<PhysicsParticle>& GetParticles() const { return particles_; }
        int GetActiveParticles() const { return activeParticles_; }
        
    private:
        void IntegrateParticle(PhysicsParticle& particle, float deltaTime);
    };

    // سیستم مفاصل و قیدها
    class ConstraintSolver {
    private:
        struct Constraint {
            RigidBody* bodyA;
            RigidBody* bodyB;
            glm::vec3 anchorA;
            glm::vec3 anchorB;
            float distance;
            float stiffness;
            float damping;
            
            Constraint(RigidBody* a, RigidBody* b, const glm::vec3& anchorA, 
                      const glm::vec3& anchorB, float stiffness = 0.9f)
                : bodyA(a), bodyB(b), anchorA(anchorA), anchorB(anchorB),
                  stiffness(stiffness), damping(0.1f) 
            {
                distance = glm::length(anchorB - anchorA);
            }
        };
        
        std::vector<Constraint> constraints_;
        int iterations_;
        
    public:
        ConstraintSolver(int iterations = 10);
        ~ConstraintSolver();
        
        void AddConstraint(RigidBody* a, RigidBody* b, const glm::vec3& anchorA, const glm::vec3& anchorB);
        void RemoveConstraint(RigidBody* a, RigidBody* b);
        void SolveConstraints(float deltaTime);
        void SetIterations(int iterations) { iterations_ = iterations; }
        
    private:
        void SolveDistanceConstraint(Constraint& constraint, float deltaTime);
    };

    // سیستم انفجار و نیروهای محیطی
    class ForceField {
    private:
        struct Explosion {
            glm::vec3 center;
            float radius;
            float force;
            float duration;
            float currentTime;
            
            Explosion(const glm::vec3& center, float radius, float force, float duration)
                : center(center), radius(radius), force(force), 
                  duration(duration), currentTime(0.0f) {}
        };
        
        std::vector<Explosion> explosions_;
        glm::vec3 windForce_;
        glm::vec3 turbulence_;
        
    public:
        ForceField();
        ~ForceField();
        
        void Update(float deltaTime, PhysicsEngine* physics);
        void AddExplosion(const glm::vec3& center, float radius, float force, float duration = 1.0f);
        void SetWind(const glm::vec3& wind) { windForce_ = wind; }
        void SetTurbulence(const glm::vec3& turbulence) { turbulence_ = turbulence; }
        
    private:
        void ApplyExplosionForces(Explosion& explosion, PhysicsEngine* physics);
        void ApplyEnvironmentalForces(RigidBody* body);
    };

    // سیستم تشخیص برخورد پیشرفته با GJK و EPA
    class CollisionDetector {
    public:
        static bool GJK(const RigidBody* a, const RigidBody* b, Contact& contact);
        static bool EPA(const RigidBody* a, const RigidBody* b, const glm::vec3& simplex, Contact& contact);
        
    private:
        static glm::vec3 Support(const RigidBody* a, const RigidBody* b, const glm::vec3& direction);
        static bool ContainsOrigin(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, glm::vec3& direction);
        static bool ContainsOrigin(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d, glm::vec3& direction);
    };

    // سیستم بهینه‌سازی فضایی
    class SpatialHash {
    private:
        struct Cell {
            std::vector<RigidBody*> bodies;
        };
        
        float cellSize_;
        int gridSize_;
        std::unordered_map<uint64_t, Cell> grid_;
        
    public:
        SpatialHash(float cellSize = 2.0f, int gridSize = 1000);
        ~SpatialHash();
        
        void Clear();
        void Insert(RigidBody* body);
        void Remove(RigidBody* body);
        void Update(RigidBody* body, const glm::vec3& oldPos);
        
        std::vector<RigidBody*> Query(const glm::vec3& position, float radius);
        std::vector<RigidBody*> Query(const glm::vec3& min, const glm::vec3& max);
        
    private:
        uint64_t Hash(int x, int y, int z) const;
        void GetCellBounds(const glm::vec3& position, float radius, 
                          int& minX, int& maxX, int& minY, int& maxY, int& minZ, int& maxZ) const;
    };

    // سیستم فیزیک نرم (Soft Body)
    class SoftBodySystem {
    private:
        struct SoftBodyParticle {
            glm::vec3 position;
            glm::vec3 velocity;
            glm::vec3 force;
            float mass;
            bool fixed;
            
            SoftBodyParticle(const glm::vec3& pos, float mass = 1.0f, bool fixed = false)
                : position(pos), velocity(0.0f), force(0.0f), mass(mass), fixed(fixed) {}
        };
        
        struct Spring {
            int particleA;
            int particleB;
            float restLength;
            float stiffness;
            float damping;
            
            Spring(int a, int b, float stiffness = 0.5f, float damping = 0.1f)
                : particleA(a), particleB(b), stiffness(stiffness), damping(damping) 
            {
                restLength = 0.0f; // در مقداردهی اولیه محاسبه می‌شود
            }
        };
        
        std::vector<SoftBodyParticle> particles_;
        std::vector<Spring> springs_;
        std::vector<glm::vec3> normals_;
        
    public:
        SoftBodySystem();
        ~SoftBodySystem();
        
        void Update(float deltaTime);
        void AddParticle(const glm::vec3& position, float mass = 1.0f, bool fixed = false);
        void AddSpring(int particleA, int particleB, float stiffness = 0.5f);
        void CreateCloth(int width, int height, float spacing = 1.0f);
        void CreateSphere(float radius, int segments = 8);
        
        const std::vector<SoftBodyParticle>& GetParticles() const { return particles_; }
        const std::vector<Spring>& GetSprings() const { return springs_; }
        const std::vector<glm::vec3>& GetNormals() const { return normals_; }
        
    private:
        void CalculateNormals();
        void ApplySpringForces();
        void IntegrateParticles(float deltaTime);
        void SatisfyConstraints();
    };

} // namespace GalacticOdyssey

#endif // PHYSICS_ENGINE_H
