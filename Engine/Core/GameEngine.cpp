#include "GameEngine.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Physics/PhysicsEngine.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Input/InputHandler.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace GalacticOdyssey {

    // پیاده‌سازی GameEngine
    GameEngine::GameEngine(int width, int height, const std::string& title)
        : window_(nullptr)
        , glContext_(nullptr)
        , isInitialized_(false)
        , isRunning_(false)
        , isPaused_(false)
        , gameTitle_(title)
        , lastFrameTime_(0)
        , currentFrameTime_(0)
        , deltaTime_(0.0f)
        , fps_(0.0f)
        , frameCount_(0)
        , fpsTimer_(0)
        , renderSystem_(nullptr)
        , physicsEngine_(nullptr)
        , audioManager_(nullptr)
        , inputHandler_(nullptr)
    {
        graphicsSettings_.screenWidth = width;
        graphicsSettings_.screenHeight = height;
        
        std::cout << "🎮 ایجاد موتور بازی: " << gameTitle_ << std::endl;
    }

    GameEngine::~GameEngine()
    {
        Cleanup();
    }

    bool GameEngine::Initialize()
    {
        std::cout << "🔧 در حال راه‌اندازی موتور بازی..." << std::endl;
        
        // راه‌اندازی SDL
        if (!InitializeSDL()) {
            LogError("خطا در راه‌اندازی SDL");
            return false;
        }
        
        // راه‌اندازی OpenGL
        if (!InitializeOpenGL()) {
            LogError("خطا در راه‌اندازی OpenGL");
            return false;
        }
        
        // راه‌اندازی سیستم‌های وابسته
        if (!InitializeSubsystems()) {
            LogError("خطا در راه‌اندازی سیستم‌های وابسته");
            return false;
        }
        
        // راه‌اندازی مدیریت دارایی‌ها
        if (!AssetManager::GetInstance().Initialize()) {
            LogError("خطا در راه‌اندازی مدیریت دارایی‌ها");
            return false;
        }
        
        // فراخوانی متد مقداردهی اولیه مشتق
        try {
            OnInitialize();
        }
        catch (const std::exception& e) {
            LogError(std::string("خطا در OnInitialize: ") + e.what());
            return false;
        }
        
        isInitialized_ = true;
        isRunning_ = true;
        
        std::cout << "✅ موتور بازی با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    bool GameEngine::InitializeSDL()
    {
        std::cout << "🖥️ در حال راه‌اندازی SDL..." << std::endl;
        
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
            LogError(std::string("خطا در مقداردهی اولیه SDL: ") + SDL_GetError());
            return false;
        }
        
        // تنظیمات OpenGL
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, graphicsSettings_.msaaSamples);
        
        // ایجاد پنجره
        Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if (graphicsSettings_.fullscreen) {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        
        window_ = SDL_CreateWindow(
            gameTitle_.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            graphicsSettings_.screenWidth,
            graphicsSettings_.screenHeight,
            windowFlags
        );
        
        if (!window_) {
            LogError(std::string("خطا در ایجاد پنجره SDL: ") + SDL_GetError());
            return false;
        }
        
        std::cout << "✅ SDL با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    bool GameEngine::InitializeOpenGL()
    {
        std::cout << "🎨 در حال راه‌اندازی OpenGL..." << std::endl;
        
        // ایجاد زمینه OpenGL
        glContext_ = SDL_GL_CreateContext(window_);
        if (!glContext_) {
            LogError(std::string("خطا در ایجاد زمینه OpenGL: ") + SDL_GetError());
            return false;
        }
        
        // راه‌اندازی GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            LogError(std::string("خطا در راه‌اندازی GLEW: ") + 
                    reinterpret_cast<const char*>(glewGetErrorString(glewError)));
            return false;
        }
        
        // تنظیم VSync
        SDL_GL_SetSwapInterval(graphicsSettings_.vsync ? 1 : 0);
        
        // تنظیمات اولیه OpenGL
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        if (graphicsSettings_.msaaSamples > 0) {
            glEnable(GL_MULTISAMPLE);
        }
        
        // چاپ اطلاعات کارت گرافیک
        std::cout << "🖥️ کارت گرافیک: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "🔢 نسخه OpenGL: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "🔤 نسخه GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        
        std::cout << "✅ OpenGL با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    bool GameEngine::InitializeSubsystems()
    {
        std::cout << "⚙️ در حال راه‌اندازی سیستم‌های وابسته..." << std::endl;
        
        try {
            // ایجاد سیستم رندر
            renderSystem_ = new RenderSystem(this);
            if (!renderSystem_->Initialize()) {
                LogError("خطا در راه‌اندازی سیستم رندر");
                return false;
            }
            
            // ایجاد موتور فیزیک
            physicsEngine_ = new PhysicsEngine();
            if (!physicsEngine_->Initialize()) {
                LogError("خطا در راه‌اندازی موتور فیزیک");
                return false;
            }
            
            // ایجاد مدیریت صدا
            audioManager_ = new AudioManager();
            if (!audioManager_->Initialize()) {
                LogError("خطا در راه‌اندازی مدیریت صدا");
                return false;
            }
            
            // ایجاد مدیریت ورودی
            inputHandler_ = new InputHandler();
            if (!inputHandler_->Initialize()) {
                LogError("خطا در راه‌اندازی مدیریت ورودی");
                return false;
            }
            
            // تنظیم تنظیمات صوتی
            audioManager_->SetMasterVolume(audioSettings_.masterVolume);
            audioManager_->SetMusicVolume(audioSettings_.musicVolume);
            audioManager_->SetEffectsVolume(audioSettings_.effectsVolume);
            
            // تنظیم تنظیمات فیزیک
            physicsEngine_->SetGravity(physicsSettings_.gravity);
            physicsEngine_->SetAirResistance(physicsSettings_.airResistance);
            physicsEngine_->SetTimeScale(physicsSettings_.timeScale);
            
        }
        catch (const std::exception& e) {
            LogError(std::string("خطا در ایجاد سیستم‌ها: ") + e.what());
            return false;
        }
        
        std::cout << "✅ سیستم‌های وابسته با موفقیت راه‌اندازی شدند" << std::endl;
        return true;
    }

    void GameEngine::Run()
    {
        if (!isInitialized_) {
            LogError("موتور بازی راه‌اندازی نشده است");
            return;
        }
        
        std::cout << "🚀 شروع حلقه اصلی بازی..." << std::endl;
        
        lastFrameTime_ = SDL_GetTicks();
        fpsTimer_ = lastFrameTime_;
        
        // حلقه اصلی بازی
        while (isRunning_) {
            currentFrameTime_ = SDL_GetTicks();
            CalculateDeltaTime();
            
            // مدیریت رویدادها
            HandleEvents();
            
            if (!isPaused_) {
                // به‌روزرسانی سیستم‌ها
                TimeManager::GetInstance().Update(deltaTime_);
                SceneManager::GetInstance().Update(deltaTime_);
                CameraManager::GetInstance().Update(deltaTime_);
                
                // به‌روزرسانی ورودی
                inputHandler_->Update();
                
                // به‌روزرسانی فیزیک
                physicsEngine_->Update(deltaTime_);
                
                // به‌روزرسانی صدا
                audioManager_->Update();
                
                // فراخوانی به‌روزرسانی بازی
                OnUpdate(deltaTime_);
                
                // رندر
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                OnRender();
                SceneManager::GetInstance().Render();
                
                // تعویض بافر
                SDL_GL_SwapWindow(window_);
            }
            
            // آمار فریم
            UpdateFrameStats();
            
            // خواب برای کنترل FPS
            Uint32 frameTime = SDL_GetTicks() - currentFrameTime_;
            Uint32 minFrameTime = 1000 / 144; // 144 FPS
            if (frameTime < minFrameTime) {
                SDL_Delay(minFrameTime - frameTime);
            }
        }
        
        std::cout << "🛑 حلقه اصلی بازی پایان یافت" << std::endl;
    }

    void GameEngine::CalculateDeltaTime()
    {
        deltaTime_ = (currentFrameTime_ - lastFrameTime_) / 1000.0f;
        lastFrameTime_ = currentFrameTime_;
        
        // محدود کردن deltaTime برای جلوگیری از Spike
        if (deltaTime_ > 0.1f) {
            deltaTime_ = 0.1f;
        }
    }

    void GameEngine::HandleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // مدیریت رویدادهای SDL
            switch (event.type) {
                case SDL_QUIT:
                    isRunning_ = false;
                    break;
                    
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        graphicsSettings_.screenWidth = event.window.data1;
                        graphicsSettings_.screenHeight = event.window.data2;
                        glViewport(0, 0, event.window.data1, event.window.data2);
                        renderSystem_->OnWindowResize(event.window.data1, event.window.data2);
                    }
                    break;
                    
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_F11) {
                        // تغییر حالت تمام صفحه
                        graphicsSettings_.fullscreen = !graphicsSettings_.fullscreen;
                        SDL_SetWindowFullscreen(window_, 
                            graphicsSettings_.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    }
                    break;
            }
            
            // انتقال رویداد به مدیریت ورودی
            inputHandler_->ProcessEvent(event);
            
            // فراخوانی رویداد بازی
            OnEvent(event);
        }
    }

    void GameEngine::UpdateFrameStats()
    {
        frameCount_++;
        
        if (currentFrameTime_ - fpsTimer_ >= 1000) {
            fps_ = frameCount_ * 1000.0f / (currentFrameTime_ - fpsTimer_);
            frameCount_ = 0;
            fpsTimer_ = currentFrameTime_;
            
            // به‌روزرسانی عنوان پنجره با FPS
            std::string titleWithFPS = gameTitle_ + " - FPS: " + std::to_string(static_cast<int>(fps_));
            SDL_SetWindowTitle(window_, titleWithFPS.c_str());
        }
    }

    void GameEngine::Cleanup()
    {
        std::cout << "🧹 در حال پاکسازی موتور بازی..." << std::endl;
        
        isRunning_ = false;
        
        // پاکسازی سیستم‌های وابسته
        if (inputHandler_) {
            inputHandler_->Cleanup();
            delete inputHandler_;
            inputHandler_ = nullptr;
        }
        
        if (audioManager_) {
            audioManager_->Cleanup();
            delete audioManager_;
            audioManager_ = nullptr;
        }
        
        if (physicsEngine_) {
            physicsEngine_->Cleanup();
            delete physicsEngine_;
            physicsEngine_ = nullptr;
        }
        
        if (renderSystem_) {
            renderSystem_->Cleanup();
            delete renderSystem_;
            renderSystem_ = nullptr;
        }
        
        // پاکسازی مدیریت‌کننده‌های singleton
        CameraManager::DestroyInstance();
        SceneManager::DestroyInstance();
        AssetManager::DestroyInstance();
        TimeManager::DestroyInstance();
        
        // پاکسازی OpenGL و SDL
        if (glContext_) {
            SDL_GL_DeleteContext(glContext_);
            glContext_ = nullptr;
        }
        
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        
        SDL_Quit();
        
        std::cout << "✅ موتور بازی پاکسازی شد" << std::endl;
    }

    void GameEngine::AddEventListener(GameEvent event, std::function<void()> callback)
    {
        eventListeners_[event].push_back(callback);
    }

    void GameEngine::RemoveEventListener(GameEvent event, std::function<void()> callback)
    {
        auto& listeners = eventListeners_[event];
        listeners.erase(
            std::remove_if(listeners.begin(), listeners.end(),
                [&](const std::function<void()>& func) {
                    return func.target_type() == callback.target_type();
                }),
            listeners.end()
        );
    }

    void GameEngine::TriggerEvent(GameEvent event)
    {
        auto it = eventListeners_.find(event);
        if (it != eventListeners_.end()) {
            for (auto& callback : it->second) {
                callback();
            }
        }
    }

    void GameEngine::Pause()
    {
        isPaused_ = true;
        audioManager_->PauseAll();
        TriggerEvent(GameEvent::GAME_PAUSE);
    }

    void GameEngine::Resume()
    {
        isPaused_ = false;
        audioManager_->ResumeAll();
        TriggerEvent(GameEvent::GAME_RESUME);
    }

    void GameEngine::Stop()
    {
        isRunning_ = false;
    }

    void GameEngine::SetGraphicsSettings(const GraphicsSettings& settings)
    {
        graphicsSettings_ = settings;
        if (renderSystem_) {
            renderSystem_->ApplySettings(settings);
        }
    }

    void GameEngine::SetAudioSettings(const AudioSettings& settings)
    {
        audioSettings_ = settings;
        if (audioManager_) {
            audioManager_->SetMasterVolume(settings.masterVolume);
            audioManager_->SetMusicVolume(settings.musicVolume);
            audioManager_->SetEffectsVolume(settings.effectsVolume);
        }
    }

    void GameEngine::SetPhysicsSettings(const PhysicsSettings& settings)
    {
        physicsSettings_ = settings;
        if (physicsEngine_) {
            physicsEngine_->SetGravity(settings.gravity);
            physicsEngine_->SetAirResistance(settings.airResistance);
            physicsEngine_->SetTimeScale(settings.timeScale);
        }
    }

    void GameEngine::LogError(const std::string& message) const
    {
        std::cerr << "❌ [ERROR] " << message << std::endl;
    }

    void GameEngine::LogInfo(const std::string& message) const
    {
        std::cout << "ℹ️ [INFO] " << message << std::endl;
    }

    // پیاده‌سازی TimeManager
    TimeManager* TimeManager::instance_ = nullptr;

    TimeManager::TimeManager()
        : gameTime_(0.0f)
        , deltaTime_(0.0f)
        , timeScale_(1.0f)
        , isSlowMotion_(false)
        , slowMotionFactor_(0.1f)
    {
    }

    TimeManager& TimeManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new TimeManager();
        }
        return *instance_;
    }

    void TimeManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    void TimeManager::Update(float dt)
    {
        deltaTime_ = dt;
        gameTime_ += dt * timeScale_;
    }

    void TimeManager::SetTimeScale(float scale)
    {
        timeScale_ = std::max(0.0f, scale);
    }

    void TimeManager::SetSlowMotion(bool enable, float factor)
    {
        isSlowMotion_ = enable;
        slowMotionFactor_ = factor;
        timeScale_ = enable ? factor : 1.0f;
    }

    // پیاده‌سازی اولیه سایر کلاس‌های Singleton
    // (پیاده‌سازی کامل در فایل‌های جداگانه خواهد آمد)

    AssetManager* AssetManager::instance_ = nullptr;
    SceneManager* SceneManager::instance_ = nullptr;
    CameraManager* CameraManager::instance_ = nullptr;

    AssetManager::AssetManager() : assetsBasePath_("Assets/") {}
    SceneManager::SceneManager() : currentScene_(nullptr), nextScene_(nullptr), 
                                 isTransitioning_(false), transitionTime_(0.0f), 
                                 transitionDuration_(1.0f) {}
    CameraManager::CameraManager() : activeCamera_(nullptr), cinematicCamera_(nullptr),
                                   isCinematicMode_(false), cinematicBlendFactor_(0.0f),
                                   cinematicBlendDuration_(3.0f), currentBlendTime_(0.0f) {}

    AssetManager& AssetManager::GetInstance() { 
        if (!instance_) instance_ = new AssetManager(); 
        return *instance_; 
    }
    
    SceneManager& SceneManager::GetInstance() { 
        if (!instance_) instance_ = new SceneManager(); 
        return *instance_; 
    }
    
    CameraManager& CameraManager::GetInstance() { 
        if (!instance_) instance_ = new CameraManager(); 
        return *instance_; 
    }

    void AssetManager::DestroyInstance() { if (instance_) delete instance_; instance_ = nullptr; }
    void SceneManager::DestroyInstance() { if (instance_) delete instance_; instance_ = nullptr; }
    void CameraManager::DestroyInstance() { if (instance_) delete instance_; instance_ = nullptr; }

    bool AssetManager::Initialize(const std::string& basePath) { 
        assetsBasePath_ = basePath; 
        return true; 
    }

    // پیاده‌سازی stub برای متدهای مدیریت دارایی
    // (پیاده‌سازی کامل در فایل AssetManager.cpp خواهد آمد)

} // namespace GalacticOdyssey
