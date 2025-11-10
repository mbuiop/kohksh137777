#pragma once
#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <array>

namespace GalacticOdyssey {

    // انواع دستگاه‌های ورودی
    enum class InputDevice {
        KEYBOARD,
        MOUSE,
        GAMEPAD,
        TOUCH,
        GYROSCOPE,
        ACCELEROMETER
    };

    // وضعیت دکمه‌ها
    enum class ButtonState {
        RELEASED,
        PRESSED,
        JUST_PRESSED,
        JUST_RELEASED
    };

    // محورهای کنترل
    enum class InputAxis {
        LEFT_STICK_X,
        LEFT_STICK_Y,
        RIGHT_STICK_X,
        RIGHT_STICK_Y,
        LEFT_TRIGGER,
        RIGHT_TRIGGER,
        MOUSE_X,
        MOUSE_Y,
        MOUSE_WHEEL,
        GYRO_X,
        GYRO_Y,
        GYRO_Z
    };

    // کدهای کلیدهای صفحه کلید
    enum class KeyCode {
        // حروف
        A = SDLK_a, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        
        // اعداد
        NUM_0 = SDLK_0, NUM_1, NUM_2, NUM_3, NUM_4,
        NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,
        
        // کلیدهای عملکرد
        F1 = SDLK_F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        
        // کلیدهای جهت
        UP = SDLK_UP,
        DOWN = SDLK_DOWN,
        LEFT = SDLK_LEFT,
        RIGHT = SDLK_RIGHT,
        
        // کلیدهای ویژه
        SPACE = SDLK_SPACE,
        RETURN = SDLK_RETURN,
        ESCAPE = SDLK_ESCAPE,
        TAB = SDLK_TAB,
        SHIFT = SDLK_LSHIFT,
        CTRL = SDLK_LCTRL,
        ALT = SDLK_LALT,
        BACKSPACE = SDLK_BACKSPACE,
        
        // کلیدهای بازی
        W = SDLK_w,
        A = SDLK_a,
        S = SDLK_s,
        D = SDLK_d,
        Q = SDLK_q,
        E = SDLK_e,
        R = SDLK_r,
        
        UNKNOWN = SDLK_UNKNOWN
    };

    // کدهای دکمه‌های ماوس
    enum class MouseButton {
        LEFT = SDL_BUTTON_LEFT,
        RIGHT = SDL_BUTTON_RIGHT,
        MIDDLE = SDL_BUTTON_MIDDLE,
        X1 = SDL_BUTTON_X1,
        X2 = SDL_BUTTON_X2
    };

    // کدهای دکمه‌های گیم پد
    enum class GamepadButton {
        A = SDL_CONTROLLER_BUTTON_A,
        B = SDL_CONTROLLER_BUTTON_B,
        X = SDL_CONTROLLER_BUTTON_X,
        Y = SDL_CONTROLLER_BUTTON_Y,
        BACK = SDL_CONTROLLER_BUTTON_BACK,
        GUIDE = SDL_CONTROLLER_BUTTON_GUIDE,
        START = SDL_CONTROLLER_BUTTON_START,
        LEFT_STICK = SDL_CONTROLLER_BUTTON_LEFTSTICK,
        RIGHT_STICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        LEFT_SHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        RIGHT_SHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
        DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
    };

    // ساختار ورودی لمسی
    struct TouchInput {
        int fingerId;
        glm::vec2 position;
        glm::vec2 delta;
        float pressure;
        bool isPrimary;
        
        TouchInput() 
            : fingerId(-1), position(0.0f), delta(0.0f), 
              pressure(1.0f), isPrimary(false) {}
    };

    // ساختار حرکت ژیروسکوپ
    struct GyroInput {
        glm::vec3 rotation;
        glm::vec3 acceleration;
        glm::vec3 rawAcceleration;
        
        GyroInput() : rotation(0.0f), acceleration(0.0f), rawAcceleration(0.0f) {}
    };

    // ساختار کنترل مجازی (مانند جویستیک روی صفحه)
    struct VirtualControl {
        std::string name;
        glm::vec2 center;
        float radius;
        bool isActive;
        glm::vec2 currentPosition;
        glm::vec2 normalizedPosition;
        
        VirtualControl(const std::string& name, const glm::vec2& center, float radius)
            : name(name), center(center), radius(radius), 
              isActive(false), currentPosition(0.0f), normalizedPosition(0.0f) {}
    };

    // کلاس مدیریت گیم پد
    class Gamepad {
    private:
        SDL_GameController* controller_;
        SDL_Joystick* joystick_;
        int instanceId_;
        std::string name_;
        bool connected_;
        
        // وضعیت دکمه‌ها
        std::unordered_map<GamepadButton, ButtonState> buttonStates_;
        
        // وضعیت محورها
        std::unordered_map<InputAxis, float> axisValues_;
        
        // لرزش
        bool hasRumble_;
        Uint16 rumbleStrength_;
        
    public:
        Gamepad(int deviceIndex);
        ~Gamepad();
        
        bool Initialize();
        void Update();
        void ProcessEvent(const SDL_Event& event);
        
        // وضعیت دکمه‌ها
        ButtonState GetButtonState(GamepadButton button) const;
        bool IsButtonPressed(GamepadButton button) const;
        bool IsButtonJustPressed(GamepadButton button) const;
        bool IsButtonJustReleased(GamepadButton button) const;
        
        // وضعیت محورها
        float GetAxisValue(InputAxis axis) const;
        glm::vec2 GetLeftStick() const;
        glm::vec2 GetRightStick() const;
        float GetLeftTrigger() const;
        float GetRightTrigger() const;
        
        // لرزش
        void SetRumble(float strength, Uint32 duration);
        void StopRumble();
        
        // اطلاعات
        bool IsConnected() const { return connected_; }
        const std::string& GetName() const { return name_; }
        int GetInstanceId() const { return instanceId_; }
        bool HasRumble() const { return hasRumble_; }
        
    private:
        void UpdateButtonStates();
        void UpdateAxisValues();
        SDL_GameControllerAxis ConvertAxis(InputAxis axis) const;
        SDL_GameControllerButton ConvertButton(GamepadButton button) const;
    };

    // کلاس مدیریت ژست‌های لمسی
    class TouchGestureRecognizer {
    private:
        struct GestureState {
            glm::vec2 startPosition;
            glm::vec2 currentPosition;
            Uint32 startTime;
            bool isActive;
            int touchCount;
            
            GestureState() 
                : startPosition(0.0f), currentPosition(0.0f),
                  startTime(0), isActive(false), touchCount(0) {}
        };
        
        GestureState currentGesture_;
        float swipeThreshold_;
        float tapThreshold_;
        float pinchThreshold_;
        
    public:
        TouchGestureRecognizer();
        ~TouchGestureRecognizer();
        
        void ProcessTouch(const TouchInput& touch);
        void Reset();
        
        // تشخیص ژست‌ها
        bool IsSwiping() const;
        bool IsPinching() const;
        bool IsTapping() const;
        bool IsDragging() const;
        
        // اطلاعات ژست
        glm::vec2 GetSwipeDirection() const;
        float GetSwipeDistance() const;
        float GetPinchScale() const;
        glm::vec2 GetDragDelta() const;
        
    private:
        void UpdateGesture(const TouchInput& touch);
        bool DetectSwipe();
        bool DetectTap();
        bool DetectPinch();
    };

    // سیستم نگاشت ورودی
    class InputMapping {
    private:
        struct ActionMapping {
            std::string actionName;
            std::vector<KeyCode> keys;
            std::vector<MouseButton> mouseButtons;
            std::vector<GamepadButton> gamepadButtons;
            float scale;
            bool inverted;
            
            ActionMapping(const std::string& name) 
                : actionName(name), scale(1.0f), inverted(false) {}
        };
        
        struct AxisMapping {
            std::string axisName;
            InputAxis positiveAxis;
            InputAxis negativeAxis;
            KeyCode positiveKey;
            KeyCode negativeKey;
            float sensitivity;
            float gravity;
            float deadZone;
            
            AxisMapping(const std::string& name)
                : axisName(name), sensitivity(1.0f), gravity(0.0f), deadZone(0.1f) {}
        };
        
        std::unordered_map<std::string, ActionMapping> actionMappings_;
        std::unordered_map<std::string, AxisMapping> axisMappings_;
        
    public:
        InputMapping();
        ~InputMapping();
        
        // مدیریت action mappings
        void AddAction(const std::string& actionName);
        void BindKeyToAction(const std::string& actionName, KeyCode key);
        void BindMouseButtonToAction(const std::string& actionName, MouseButton button);
        void BindGamepadButtonToAction(const std::string& actionName, GamepadButton button);
        
        // مدیریت axis mappings
        void AddAxis(const std::string& axisName);
        void BindAxisToKeys(const std::string& axisName, KeyCode positive, KeyCode negative);
        void BindAxisToGamepad(const std::string& axisName, InputAxis axis);
        void SetAxisSensitivity(const std::string& axisName, float sensitivity);
        void SetAxisDeadZone(const std::string& axisName, float deadZone);
        
        // ارزیابی نگاشت‌ها
        bool GetAction(const std::string& actionName) const;
        float GetAxis(const std::string& axisName) const;
        
        // بارگذاری/ذخیره سازی پیکربندی
        bool LoadFromFile(const std::string& filePath);
        bool SaveToFile(const std::string& filePath);
        
    private:
        float EvaluateAxis(const AxisMapping& mapping) const;
        bool EvaluateAction(const ActionMapping& mapping) const;
    };

    // سیستم کنترل مجازی (برای موبایل)
    class VirtualInputSystem {
    private:
        std::unordered_map<std::string, std::unique_ptr<VirtualControl>> virtualControls_;
        std::vector<TouchInput> activeTouches_;
        
        float stickDeadZone_;
        float buttonRadius_;
        
    public:
        VirtualInputSystem();
        ~VirtualInputSystem();
        
        void Initialize();
        void Update();
        void ProcessTouchEvent(const SDL_Event& event);
        
        // مدیریت کنترل‌های مجازی
        void AddVirtualStick(const std::string& name, const glm::vec2& center, float radius);
        void AddVirtualButton(const std::string& name, const glm::vec2& center, float radius);
        void RemoveVirtualControl(const std::string& name);
        
        // وضعیت کنترل‌های مجازی
        glm::vec2 GetStickDirection(const std::string& name) const;
        bool IsButtonPressed(const std::string& name) const;
        bool IsButtonJustPressed(const std::string& name) const;
        
        // تنظیمات
        void SetStickDeadZone(float deadZone) { stickDeadZone_ = deadZone; }
        void SetButtonRadius(float radius) { buttonRadius_ = radius; }
        
    private:
        void UpdateVirtualControls();
        void ProcessTouchForControls(const TouchInput& touch);
        VirtualControl* FindControlAtPosition(const glm::vec2& position);
    };

    // سیستم ورودی ترکیبی (ورودی‌های مختلف را ترکیب می‌کند)
    class InputCombiner {
    private:
        struct InputSource {
            InputDevice device;
            float weight;
            bool enabled;
        };
        
        std::vector<InputSource> activeSources_;
        InputMapping* inputMapping_;
        
        float mouseSensitivity_;
        float gamepadSensitivity_;
        float gyroSensitivity_;
        
    public:
        InputCombiner(InputMapping* mapping);
        ~InputCombiner();
        
        void AddInputSource(InputDevice device, float weight = 1.0f);
        void RemoveInputSource(InputDevice device);
        void SetSourceWeight(InputDevice device, float weight);
        void SetSourceEnabled(InputDevice device, bool enabled);
        
        // ترکیب ورودی‌ها
        float GetCombinedAxis(const std::string& axisName) const;
        bool GetCombinedAction(const std::string& actionName) const;
        glm::vec2 GetCombinedLookDelta() const;
        glm::vec2 GetCombinedMovement() const;
        
        // تنظیمات حساسیت
        void SetMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
        void SetGamepadSensitivity(float sensitivity) { gamepadSensitivity_ = sensitivity; }
        void SetGyroSensitivity(float sensitivity) { gyroSensitivity_ = sensitivity; }
        
    private:
        float GetSourceWeight(InputDevice device) const;
        bool IsSourceEnabled(InputDevice device) const;
        float CombineAxisValues(const std::vector<float>& values, const std::vector<float>& weights) const;
    };

    // سیستم بافر ورودی برای ثبت ورودی‌ها
    class InputBuffer {
    private:
        struct BufferedInput {
            std::string actionName;
            Uint32 timestamp;
            float value;
            
            BufferedInput(const std::string& name, Uint32 time, float val = 1.0f)
                : actionName(name), timestamp(time), value(val) {}
        };
        
        std::vector<BufferedInput> buffer_;
        Uint32 bufferDuration_;
        Uint32 currentTime_;
        
    public:
        InputBuffer(Uint32 duration = 500); // 500ms default
        ~InputBuffer();
        
        void Update(Uint32 deltaTime);
        void RecordInput(const std::string& actionName, float value = 1.0f);
        void Clear();
        
        // جستجو در بافر
        bool FindInputSequence(const std::vector<std::string>& sequence, Uint32 maxTimeGap = 200);
        bool FindInput(const std::string& actionName, Uint32 withinTime = 100);
        float GetInputValue(const std::string& actionName, Uint32 withinTime = 100) const;
        
        // مدیریت بافر
        void SetBufferDuration(Uint32 duration) { bufferDuration_ = duration; }
        Uint32 GetBufferDuration() const { return bufferDuration_; }
        size_t GetBufferSize() const { return buffer_.size(); }
        
    private:
        void RemoveExpiredInputs();
        bool CheckInputSequence(const std::vector<const BufferedInput*>& inputs, 
                               const std::vector<std::string>& sequence, Uint32 maxTimeGap) const;
    };

    // سیستم تشخیص ترکیب دکمه‌ها (مثل Combos در بازی‌های مبارزه‌ای)
    class InputComboSystem {
    private:
        struct ComboSequence {
            std::string comboName;
            std::vector<std::string> inputSequence;
            Uint32 maxTimeBetweenInputs;
            float damageMultiplier;
            bool requiresExactTiming;
            
            ComboSequence(const std::string& name, Uint32 maxTime = 300)
                : comboName(name), maxTimeBetweenInputs(maxTime), 
                  damageMultiplier(1.0f), requiresExactTiming(false) {}
        };
        
        std::vector<ComboSequence> combos_;
        InputBuffer* inputBuffer_;
        
    public:
        InputComboSystem(InputBuffer* buffer);
        ~InputComboSystem();
        
        void AddCombo(const std::string& name, const std::vector<std::string>& sequence, 
                     Uint32 maxTimeBetweenInputs = 300);
        void RemoveCombo(const std::string& name);
        void Update();
        
        // تشخیص کامبو
        bool CheckForCombo(const std::string& comboName) const;
        std::vector<std::string> GetActiveCombos() const;
        float GetComboDamageMultiplier(const std::string& comboName) const;
        
        // تنظیمات کامبو
        void SetComboTiming(const std::string& comboName, Uint32 maxTime);
        void SetComboDamageMultiplier(const std::string& comboName, float multiplier);
        void SetComboExactTiming(const std::string& comboName, bool exact);
        
    private:
        ComboSequence* FindCombo(const std::string& name);
        const ComboSequence* FindCombo(const std::string& name) const;
    };

    // کلاس اصلی مدیریت ورودی
    class InputHandler {
    private:
        static InputHandler* instance_;
        
        // وضعیت دستگاه‌های ورودی
        std::unordered_map<KeyCode, ButtonState> keyboardState_;
        std::unordered_map<MouseButton, ButtonState> mouseState_;
        std::vector<std::unique_ptr<Gamepad>> gamepads_;
        std::vector<TouchInput> touchInputs_;
        GyroInput gyroInput_;
        
        // موقعیت ماوس
        glm::vec2 mousePosition_;
        glm::vec2 mouseDelta_;
        glm::vec2 mouseWheel_;
        
        // سیستم‌های کمکی
        std::unique_ptr<InputMapping> inputMapping_;
        std::unique_ptr<VirtualInputSystem> virtualInput_;
        std::unique_ptr<InputCombiner> inputCombiner_;
        std::unique_ptr<InputBuffer> inputBuffer_;
        std::unique_ptr<InputComboSystem> comboSystem_;
        std::unique_ptr<TouchGestureRecognizer> gestureRecognizer_;
        
        // تنظیمات
        float mouseSensitivity_;
        bool mouseCaptured_;
        bool inputEnabled_;
        
        // آمار
        int keyPressCount_;
        int mouseClickCount_;
        int gamepadEventCount_;
        
    public:
        InputHandler();
        ~InputHandler();
        
        static InputHandler& GetInstance();
        static void DestroyInstance();
        
        bool Initialize();
        void Cleanup();
        void Update(float deltaTime);
        void ProcessEvent(const SDL_Event& event);
        
        // وضعیت صفحه کلید
        ButtonState GetKeyState(KeyCode key) const;
        bool IsKeyPressed(KeyCode key) const;
        bool IsKeyJustPressed(KeyCode key) const;
        bool IsKeyJustReleased(KeyCode key) const;
        
        // وضعیت ماوس
        ButtonState GetMouseButtonState(MouseButton button) const;
        bool IsMouseButtonPressed(MouseButton button) const;
        bool IsMouseButtonJustPressed(MouseButton button) const;
        bool IsMouseButtonJustReleased(MouseButton button) const;
        const glm::vec2& GetMousePosition() const { return mousePosition_; }
        const glm::vec2& GetMouseDelta() const { return mouseDelta_; }
        const glm::vec2& GetMouseWheel() const { return mouseWheel_; }
        
        // وضعیت گیم پد
        Gamepad* GetGamepad(int index = 0) const;
        int GetGamepadCount() const { return static_cast<int>(gamepads_.size()); }
        bool IsGamepadConnected(int index = 0) const;
        
        // وضعیت لمسی
        const std::vector<TouchInput>& GetTouchInputs() const { return touchInputs_; }
        int GetTouchCount() const { return static_cast<int>(touchInputs_.size()); }
        bool IsTouching() const { return !touchInputs_.empty(); }
        
        // وضعیت ژیروسکوپ
        const GyroInput& GetGyroInput() const { return gyroInput_; }
        bool HasGyro() const;
        
        // کنترل‌های مجازی
        VirtualInputSystem* GetVirtualInput() const { return virtualInput_.get(); }
        glm::vec2 GetVirtualStick(const std::string& name) const;
        bool GetVirtualButton(const std::string& name) const;
        
        // سیستم نگاشت ورودی
        InputMapping* GetInputMapping() const { return inputMapping_.get(); }
        bool GetAction(const std::string& actionName) const;
        float GetAxis(const std::string& axisName) const;
        
        // سیستم کامبو
        InputComboSystem* GetComboSystem() const { return comboSystem_.get(); }
        
        // سیستم ژست‌ها
        TouchGestureRecognizer* GetGestureRecognizer() const { return gestureRecognizer_.get(); }
        
        // تنظیمات
        void SetMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
        void SetMouseCaptured(bool captured);
        void SetInputEnabled(bool enabled) { inputEnabled_ = enabled; }
        
        float GetMouseSensitivity() const { return mouseSensitivity_; }
        bool IsMouseCaptured() const { return mouseCaptured_; }
        bool IsInputEnabled() const { return inputEnabled_; }
        
        // آمار
        int GetKeyPressCount() const { return keyPressCount_; }
        int GetMouseClickCount() const { return mouseClickCount_; }
        int GetGamepadEventCount() const { return gamepadEventCount_; }
        
    private:
        void InitializeDefaultMappings();
        void UpdateKeyboardState();
        void UpdateMouseState();
        void UpdateGamepads();
        void ProcessKeyboardEvent(const SDL_Event& event);
        void ProcessMouseEvent(const SDL_Event& event);
        void ProcessGamepadEvent(const SDL_Event& event);
        void ProcessTouchEvent(const SDL_Event& event);
        void ProcessGyroEvent(const SDL_Event& event);
        void AddGamepad(int deviceIndex);
        void RemoveGamepad(int instanceId);
    };

    // سیستم ورودی مبتنی بر رویداد
    class InputEventSystem {
    private:
        struct InputEvent {
            std::string eventName;
            std::function<void()> callback;
            bool once;
            bool triggered;
            
            InputEvent(const std::string& name, std::function<void()> cb, bool once = false)
                : eventName(name), callback(cb), once(once), triggered(false) {}
        };
        
        std::vector<InputEvent> events_;
        InputHandler* inputHandler_;
        
    public:
        InputEventSystem(InputHandler* handler);
        ~InputEventSystem();
        
        void RegisterEvent(const std::string& eventName, std::function<void()> callback, bool once = false);
        void UnregisterEvent(const std::string& eventName);
        void Update();
        
        // رویدادهای از پیش تعریف شده
        void OnAnyKeyPressed(std::function<void(KeyCode)> callback);
        void OnMouseClick(std::function<void(MouseButton, glm::vec2)> callback);
        void OnGamepadConnected(std::function<void(int)> callback);
        void OnGamepadDisconnected(std::function<void(int)> callback);
        void OnSwipe(std::function<void(glm::vec2, float)> callback);
        
    private:
        void CheckForEvents();
        void TriggerEvent(const std::string& eventName);
    };

} // namespace GalacticOdyssey

#endif // INPUT_HANDLER_H
