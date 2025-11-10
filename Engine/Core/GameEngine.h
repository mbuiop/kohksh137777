#pragma once
#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace GalacticOdyssey {

    // تعریف انواع رویدادهای بازی
    enum class GameEvent {
        GAME_START,
        GAME_PAUSE,
        GAME_RESUME,
        GAME_OVER,
        LEVEL_COMPLETE,
        PLAYER_HIT,
        COIN_COLLECTED,
        BOMB_USED,
        FUEL_LOW,
        ACHIEVEMENT_UNLOCKED
    };

    // ساختار تنظیمات گرافیکی
    struct GraphicsSettings {
        int screenWidth;
        int screenHeight;
        bool fullscreen;
        bool vsync;
        int msaaSamples;
        float fieldOfView;
        float nearPlane;
        float farPlane;
        bool bloomEnabled;
        bool motionBlurEnabled;
        bool shadowsEnabled;
        
        GraphicsSettings() :
            screenWidth(1920),
            screenHeight(1080),
            fullscreen(false),
            vsync(true),
            msaaSamples(4),
            fieldOfView(60.0f),
            nearPlane(0.1f),
            farPlane(1000.0f),
            bloomEnabled(true),
            motionBlurEnabled(false),
            shadowsEnabled(true) {}
    };

    // ساختار تنظیمات صوتی
    struct AudioSettings {
        float masterVolume;
        float musicVolume;
        float effectsVolume;
        bool spatialAudio;
        int audioChannels;
        
        AudioSettings() :
            masterVolume(1.0f),
            musicVolume(0.7f),
            effectsVolume(0.8f),
            spatialAudio(true),
            audioChannels(32) {}
    };

    // ساختار تنظیمات فیزیک
    struct PhysicsSettings {
        float gravity;
        float airResistance;
        float timeScale;
        bool collisionDetection;
        bool raycastingEnabled;
        
        PhysicsSettings() :
            gravity(0.0f),
            airResistance(0.1f),
            timeScale(1.0f),
            collisionDetection(true),
            raycastingEnabled(true) {}
    };

    // کلاس اصلی موتور بازی
    class GameEngine {
    protected:
        // پنجره و زمینه OpenGL
        SDL_Window* window_;
        SDL_GLContext glContext_;
        
        // تنظیمات
        GraphicsSettings graphicsSettings_;
        AudioSettings audioSettings_;
        PhysicsSettings physicsSettings_;
        
        // وضعیت موتور
        bool isInitialized_;
        bool isRunning_;
        bool isPaused_;
        std::string gameTitle_;
        
        // زمان‌سنج‌ها
        Uint32 lastFrameTime_;
        Uint32 currentFrameTime_;
        float deltaTime_;
        float fps_;
        Uint32 frameCount_;
        Uint32 fpsTimer_;
        
        // سیستم‌های وابسته
        class RenderSystem* renderSystem_;
        class PhysicsEngine* physicsEngine_;
        class AudioManager* audioManager_;
        class InputHandler* inputHandler_;
        
        // مدیریت رویدادها
        std::unordered_map<GameEvent, std::vector<std::function<void()>>> eventListeners_;

    public:
        // سازنده و تخریب‌کننده
        GameEngine(int width = 1920, int height = 1080, const std::string& title = "Galactic Odyssey");
        virtual ~GameEngine();

        // متدهای اصلی
        virtual bool Initialize();
        virtual void Run();
        virtual void Cleanup();
        
        // مدیریت رویدادها
        void AddEventListener(GameEvent event, std::function<void()> callback);
        void RemoveEventListener(GameEvent event, std::function<void()> callback);
        void TriggerEvent(GameEvent event);
        
        // مدیریت وضعیت بازی
        void Pause();
        void Resume();
        void Stop();
        
        // تنظیمات
        void SetGraphicsSettings(const GraphicsSettings& settings);
        void SetAudioSettings(const AudioSettings& settings);
        void SetPhysicsSettings(const PhysicsSettings& settings);
        
        const GraphicsSettings& GetGraphicsSettings() const { return graphicsSettings_; }
        const AudioSettings& GetAudioSettings() const { return audioSettings_; }
        const PhysicsSettings& GetPhysicsSettings() const { return physicsSettings_; }
        
        // اطلاعات وضعیت
        bool IsRunning() const { return isRunning_; }
        bool IsPaused() const { return isPaused_; }
        float GetDeltaTime() const { return deltaTime_; }
        float GetFPS() const { return fps_; }
        SDL_Window* GetWindow() const { return window_; }
        
        // متدهای مجازی برای پیاده‌سازی در کلاس‌های مشتق
        virtual void OnInitialize() = 0;
        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnRender() = 0;
        virtual void OnEvent(const SDL_Event& event) = 0;
        virtual void OnCleanup() = 0;

    protected:
        // متدهای داخلی
        bool InitializeSDL();
        bool InitializeOpenGL();
        bool InitializeSubsystems();
        void CalculateDeltaTime();
        void HandleEvents();
        void UpdateFrameStats();
        
        // مدیریت خطاها
        void LogError(const std::string& message) const;
        void LogInfo(const std::string& message) const;
    };

    // کلاس مدیریت زمان برای انیمیشن‌های سینمایی
    class TimeManager {
    private:
        static TimeManager* instance_;
        
        float gameTime_;
        float deltaTime_;
        float timeScale_;
        bool isSlowMotion_;
        float slowMotionFactor_;
        
        TimeManager();

    public:
        static TimeManager& GetInstance();
        static void DestroyInstance();
        
        void Update(float dt);
        void SetTimeScale(float scale);
        void SetSlowMotion(bool enable, float factor = 0.1f);
        
        float GetGameTime() const { return gameTime_; }
        float GetDeltaTime() const { return deltaTime_ * timeScale_; }
        float GetUnscaledDeltaTime() const { return deltaTime_; }
        float GetTimeScale() const { return timeScale_; }
        bool IsSlowMotion() const { return isSlowMotion_; }
    };

    // کلاس مدیریت دارایی‌ها
    class AssetManager {
    private:
        static AssetManager* instance_;
        
        std::unordered_map<std::string, class Texture*> textures_;
        std::unordered_map<std::string, class Shader*> shaders_;
        std::unordered_map<std::string, class Model3D*> models_;
        std::unordered_map<std::string, class Sound*> sounds_;
        std::unordered_map<std::string, class Font*> fonts_;
        
        std::string assetsBasePath_;

        AssetManager();

    public:
        static AssetManager& GetInstance();
        static void DestroyInstance();
        
        bool Initialize(const std::string& basePath = "Assets/");
        
        // مدیریت بافت‌ها
        class Texture* LoadTexture(const std::string& name, const std::string& filePath);
        class Texture* GetTexture(const std::string& name);
        void UnloadTexture(const std::string& name);
        
        // مدیریت شیدرها
        class Shader* LoadShader(const std::string& name, 
                                const std::string& vertexPath, 
                                const std::string& fragmentPath);
        class Shader* GetShader(const std::string& name);
        void UnloadShader(const std::string& name);
        
        // مدیریت مدل‌های سه بعدی
        class Model3D* LoadModel(const std::string& name, const std::string& filePath);
        class Model3D* GetModel(const std::string& name);
        void UnloadModel(const std::string& name);
        
        // مدیریت صداها
        class Sound* LoadSound(const std::string& name, const std::string& filePath);
        class Sound* GetSound(const std::string& name);
        void UnloadSound(const std::string& name);
        
        // مدیریت فونت‌ها
        class Font* LoadFont(const std::string& name, const std::string& filePath, int size);
        class Font* GetFont(const std::string& name);
        void UnloadFont(const std::string& name);
        
        void Cleanup();
        
        const std::string& GetAssetsBasePath() const { return assetsBasePath_; }
    };

    // کلاس مدیریت صحنه برای رندر سینمایی
    class SceneManager {
    private:
        static SceneManager* instance_;
        
        class Scene* currentScene_;
        class Scene* nextScene_;
        bool isTransitioning_;
        float transitionTime_;
        float transitionDuration_;
        std::function<void()> onTransitionComplete_;
        
        SceneManager();

    public:
        static SceneManager& GetInstance();
        static void DestroyInstance();
        
        void Update(float deltaTime);
        void Render();
        
        void LoadScene(class Scene* scene, float transitionDuration = 1.0f);
        void LoadSceneImmediate(class Scene* scene);
        
        class Scene* GetCurrentScene() const { return currentScene_; }
        bool IsTransitioning() const { return isTransitioning_; }
        
        void Cleanup();
    };

    // کلاس پایه برای تمام صحنه‌های بازی
    class Scene {
    protected:
        std::string name_;
        bool isActive_;
        bool isInitialized_;
        
    public:
        Scene(const std::string& name) : name_(name), isActive_(false), isInitialized_(false) {}
        virtual ~Scene() = default;
        
        virtual bool Initialize() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Render() = 0;
        virtual void Cleanup() = 0;
        
        virtual void OnEnter() {}
        virtual void OnExit() {}
        virtual void OnPause() {}
        virtual void OnResume() {}
        
        const std::string& GetName() const { return name_; }
        bool IsActive() const { return isActive_; }
        bool IsInitialized() const { return isInitialized_; }
        
        void SetActive(bool active) { isActive_ = active; }
    };

    // کلاس مدیریت دوربین برای حرکات سینمایی
    class CameraManager {
    private:
        static CameraManager* instance_;
        
        class Camera* activeCamera_;
        class Camera* cinematicCamera_;
        bool isCinematicMode_;
        float cinematicBlendFactor_;
        float cinematicBlendDuration_;
        float currentBlendTime_;
        
        CameraManager();

    public:
        static CameraManager& GetInstance();
        static void DestroyInstance();
        
        void Update(float deltaTime);
        
        void SetActiveCamera(class Camera* camera);
        void StartCinematicShot(class Camera* cinematicCamera, float duration = 3.0f);
        void EndCinematicShot();
        
        class Camera* GetActiveCamera() const;
        bool IsCinematicMode() const { return isCinematicMode_; }
        
        class Camera* CreateCamera(const std::string& name);
        void DestroyCamera(const std::string& name);
    };

} // namespace GalacticOdyssey

#endif // GAME_ENGINE_H
