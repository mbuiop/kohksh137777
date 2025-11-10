#pragma once
#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Input/InputHandler.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace GalacticOdyssey {

    // انواع المان‌های UI
    enum class UIElementType {
        BUTTON,
        LABEL,
        PANEL,
        SLIDER,
        PROGRESS_BAR,
        CHECKBOX,
        TEXTBOX,
        LIST,
        DROPDOWN,
        IMAGE,
        ANIMATED_SPRITE,
        HEALTH_BAR,
        MINIMAP,
        HUD,
        TOOLTIP,
        MODAL,
        NOTIFICATION
    };

    // وضعیت المان‌های UI
    enum class UIState {
        NORMAL,
        HOVERED,
        PRESSED,
        DISABLED,
        FOCUSED,
        HIDDEN
    };

    // انیمیشن‌های UI
    enum class UIAnimation {
        NONE,
        FADE_IN,
        FADE_OUT,
        SLIDE_IN_LEFT,
        SLIDE_IN_RIGHT,
        SLIDE_IN_TOP,
        SLIDE_IN_BOTTOM,
        SLIDE_OUT_LEFT,
        SLIDE_OUT_RIGHT,
        SLIDE_OUT_TOP,
        SLIDE_OUT_BOTTOM,
        SCALE_IN,
        SCALE_OUT,
        BOUNCE,
        PULSE,
        SHAKE
    };

    // ساختار استایل UI
    struct UIStyle {
        glm::vec4 backgroundColor;
        glm::vec4 borderColor;
        glm::vec4 textColor;
        glm::vec4 hoverColor;
        glm::vec4 pressedColor;
        glm::vec4 disabledColor;
        float borderWidth;
        float borderRadius;
        float padding;
        float margin;
        std::string fontName;
        int fontSize;
        glm::vec2 shadowOffset;
        glm::vec4 shadowColor;
        
        UIStyle() 
            : backgroundColor(0.1f, 0.1f, 0.1f, 0.8f),
              borderColor(0.3f, 0.3f, 0.3f, 1.0f),
              textColor(1.0f, 1.0f, 1.0f, 1.0f),
              hoverColor(0.2f, 0.4f, 0.8f, 0.9f),
              pressedColor(0.1f, 0.3f, 0.7f, 1.0f),
              disabledColor(0.5f, 0.5f, 0.5f, 0.5f),
              borderWidth(2.0f), borderRadius(5.0f),
              padding(10.0f), margin(5.0f),
              fontName("default"), fontSize(16),
              shadowOffset(2.0f, 2.0f),
              shadowColor(0.0f, 0.0f, 0.0f, 0.5f) {}
    };

    // ساختار انیمیشن UI
    struct UIAnimationData {
        UIAnimation type;
        float duration;
        float elapsed;
        bool looping;
        std::function<void(class UIElement*, float)> updateCallback;
        
        UIAnimationData(UIAnimation animType = UIAnimation::NONE, float animDuration = 0.3f)
            : type(animType), duration(animDuration), elapsed(0.0f), looping(false) {}
    };

    // کلاس پایه المان UI
    class UIElement {
    protected:
        std::string id_;
        UIElementType type_;
        UIState state_;
        UIStyle style_;
        
        glm::vec2 position_;
        glm::vec2 size_;
        glm::vec2 originalPosition_;
        glm::vec2 originalSize_;
        
        bool visible_;
        bool enabled_;
        bool interactive_;
        bool draggable_;
        
        UIAnimationData currentAnimation_;
        std::vector<std::function<void(UIElement*)>> eventCallbacks_;
        
        class UIManager* manager_;
        UIElement* parent_;
        std::vector<std::unique_ptr<UIElement>> children_;
        
    public:
        UIElement(const std::string& id, UIElementType type, const glm::vec2& position, const glm::vec2& size);
        virtual ~UIElement();
        
        virtual void Update(float deltaTime);
        virtual void Render(RenderSystem* renderer);
        virtual void HandleInput(InputHandler* input, const glm::vec2& mousePos);
        
        // مدیریت وضعیت
        void SetState(UIState newState);
        void SetVisible(bool visible);
        void SetEnabled(bool enabled);
        void SetInteractive(bool interactive);
        
        // مدیریت موقعیت و اندازه
        void SetPosition(const glm::vec2& position);
        void SetSize(const glm::vec2& size);
        void SetCenterPosition(const glm::vec2& center);
        void Move(const glm::vec2& offset);
        void Scale(float factor);
        
        // مدیریت استایل
        void SetStyle(const UIStyle& newStyle);
        void ApplyStyleVariant(const std::string& variantName);
        
        // مدیریت انیمیشن
        void StartAnimation(UIAnimation animation, float duration = 0.3f, bool looping = false);
        void StopAnimation();
        bool IsAnimating() const;
        
        // مدیریت فرزندان
        void AddChild(std::unique_ptr<UIElement> child);
        void RemoveChild(const std::string& childId);
        void RemoveAllChildren();
        UIElement* GetChild(const std::string& childId) const;
        
        // رویدادها
        void AddEventListener(const std::string& eventType, std::function<void(UIElement*)> callback);
        void RemoveEventListener(const std::string& eventType);
        void TriggerEvent(const std::string& eventType);
        
        // اطلاعات
        const std::string& GetId() const { return id_; }
        UIElementType GetType() const { return type_; }
        UIState GetState() const { return state_; }
        const glm::vec2& GetPosition() const { return position_; }
        const glm::vec2& GetSize() const { return size_; }
        glm::vec2 GetAbsolutePosition() const;
        glm::vec2 GetCenter() const;
        bool IsVisible() const { return visible_; }
        bool IsEnabled() const { return enabled_; }
        bool IsInteractive() const { return interactive_; }
        bool ContainsPoint(const glm::vec2& point) const;
        
        // تنظیمات
        void SetManager(class UIManager* manager) { manager_ = manager; }
        void SetParent(UIElement* parent) { parent_ = parent; }
        
    protected:
        virtual void OnStateChange(UIState oldState, UIState newState);
        virtual void OnAnimationUpdate(float progress);
        virtual void UpdateAnimation(float deltaTime);
        virtual void ApplyCurrentStyle();
        
        // محاسبات
        glm::vec4 GetCurrentColor() const;
        glm::vec2 CalculateAnimationOffset(float progress) const;
        float CalculateAnimationScale(float progress) const;
        float CalculateAnimationAlpha(float progress) const;
    };

    // کلاس دکمه
    class UIButton : public UIElement {
    private:
        std::string text_;
        std::string icon_;
        std::function<void()> onClick_;
        bool toggleable_;
        bool toggled_;
        
    public:
        UIButton(const std::string& id, const std::string& text, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        void HandleInput(InputHandler* input, const glm::vec2& mousePos) override;
        
        void SetText(const std::string& text);
        void SetIcon(const std::string& iconPath);
        void SetOnClick(std::function<void()> callback);
        void SetToggleable(bool toggleable);
        void SetToggled(bool toggled);
        void Toggle();
        
        const std::string& GetText() const { return text_; }
        bool IsToggled() const { return toggled_; }
        
    private:
        void RenderButtonBackground(RenderSystem* renderer);
        void RenderButtonText(RenderSystem* renderer);
        void RenderButtonIcon(RenderSystem* renderer);
    };

    // کلاس نوار پیشرفت
    class UIProgressBar : public UIElement {
    private:
        float value_;
        float maxValue_;
        float minValue_;
        glm::vec4 fillColor_;
        glm::vec4 backgroundColor_;
        bool showText_;
        std::string textFormat_;
        
    public:
        UIProgressBar(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Render(RenderSystem* renderer) override;
        
        void SetValue(float value);
        void SetMaxValue(float maxValue);
        void SetMinValue(float minValue);
        void SetFillColor(const glm::vec4& color);
        void SetShowText(bool show);
        void SetTextFormat(const std::string& format);
        
        float GetValue() const { return value_; }
        float GetMaxValue() const { return maxValue_; }
        float GetPercentage() const;
        
    private:
        void RenderBarBackground(RenderSystem* renderer);
        void RenderBarFill(RenderSystem* renderer);
        void RenderBarText(RenderSystem* renderer);
        std::string GetDisplayText() const;
    };

    // کلاس نوار سلامت
    class UIHealthBar : public UIProgressBar {
    private:
        bool showShield_;
        float shieldValue_;
        float maxShieldValue_;
        glm::vec4 shieldColor_;
        
    public:
        UIHealthBar(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Render(RenderSystem* renderer) override;
        
        void SetShieldValue(float value);
        void SetMaxShieldValue(float maxValue);
        void SetShieldColor(const glm::vec4& color);
        void SetShowShield(bool show);
        
    private:
        void RenderShieldBar(RenderSystem* renderer);
    };

    // کلاس مینی‌مپ
    class UIMinimap : public UIElement {
    private:
        float worldSize_;
        float zoomLevel_;
        glm::vec2 worldCenter_;
        std::vector<glm::vec2> playerPositions_;
        std::vector<glm::vec2> enemyPositions_;
        std::vector<glm::vec2> objectivePositions_;
        glm::vec4 playerColor_;
        glm::vec4 enemyColor_;
        glm::vec4 objectiveColor_;
        bool showGrid_;
        
    public:
        UIMinimap(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        
        void SetWorldSize(float size);
        void SetZoomLevel(float zoom);
        void SetWorldCenter(const glm::vec2& center);
        void AddPlayerPosition(const glm::vec2& position);
        void AddEnemyPosition(const glm::vec2& position);
        void AddObjectivePosition(const glm::vec2& position);
        void ClearPositions();
        
    private:
        void RenderMinimapBackground(RenderSystem* renderer);
        void RenderMinimapGrid(RenderSystem* renderer);
        void RenderMinimapEntities(RenderSystem* renderer);
        glm::vec2 WorldToMinimap(const glm::vec2& worldPos) const;
    };

    // کلاس HUD بازی
    class UIHUD : public UIElement {
    private:
        std::unique_ptr<UIHealthBar> healthBar_;
        std::unique_ptr<UIProgressBar> energyBar_;
        std::unique_ptr<UIMinimap> minimap_;
        std::unique_ptr<UIElement> scoreDisplay_;
        std::unique_ptr<UIElement> weaponDisplay_;
        std::unique_ptr<UIElement> ammoDisplay_;
        
        int currentScore_;
        int currentLevel_;
        int currentWave_;
        float gameTime_;
        
    public:
        UIHUD(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        
        void SetScore(int score);
        void SetLevel(int level);
        void SetWave(int wave);
        void SetGameTime(float time);
        void UpdatePlayerStats(class Player* player);
        void UpdateWeaponInfo(const std::string& weaponName, int ammo, int maxAmmo);
        
    private:
        void RenderHUDBackground(RenderSystem* renderer);
        void RenderScore(RenderSystem* renderer);
        void RenderLevelInfo(RenderSystem* renderer);
        void RenderGameTime(RenderSystem* renderer);
        void CreateHUDElements();
    };

    // کلاس منوی اصلی
    class UIMainMenu : public UIElement {
    private:
        std::unique_ptr<UIButton> startButton_;
        std::unique_ptr<UIButton> settingsButton_;
        std::unique_ptr<UIButton> quitButton_;
        std::unique_ptr<UIElement> title_;
        std::unique_ptr<UIElement> background_;
        std::unique_ptr<UIElement> versionText_;
        
    public:
        UIMainMenu(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        void HandleInput(InputHandler* input, const glm::vec2& mousePos) override;
        
        void SetOnStartGame(std::function<void()> callback);
        void SetOnOpenSettings(std::function<void()> callback);
        void SetOnQuitGame(std::function<void()> callback);
        
    private:
        void CreateMenuElements();
        void ArrangeMenuItems();
        void RenderMenuBackground(RenderSystem* renderer);
    };

    // کلاس منوی مکث
    class UIPauseMenu : public UIElement {
    private:
        std::unique_ptr<UIButton> resumeButton_;
        std::unique_ptr<UIButton> restartButton_;
        std::unique_ptr<UIButton> settingsButton_;
        std::unique_ptr<UIButton> mainMenuButton_;
        std::unique_ptr<UIElement> title_;
        std::unique_ptr<UIElement> background_;
        
    public:
        UIPauseMenu(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        void HandleInput(InputHandler* input, const glm::vec2& mousePos) override;
        
        void SetOnResume(std::function<void()> callback);
        void SetOnRestart(std::function<void()> callback);
        void SetOnOpenSettings(std::function<void()> callback);
        void SetOnMainMenu(std::function<void()> callback);
        
    private:
        void CreateMenuElements();
        void ArrangeMenuItems();
    };

    // کلاس منوی تنظیمات
    class UISettingsMenu : public UIElement {
    private:
        std::unique_ptr<UIElement> title_;
        std::unique_ptr<UIElement> audioSection_;
        std::unique_ptr<UIElement> graphicsSection_;
        std::unique_ptr<UIElement> controlsSection_;
        std::unique_ptr<UIButton> backButton_;
        std::unique_ptr<UIButton> applyButton_;
        std::unique_ptr<UIButton> defaultsButton_;
        
        // کنترل‌های تنظیمات
        std::unique_ptr<class UISlider> masterVolumeSlider_;
        std::unique_ptr<class UISlider> musicVolumeSlider_;
        std::unique_ptr<class UISlider> effectsVolumeSlider_;
        std::unique_ptr<class UICheckbox> fullscreenCheckbox_;
        std::unique_ptr<class UIDropdown> resolutionDropdown_;
        std::unique_ptr<class UISlider> sensitivitySlider_;
        
    public:
        UISettingsMenu(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        void HandleInput(InputHandler* input, const glm::vec2& mousePos) override;
        
        void LoadSettings();
        void ApplySettings();
        void ResetToDefaults();
        
        void SetOnBack(std::function<void()> callback);
        void SetOnApply(std::function<void()> callback);
        
    private:
        void CreateSettingsElements();
        void CreateAudioSettings();
        void CreateGraphicsSettings();
        void CreateControlsSettings();
        void ArrangeSettingsSections();
    };

    // کلاس نمایش امتیاز نهایی
    class UIResultsScreen : public UIElement {
    private:
        int finalScore_;
        int highScore_;
        int enemiesKilled_;
        int coinsCollected_;
        float completionTime_;
        int starsEarned_;
        bool newHighScore_;
        
        std::unique_ptr<UIElement> title_;
        std::unique_ptr<UIElement> scoreDisplay_;
        std::unique_ptr<UIElement> statsPanel_;
        std::unique_ptr<UIButton> continueButton_;
        std::unique_ptr<UIButton> retryButton_;
        std::unique_ptr<UIButton> mainMenuButton_;
        std::unique_ptr<UIElement> starDisplay_;
        
    public:
        UIResultsScreen(const std::string& id, const glm::vec2& position, const glm::vec2& size);
        
        void Update(float deltaTime) override;
        void Render(RenderSystem* renderer) override;
        
        void SetResults(int score, int highScore, int enemies, int coins, float time, bool victory);
        void CalculateStars();
        
        void SetOnContinue(std::function<void()> callback);
        void SetOnRetry(std::function<void()> callback);
        void SetOnMainMenu(std::function<void()> callback);
        
    private:
        void CreateResultElements();
        void RenderStars(RenderSystem* renderer);
        void RenderStats(RenderSystem* renderer);
        void AnimateScore();
    };

    // کلاس سیستم tooltip
    class UITooltipSystem {
    private:
        struct Tooltip {
            std::string id;
            std::string text;
            glm::vec2 position;
            float showDelay;
            float currentDelay;
            bool showing;
            UIStyle style;
            
            Tooltip(const std::string& id, const std::string& text, const glm::vec2& pos)
                : id(id), text(text), position(pos), showDelay(0.5f), 
                  currentDelay(0.0f), showing(false) {}
        };
        
        std::vector<Tooltip> activeTooltips_;
        std::vector<Tooltip> tooltipQueue_;
        RenderSystem* renderSystem_;
        
    public:
        UITooltipSystem(RenderSystem* renderer);
        ~UITooltipSystem();
        
        void Update(float deltaTime);
        void Render();
        
        void ShowTooltip(const std::string& id, const std::string& text, const glm::vec2& position, float delay = 0.5f);
        void HideTooltip(const std::string& id);
        void HideAllTooltips();
        
        void SetTooltipStyle(const std::string& id, const UIStyle& style);
        
    private:
        void ProcessQueue(float deltaTime);
        void RenderTooltip(const Tooltip& tooltip);
        Tooltip* FindTooltip(const std::string& id);
    };

    // کلاس مدیر اصلی UI
    class UIManager {
    private:
        static UIManager* instance_;
        
        RenderSystem* renderSystem_;
        InputHandler* inputHandler_;
        
        std::unordered_map<std::string, std::unique_ptr<UIElement>> screens_;
        std::vector<UIElement*> activeScreens_;
        std::unique_ptr<UITooltipSystem> tooltipSystem_;
        
        glm::vec2 screenSize_;
        UIStyle defaultStyle_;
        std::unordered_map<std::string, UIStyle> styleThemes_;
        
        UIElement* focusedElement_;
        UIElement* hoveredElement_;
        UIElement* draggedElement_;
        
        bool debugMode_;
        float uiScale_;
        
    public:
        UIManager();
        ~UIManager();
        
        static UIManager& GetInstance();
        static void DestroyInstance();
        
        bool Initialize(RenderSystem* renderer, InputHandler* input);
        void Cleanup();
        void Update(float deltaTime);
        void Render();
        
        // مدیریت اسکرین‌ها
        void AddScreen(const std::string& screenId, std::unique_ptr<UIElement> screen);
        void RemoveScreen(const std::string& screenId);
        void ShowScreen(const std::string& screenId);
        void HideScreen(const std::string& screenId);
        void HideAllScreens();
        UIElement* GetScreen(const std::string& screenId) const;
        
        // مدیریت المان‌ها
        UIElement* GetElementById(const std::string& id) const;
        std::vector<UIElement*> GetElementsByType(UIElementType type) const;
        
        // مدیریت استایل
        void SetDefaultStyle(const UIStyle& style);
        void AddStyleTheme(const std::string& themeName, const UIStyle& style);
        void ApplyTheme(const std::string& themeName);
        const UIStyle& GetStyle(const std::string& themeName = "") const;
        
        // مدیریت tooltip
        void ShowTooltip(const std::string& id, const std::string& text, const glm::vec2& position, float delay = 0.5f);
        void HideTooltip(const std::string& id);
        
        // تنظیمات
        void SetScreenSize(const glm::vec2& size);
        void SetUIScale(float scale);
        void SetDebugMode(bool debug);
        
        const glm::vec2& GetScreenSize() const { return screenSize_; }
        float GetUIScale() const { return uiScale_; }
        bool IsDebugMode() const { return debugMode_; }
        
        // utility functions
        glm::vec2 ScreenToUIPosition(const glm::vec2& screenPos) const;
        glm::vec2 UIToScreenPosition(const glm::vec2& uiPos) const;
        float ScaleValue(float value) const;
        glm::vec2 ScaleVector(const glm::vec2& vector) const;
        
    private:
        void UpdateInput();
        void UpdateElements(float deltaTime);
        void RenderElements();
        void HandleMouseInput();
        void HandleKeyboardInput();
        void CleanupInactiveScreens();
        
        UIElement* FindElementById(const std::string& id, UIElement* root) const;
        void CollectElementsByType(UIElementType type, UIElement* root, std::vector<UIElement*>& results) const;
    };

    // کلاس فکتوری برای ایجاد المان‌های UI
    class UIFactory {
    private:
        UIManager* uiManager_;
        
    public:
        UIFactory(UIManager* manager);
        
        // ایجاد المان‌های پایه
        std::unique_ptr<UIButton> CreateButton(const std::string& id, const std::string& text, 
                                              const glm::vec2& position, const glm::vec2& size);
        std::unique_ptr<UIProgressBar> CreateProgressBar(const std::string& id, 
                                                        const glm::vec2& position, const glm::vec2& size);
        std::unique_ptr<UIHealthBar> CreateHealthBar(const std::string& id,
                                                    const glm::vec2& position, const glm::vec2& size);
        std::unique_ptr<UIMinimap> CreateMinimap(const std::string& id,
                                                const glm::vec2& position, const glm::vec2& size);
        
        // ایجاد اسکرین‌های از پیش ساخته شده
        std::unique_ptr<UIMainMenu> CreateMainMenu();
        std::unique_ptr<UIPauseMenu> CreatePauseMenu();
        std::unique_ptr<UISettingsMenu> CreateSettingsMenu();
        std::unique_ptr<UIHUD> CreateGameHUD();
        std::unique_ptr<UIResultsScreen> CreateResultsScreen();
        
        // ایجاد المان‌های مرکب
        std::unique_ptr<UIElement> CreateStatDisplay(const std::string& id, const glm::vec2& position,
                                                    const std::string& icon, const std::string& value);
        std::unique_ptr<UIElement> CreateWeaponDisplay(const std::string& id, const glm::vec2& position);
        std::unique_ptr<UIElement> CreateAbilityBar(const std::string& id, const glm::vec2& position);
        
    private:
        glm::vec2 CalculateCenteredPosition(const glm::vec2& size) const;
        UIStyle GetStyleForType(UIElementType type) const;
    };

    // کلاس مدیریت فونت
    class UIFontManager {
    private:
        struct Font {
            std::string name;
            int size;
            void* fontData; // برای پیاده‌سازی خاص
            bool bold;
            bool italic;
            
            Font(const std::string& fontName, int fontSize, bool isBold = false, bool isItalic = false)
                : name(fontName), size(fontSize), fontData(nullptr), 
                  bold(isBold), italic(isItalic) {}
        };
        
        std::unordered_map<std::string, Font> loadedFonts_;
        std::string defaultFont_;
        
    public:
        UIFontManager();
        ~UIFontManager();
        
        bool LoadFont(const std::string& fontName, const std::string& filePath, int size);
        bool LoadSystemFont(const std::string& fontName, int size, bool bold = false, bool italic = false);
        void UnloadFont(const std::string& fontName);
        void UnloadAllFonts();
        
        void SetDefaultFont(const std::string& fontName);
        Font* GetFont(const std::string& fontName);
        Font* GetDefaultFont();
        
        glm::vec2 CalculateTextSize(const std::string& text, const std::string& fontName, int size);
        bool IsFontLoaded(const std::string& fontName) const;
        
    private:
        std::string GenerateFontKey(const std::string& fontName, int size, bool bold, bool italic) const;
    };

    // کلاس مدیریت تم‌ها
    class UIThemeManager {
    private:
        struct Theme {
            std::string name;
            UIStyle defaultStyle;
            std::unordered_map<std::string, UIStyle> componentStyles;
            std::unordered_map<UIState, UIStyle> stateStyles;
            
            Theme(const std::string& themeName) : name(themeName) {}
        };
        
        std::unordered_map<std::string, Theme> themes_;
        std::string currentTheme_;
        
    public:
        UIThemeManager();
        ~UIThemeManager();
        
        void LoadTheme(const std::string& themeName, const std::string& filePath);
        void CreateTheme(const std::string& themeName, const UIStyle& defaultStyle);
        void SaveTheme(const std::string& themeName, const std::string& filePath);
        
        void SetCurrentTheme(const std::string& themeName);
        void ApplyThemeToElement(UIElement* element, const std::string& themeName = "");
        
        UIStyle GetStyleForComponent(const std::string& componentType, const std::string& themeName = "") const;
        UIStyle GetStyleForState(UIState state, const std::string& themeName = "") const;
        
        const std::string& GetCurrentTheme() const { return currentTheme_; }
        bool ThemeExists(const std::string& themeName) const;
        
    private:
        void LoadDefaultThemes();
        void ParseThemeFile(const std::string& filePath, Theme& theme);
        UIStyle MergeStyles(const UIStyle& base, const UIStyle& overlay) const;
    };

} // namespace GalacticOdyssey

#endif // UI_MANAGER_H
