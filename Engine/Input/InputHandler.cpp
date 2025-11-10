#include "InputHandler.h"
#include <iostream>
#include <algorithm>

namespace GalacticOdyssey {

    // پیاده‌سازی InputHandler (Singleton)
    InputHandler* InputHandler::instance_ = nullptr;

    InputHandler::InputHandler()
        : mouseSensitivity_(1.0f), mouseCaptured_(false), inputEnabled_(true),
          keyPressCount_(0), mouseClickCount_(0), gamepadEventCount_(0)
    {
        std::cout << "🎮 ایجاد مدیریت‌کننده ورودی" << std::endl;
        
        // مقداردهی اولیه وضعیت کلیدها
        for (int key = static_cast<int>(KeyCode::A); key <= static_cast<int>(KeyCode::Z); key++) {
            keyboardState_[static_cast<KeyCode>(key)] = ButtonState::RELEASED;
        }
        
        // مقداردهی اولیه وضعیت دکمه‌های ماوس
        for (int button = static_cast<int>(MouseButton::LEFT); button <= static_cast<int>(MouseButton::X2); button++) {
            mouseState_[static_cast<MouseButton>(button)] = ButtonState::RELEASED;
        }
    }

    InputHandler::~InputHandler()
    {
        Cleanup();
    }

    InputHandler& InputHandler::GetInstance()
    {
        if (!instance_) {
            instance_ = new InputHandler();
        }
        return *instance_;
    }

    void InputHandler::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool InputHandler::Initialize()
    {
        std::cout << "🔧 در حال راه‌اندازی سیستم ورودی..." << std::endl;
        
        // راه‌اندازی SDL برای گیم پد
        if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
            std::cerr << "❌ خطا در راه‌اندازی SDL GameController: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // فعال کردن رویدادهای لمسی
        SDL_EventState(SDL_FINGERDOWN, SDL_ENABLE);
        SDL_EventState(SDL_FINGERUP, SDL_ENABLE);
        SDL_EventState(SDL_FINGERMOTION, SDL_ENABLE);
        
        // ایجاد سیستم‌های کمکی
        inputMapping_ = std::make_unique<InputMapping>();
        virtualInput_ = std::make_unique<VirtualInputSystem>();
        inputBuffer_ = std::make_unique<InputBuffer>();
        comboSystem_ = std::make_unique<InputComboSystem>(inputBuffer_.get());
        gestureRecognizer_ = std::make_unique<TouchGestureRecognizer>();
        inputCombiner_ = std::make_unique<InputCombiner>(inputMapping_.get());
        
        // بارگذاری نگاشت‌های پیش‌فرض
        InitializeDefaultMappings();
        
        // راه‌اندازی کنترل‌های مجازی
        virtualInput_->Initialize();
        
        std::cout << "✅ سیستم ورودی با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void InputHandler::Cleanup()
    {
        std::cout << "🧹 پاکسازی سیستم ورودی..." << std::endl;
        
        gamepads_.clear();
        touchInputs_.clear();
        
        comboSystem_.reset();
        inputBuffer_.reset();
        inputCombiner_.reset();
        gestureRecognizer_.reset();
        virtualInput_.reset();
        inputMapping_.reset();
        
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        
        std::cout << "✅ سیستم ورودی پاکسازی شد" << std::endl;
    }

    void InputHandler::Update(float deltaTime)
    {
        if (!inputEnabled_) return;
        
        // به‌روزرسانی وضعیت کلیدها
        UpdateKeyboardState();
        UpdateMouseState();
        UpdateGamepads();
        
        // به‌روزرسانی سیستم‌های کمکی
        virtualInput_->Update();
        inputBuffer_->Update(static_cast<Uint32>(deltaTime * 1000));
        comboSystem_->Update();
        
        // ریست کردن دلتای ماوس برای فریم بعدی
        mouseDelta_ = glm::vec2(0.0f);
        mouseWheel_ = glm::vec2(0.0f);
    }

    void InputHandler::ProcessEvent(const SDL_Event& event)
    {
        if (!inputEnabled_) return;
        
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                ProcessKeyboardEvent(event);
                break;
                
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                ProcessMouseEvent(event);
                break;
                
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERAXISMOTION:
                ProcessGamepadEvent(event);
                break;
                
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
                ProcessTouchEvent(event);
                break;
                
            case SDL_JOYAXISMOTION:
            case SDL_JOYBALLMOTION:
            case SDL_JOYHATMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // پشتیبانی از جویستیک قدیمی
                break;
        }
    }

    void InputHandler::ProcessKeyboardEvent(const SDL_Event& event)
    {
        KeyCode key = static_cast<KeyCode>(event.key.keysym.sym);
        
        if (event.type == SDL_KEYDOWN) {
            if (keyboardState_[key] != ButtonState::PRESSED) {
                keyboardState_[key] = ButtonState::JUST_PRESSED;
                keyPressCount_++;
                
                // ثبت در بافر ورودی
                inputBuffer_->RecordInput(SDL_GetKeyName(event.key.keysym.sym));
            }
        } else if (event.type == SDL_KEYUP) {
            keyboardState_[key] = ButtonState::JUST_RELEASED;
        }
    }

    void InputHandler::ProcessMouseEvent(const SDL_Event& event)
    {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                mousePosition_.x = static_cast<float>(event.motion.x);
                mousePosition_.y = static_cast<float>(event.motion.y);
                mouseDelta_.x += static_cast<float>(event.motion.xrel) * mouseSensitivity_;
                mouseDelta_.y += static_cast<float>(event.motion.yrel) * mouseSensitivity_;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                MouseButton button = static_cast<MouseButton>(event.button.button);
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (mouseState_[button] != ButtonState::PRESSED) {
                        mouseState_[button] = ButtonState::JUST_PRESSED;
                        mouseClickCount_++;
                    }
                } else {
                    mouseState_[button] = ButtonState::JUST_RELEASED;
                }
                break;
            }
                
            case SDL_MOUSEWHEEL:
                mouseWheel_.x = static_cast<float>(event.wheel.x);
                mouseWheel_.y = static_cast<float>(event.wheel.y);
                break;
        }
    }

    void InputHandler::ProcessGamepadEvent(const SDL_Event& event)
    {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEADDED:
                AddGamepad(event.cdevice.which);
                break;
                
            case SDL_CONTROLLERDEVICEREMOVED:
                RemoveGamepad(event.cdevice.which);
                break;
                
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERAXISMOTION: {
                for (auto& gamepad : gamepads_) {
                    if (gamepad->GetInstanceId() == event.cbutton.which || 
                        gamepad->GetInstanceId() == event.caxis.which) {
                        gamepad->ProcessEvent(event);
                        gamepadEventCount_++;
                    }
                }
                break;
            }
        }
    }

    void InputHandler::ProcessTouchEvent(const SDL_Event& event)
    {
        TouchInput touch;
        touch.fingerId = event.tfinger.fingerId;
        touch.position.x = event.tfinger.x;
        touch.position.y = event.tfinger.y;
        touch.delta.x = event.tfinger.dx;
        touch.delta.y = event.tfinger.dy;
        touch.pressure = event.tfinger.pressure;
        
        switch (event.type) {
            case SDL_FINGERDOWN:
                touchInputs_.push_back(touch);
                gestureRecognizer_->ProcessTouch(touch);
                virtualInput_->ProcessTouchEvent(event);
                break;
                
            case SDL_FINGERUP:
                touchInputs_.erase(
                    std::remove_if(touchInputs_.begin(), touchInputs_.end(),
                        [&](const TouchInput& t) { return t.fingerId == touch.fingerId; }),
                    touchInputs_.end()
                );
                gestureRecognizer_->ProcessTouch(touch);
                virtualInput_->ProcessTouchEvent(event);
                break;
                
            case SDL_FINGERMOTION:
                for (auto& existingTouch : touchInputs_) {
                    if (existingTouch.fingerId == touch.fingerId) {
                        existingTouch.position = touch.position;
                        existingTouch.delta = touch.delta;
                        gestureRecognizer_->ProcessTouch(existingTouch);
                        break;
                    }
                }
                virtualInput_->ProcessTouchEvent(event);
                break;
        }
    }

    void InputHandler::AddGamepad(int deviceIndex)
    {
        auto gamepad = std::make_unique<Gamepad>(deviceIndex);
        if (gamepad->Initialize()) {
            gamepads_.push_back(std::move(gamepad));
            std::cout << "🎮 گیم پد متصل شد: " << gamepads_.back()->GetName() << std::endl;
        }
    }

    void InputHandler::RemoveGamepad(int instanceId)
    {
        auto it = std::find_if(gamepads_.begin(), gamepads_.end(),
            [&](const std::unique_ptr<Gamepad>& gp) { return gp->GetInstanceId() == instanceId; });
        
        if (it != gamepads_.end()) {
            std::cout << "🎮 گیم پد قطع شد: " << (*it)->GetName() << std::endl;
            gamepads_.erase(it);
        }
    }

    void InputHandler::UpdateKeyboardState()
    {
        for (auto& keyState : keyboardState_) {
            if (keyState.second == ButtonState::JUST_PRESSED) {
                keyState.second = ButtonState::PRESSED;
            } else if (keyState.second == ButtonState::JUST_RELEASED) {
                keyState.second = ButtonState::RELEASED;
            }
        }
    }

    void InputHandler::UpdateMouseState()
    {
        for (auto& buttonState : mouseState_) {
            if (buttonState.second == ButtonState::JUST_PRESSED) {
                buttonState.second = ButtonState::PRESSED;
            } else if (buttonState.second == ButtonState::JUST_RELEASED) {
                buttonState.second = ButtonState::RELEASED;
            }
        }
    }

    void InputHandler::UpdateGamepads()
    {
        for (auto& gamepad : gamepads_) {
            gamepad->Update();
        }
    }

    void InputHandler::InitializeDefaultMappings()
    {
        // نگاشت‌های پیش‌فرض برای حرکت
        inputMapping_->AddAxis("MoveHorizontal");
        inputMapping_->BindAxisToKeys("MoveHorizontal", KeyCode::D, KeyCode::A);
        inputMapping_->BindAxisToGamepad("MoveHorizontal", InputAxis::LEFT_STICK_X);
        
        inputMapping_->AddAxis("MoveVertical");
        inputMapping_->BindAxisToKeys("MoveVertical", KeyCode::W, KeyCode::S);
        inputMapping_->BindAxisToGamepad("MoveVertical", InputAxis::LEFT_STICK_Y);
        
        // نگاشت‌های پیش‌فرض برای نگاه کردن
        inputMapping_->AddAxis("LookHorizontal");
        inputMapping_->BindAxisToGamepad("LookHorizontal", InputAxis::RIGHT_STICK_X);
        
        inputMapping_->AddAxis("LookVertical");
        inputMapping_->BindAxisToGamepad("LookVertical", InputAxis::RIGHT_STICK_Y);
        
        // نگاشت‌های پیش‌فرض برای actions
        inputMapping_->AddAction("Jump");
        inputMapping_->BindKeyToAction("Jump", KeyCode::SPACE);
        inputMapping_->BindGamepadButtonToAction("Jump", GamepadButton::A);
        
        inputMapping_->AddAction("Attack");
        inputMapping_->BindMouseButtonToAction("Attack", MouseButton::LEFT);
        inputMapping_->BindGamepadButtonToAction("Attack", GamepadButton::X);
        
        inputMapping_->AddAction("Special");
        inputMapping_->BindKeyToAction("Special", KeyCode::E);
        inputMapping_->BindGamepadButtonToAction("Special", GamepadButton::Y);
        
        std::cout << "✅ نگاشت‌های ورودی پیش‌فرض بارگذاری شدند" << std::endl;
    }

    ButtonState InputHandler::GetKeyState(KeyCode key) const
    {
        auto it = keyboardState_.find(key);
        return it != keyboardState_.end() ? it->second : ButtonState::RELEASED;
    }

    bool InputHandler::IsKeyPressed(KeyCode key) const
    {
        ButtonState state = GetKeyState(key);
        return state == ButtonState::PRESSED || state == ButtonState::JUST_PRESSED;
    }

    bool InputHandler::IsKeyJustPressed(KeyCode key) const
    {
        return GetKeyState(key) == ButtonState::JUST_PRESSED;
    }

    bool InputHandler::IsKeyJustReleased(KeyCode key) const
    {
        return GetKeyState(key) == ButtonState::JUST_RELEASED;
    }

    ButtonState InputHandler::GetMouseButtonState(MouseButton button) const
    {
        auto it = mouseState_.find(button);
        return it != mouseState_.end() ? it->second : ButtonState::RELEASED;
    }

    bool InputHandler::IsMouseButtonPressed(MouseButton button) const
    {
        ButtonState state = GetMouseButtonState(button);
        return state == ButtonState::PRESSED || state == ButtonState::JUST_PRESSED;
    }

    bool InputHandler::IsMouseButtonJustPressed(MouseButton button) const
    {
        return GetMouseButtonState(button) == ButtonState::JUST_PRESSED;
    }

    bool InputHandler::IsMouseButtonJustReleased(MouseButton button) const
    {
        return GetMouseButtonState(button) == ButtonState::JUST_RELEASED;
    }

    Gamepad* InputHandler::GetGamepad(int index) const
    {
        if (index >= 0 && index < static_cast<int>(gamepads_.size())) {
            return gamepads_[index].get();
        }
        return nullptr;
    }

    bool InputHandler::IsGamepadConnected(int index) const
    {
        Gamepad* gamepad = GetGamepad(index);
        return gamepad && gamepad->IsConnected();
    }

    void InputHandler::SetMouseCaptured(bool captured)
    {
        mouseCaptured_ = captured;
        SDL_SetRelativeMouseMode(captured ? SDL_TRUE : SDL_FALSE);
        std::cout << (captured ? "🔒 ماوس captured شد" : "🔓 ماوس released شد") << std::endl;
    }

    bool InputHandler::GetAction(const std::string& actionName) const
    {
        return inputMapping_ ? inputMapping_->GetAction(actionName) : false;
    }

    float InputHandler::GetAxis(const std::string& axisName) const
    {
        return inputMapping_ ? inputMapping_->GetAxis(axisName) : 0.0f;
    }

    glm::vec2 InputHandler::GetVirtualStick(const std::string& name) const
    {
        return virtualInput_ ? virtualInput_->GetStickDirection(name) : glm::vec2(0.0f);
    }

    bool InputHandler::GetVirtualButton(const std::string& name) const
    {
        return virtualInput_ ? virtualInput_->IsButtonPressed(name) : false;
    }

    // پیاده‌سازی Gamepad
    Gamepad::Gamepad(int deviceIndex)
        : controller_(nullptr), joystick_(nullptr), instanceId_(-1),
          connected_(false), hasRumble_(false), rumbleStrength_(0)
    {
        controller_ = SDL_GameControllerOpen(deviceIndex);
        if (controller_) {
            joystick_ = SDL_GameControllerGetJoystick(controller_);
            instanceId_ = SDL_JoystickInstanceID(joystick_);
            name_ = SDL_GameControllerName(controller_);
            connected_ = true;
            
            // بررسی پشتیبانی از لرزش
            hasRumble_ = SDL_GameControllerHasRumble(controller_);
        }
    }

    Gamepad::~Gamepad()
    {
        if (controller_) {
            SDL_GameControllerClose(controller_);
        }
    }

    bool Gamepad::Initialize()
    {
        return controller_ != nullptr;
    }

    void Gamepad::Update()
    {
        UpdateButtonStates();
        UpdateAxisValues();
    }

    void Gamepad::ProcessEvent(const SDL_Event& event)
    {
        // رویدادها در Update پردازش می‌شوند
    }

    void Gamepad::UpdateButtonStates()
    {
        // به‌روزرسانی وضعیت تمام دکمه‌ها
        for (int button = SDL_CONTROLLER_BUTTON_A; button < SDL_CONTROLLER_BUTTON_MAX; button++) {
            GamepadButton gamepadButton = static_cast<GamepadButton>(button);
            bool isPressed = SDL_GameControllerGetButton(controller_, static_cast<SDL_GameControllerButton>(button));
            
            ButtonState currentState = buttonStates_[gamepadButton];
            
            if (isPressed) {
                buttonStates_[gamepadButton] = (currentState == ButtonState::RELEASED || 
                                               currentState == ButtonState::JUST_RELEASED) 
                                               ? ButtonState::JUST_PRESSED : ButtonState::PRESSED;
            } else {
                buttonStates_[gamepadButton] = (currentState == ButtonState::PRESSED || 
                                               currentState == ButtonState::JUST_PRESSED) 
                                               ? ButtonState::JUST_RELEASED : ButtonState::RELEASED;
            }
        }
    }

    void Gamepad::UpdateAxisValues()
    {
        // محورهای استیک چپ
        axisValues_[InputAxis::LEFT_STICK_X] = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
        axisValues_[InputAxis::LEFT_STICK_Y] = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
        
        // محورهای استیک راست
        axisValues_[InputAxis::RIGHT_STICK_X] = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
        axisValues_[InputAxis::RIGHT_STICK_Y] = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
        
        // ماشه‌ها
        axisValues_[InputAxis::LEFT_TRIGGER] = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
        axisValues_[InputAxis::RIGHT_TRIGGER] = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
        
        // اعمال deadzone
        const float deadzone = 0.1f;
        for (auto& axis : axisValues_) {
            if (std::abs(axis.second) < deadzone) {
                axis.second = 0.0f;
            }
        }
    }

    ButtonState Gamepad::GetButtonState(GamepadButton button) const
    {
        auto it = buttonStates_.find(button);
        return it != buttonStates_.end() ? it->second : ButtonState::RELEASED;
    }

    bool Gamepad::IsButtonPressed(GamepadButton button) const
    {
        ButtonState state = GetButtonState(button);
        return state == ButtonState::PRESSED || state == ButtonState::JUST_PRESSED;
    }

    bool Gamepad::IsButtonJustPressed(GamepadButton button) const
    {
        return GetButtonState(button) == ButtonState::JUST_PRESSED;
    }

    bool Gamepad::IsButtonJustReleased(GamepadButton button) const
    {
        return GetButtonState(button) == ButtonState::JUST_RELEASED;
    }

    float Gamepad::GetAxisValue(InputAxis axis) const
    {
        auto it = axisValues_.find(axis);
        return it != axisValues_.end() ? it->second : 0.0f;
    }

    glm::vec2 Gamepad::GetLeftStick() const
    {
        return glm::vec2(GetAxisValue(InputAxis::LEFT_STICK_X), 
                        GetAxisValue(InputAxis::LEFT_STICK_Y));
    }

    glm::vec2 Gamepad::GetRightStick() const
    {
        return glm::vec2(GetAxisValue(InputAxis::RIGHT_STICK_X), 
                        GetAxisValue(InputAxis::RIGHT_STICK_Y));
    }

    float Gamepad::GetLeftTrigger() const
    {
        return GetAxisValue(InputAxis::LEFT_TRIGGER);
    }

    float Gamepad::GetRightTrigger() const
    {
        return GetAxisValue(InputAxis::RIGHT_TRIGGER);
    }

    void Gamepad::SetRumble(float strength, Uint32 duration)
    {
        if (hasRumble_) {
            rumbleStrength_ = static_cast<Uint16>(strength * 0xFFFF);
            SDL_GameControllerRumble(controller_, rumbleStrength_, rumbleStrength_, duration);
        }
    }

    // پیاده‌سازی VirtualInputSystem
    VirtualInputSystem::VirtualInputSystem()
        : stickDeadZone_(0.1f), buttonRadius_(50.0f) {}

    void VirtualInputSystem::Initialize()
    {
        // ایجاد کنترل‌های مجازی پیش‌فرض برای موبایل
        AddVirtualStick("Movement", glm::vec2(150.0f, 600.0f), 80.0f);
        AddVirtualButton("Action", glm::vec2(850.0f, 600.0f), 60.0f);
        AddVirtualButton("Special", glm::vec2(750.0f, 500.0f), 50.0f);
    }

    void VirtualInputSystem::Update()
    {
        UpdateVirtualControls();
    }

    void VirtualInputSystem::AddVirtualStick(const std::string& name, const glm::vec2& center, float radius)
    {
        virtualControls_[name] = std::make_unique<VirtualControl>(name, center, radius);
    }

    glm::vec2 VirtualInputSystem::GetStickDirection(const std::string& name) const
    {
        auto it = virtualControls_.find(name);
        if (it != virtualControls_.end() && it->second->isActive) {
            glm::vec2 direction = it->second->normalizedPosition;
            
            // اعمال deadzone
            if (glm::length(direction) < stickDeadZone_) {
                return glm::vec2(0.0f);
            }
            
            return direction;
        }
        return glm::vec2(0.0f);
    }

    bool VirtualInputSystem::IsButtonPressed(const std::string& name) const
    {
        auto it = virtualControls_.find(name);
        return it != virtualControls_.end() && it->second->isActive;
    }

    // پیاده‌سازی InputMapping
    InputMapping::InputMapping() {}

    bool InputMapping::GetAction(const std::string& actionName) const
    {
        auto it = actionMappings_.find(actionName);
        return it != actionMappings_.end() ? EvaluateAction(it->second) : false;
    }

    float InputMapping::GetAxis(const std::string& axisName) const
    {
        auto it = axisMappings_.find(axisName);
        return it != axisMappings_.end() ? EvaluateAxis(it->second) : 0.0f;
    }

    // پیاده‌سازی InputBuffer
    InputBuffer::InputBuffer(Uint32 duration)
        : bufferDuration_(duration), currentTime_(0) {}

    void InputBuffer::Update(Uint32 deltaTime)
    {
        currentTime_ += deltaTime;
        RemoveExpiredInputs();
    }

    void InputBuffer::RecordInput(const std::string& actionName, float value)
    {
        buffer_.emplace_back(actionName, currentTime_, value);
    }

} // namespace GalacticOdyssey
