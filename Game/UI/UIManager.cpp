#include "UIManager.h"
#include <iostream>
#include <algorithm>

namespace GalacticOdyssey {

    // پیاده‌سازی UIManager (Singleton)
    UIManager* UIManager::instance_ = nullptr;

    UIManager::UIManager()
        : renderSystem_(nullptr), inputHandler_(nullptr),
          screenSize_(1920.0f, 1080.0f), focusedElement_(nullptr),
          hoveredElement_(nullptr), draggedElement_(nullptr),
          debugMode_(false), uiScale_(1.0f)
    {
        std::cout << "🎨 ایجاد مدیر رابط کاربری" << std::endl;
        
        // ایجاد سیستم tooltip
        tooltipSystem_ = std::make_unique<UITooltipSystem>(renderSystem_);
        
        // بارگذاری تم پیش‌فرض
        LoadDefaultThemes();
    }

    UIManager::~UIManager()
    {
        Cleanup();
    }

    UIManager& UIManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new UIManager();
        }
        return *instance_;
    }

    void UIManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool UIManager::Initialize(RenderSystem* renderer, InputHandler* input)
    {
        std::cout << "🔧 در حال راه‌اندازی مدیر رابط کاربری..." << std::endl;
        
        renderSystem_ = renderer;
        inputHandler_ = input;
        
        if (!renderSystem_ || !inputHandler_) {
            std::cerr << "❌ سیستم‌های وابسته راه‌اندازی نشده‌اند" << std::endl;
            return false;
        }
        
        // تنظیم اندازه صفحه
        screenSize_ = glm::vec2(renderSystem_->GetGraphicsSettings().screenWidth,
                               renderSystem_->GetGraphicsSettings().screenHeight);
        
        // راه‌اندازی سیستم tooltip
        tooltipSystem_ = std::make_unique<UITooltipSystem>(renderSystem_);
        
        std::cout << "✅ مدیر رابط کاربری با موفقیت راه‌اندازی شد" << std::endl;
        return true;
    }

    void UIManager::Cleanup()
    {
        std::cout << "🧹 پاکسازی مدیر رابط کاربری..." << std::endl;
        
        activeScreens_.clear();
        screens_.clear();
        tooltipSystem_.reset();
        
        focusedElement_ = nullptr;
        hoveredElement_ = nullptr;
        draggedElement_ = nullptr;
        
        std::cout << "✅ مدیر رابط کاربری پاکسازی شد" << std::endl;
    }

    void UIManager::Update(float deltaTime)
    {
        // به‌روزرسانی ورودی
        UpdateInput();
        
        // به‌روزرسانی المان‌ها
        UpdateElements(deltaTime);
        
        // به‌روزرسانی سیستم tooltip
        if (tooltipSystem_) {
            tooltipSystem_->Update(deltaTime);
        }
        
        // پاکسازی اسکرین‌های غیرفعال
        CleanupInactiveScreens();
    }

    void UIManager::Render()
    {
        if (!renderSystem_) return;
        
        // رندر المان‌ها
        RenderElements();
        
        // رندر سیستم tooltip
        if (tooltipSystem_) {
            tooltipSystem_->Render();
        }
        
        // رندر اطلاعات دیباگ
        if (debugMode_) {
            RenderDebugInfo();
        }
    }

    void UIManager::UpdateInput()
    {
        if (!inputHandler_) return;
        
        HandleMouseInput();
        HandleKeyboardInput();
    }

    void UIManager::HandleMouseInput()
    {
        glm::vec2 mousePos = inputHandler_->GetMousePosition();
        glm::vec2 uiMousePos = ScreenToUIPosition(mousePos);
        
        // بررسی درگ
        if (draggedElement_ && inputHandler_->IsMouseButtonPressed(MouseButton::LEFT)) {
            // ادامه درگ
            glm::vec2 delta = uiMousePos - draggedElement_->GetPosition();
            draggedElement_->Move(delta);
        } else if (draggedElement_) {
            // پایان درگ
            draggedElement_ = nullptr;
        }
        
        // بررسی هاور
        UIElement* newHovered = nullptr;
        for (auto it = activeScreens_.rbegin(); it != activeScreens_.rend(); ++it) {
            UIElement* element = *it;
            if (element->IsVisible() && element->IsInteractive() && element->ContainsPoint(uiMousePos)) {
                newHovered = element;
                break;
            }
        }
        
        if (newHovered != hoveredElement_) {
            if (hoveredElement_) {
                hoveredElement_->SetState(UIState::NORMAL);
            }
            hoveredElement_ = newHovered;
            if (hoveredElement_) {
                hoveredElement_->SetState(UIState::HOVERED);
            }
        }
        
        // بررسی کلیک
        if (inputHandler_->IsMouseButtonJustPressed(MouseButton::LEFT)) {
            if (hoveredElement_) {
                hoveredElement_->SetState(UIState::PRESSED);
                
                // فوکوس
                if (focusedElement_ && focusedElement_ != hoveredElement_) {
                    focusedElement_->SetState(UIState::NORMAL);
                }
                focusedElement_ = hoveredElement_;
                focusedElement_->SetState(UIState::FOCUSED);
                
                // بررسی درگ
                if (hoveredElement_->IsDraggable()) {
                    draggedElement_ = hoveredElement_;
                }
            } else {
                // از دست دادن فوکوس
                if (focusedElement_) {
                    focusedElement_->SetState(UIState::NORMAL);
                    focusedElement_ = nullptr;
                }
            }
        }
    }

    void UIManager::UpdateElements(float deltaTime)
    {
        for (auto& screen : screens_) {
            if (screen.second->IsVisible()) {
                screen.second->Update(deltaTime);
            }
        }
    }

    void UIManager::RenderElements()
    {
        for (auto& screen : screens_) {
            if (screen.second->IsVisible()) {
                screen.second->Render(renderSystem_);
            }
        }
    }

    void UIManager::AddScreen(const std::string& screenId, std::unique_ptr<UIElement> screen)
    {
        screen->SetManager(this);
        screens_[screenId] = std::move(screen);
        std::cout << "📺 افزودن اسکرین: " << screenId << std::endl;
    }

    void UIManager::ShowScreen(const std::string& screenId)
    {
        auto it = screens_.find(screenId);
        if (it != screens_.end()) {
            it->second->SetVisible(true);
            activeScreens_.push_back(it->second.get());
            std::cout << "👁️ نمایش اسکرین: " << screenId << std::endl;
        }
    }

    void UIManager::HideScreen(const std::string& screenId)
    {
        auto it = screens_.find(screenId);
        if (it != screens_.end()) {
            it->second->SetVisible(false);
            
            // حذف از لیست فعال
            auto activeIt = std::find(activeScreens_.begin(), activeScreens_.end(), it->second.get());
            if (activeIt != activeScreens_.end()) {
                activeScreens_.erase(activeIt);
            }
            
            std::cout << "🚫 مخفی کردن اسکرین: " << screenId << std::endl;
        }
    }

    UIElement* UIManager::GetElementById(const std::string& id) const
    {
        for (auto& screen : screens_) {
            UIElement* element = FindElementById(id, screen.second.get());
            if (element) {
                return element;
            }
        }
        return nullptr;
    }

    UIElement* UIManager::FindElementById(const std::string& id, UIElement* root) const
    {
        if (!root) return nullptr;
        
        if (root->GetId() == id) {
            return root;
        }
        
        // جستجو در فرزندان (در پیاده‌سازی کامل)
        return nullptr;
    }

    void UIManager::RenderDebugInfo()
    {
        if (!renderSystem_) return;
        
        std::string debugText = "UI Debug Mode\n";
        debugText += "Active Screens: " + std::to_string(activeScreens_.size()) + "\n";
        debugText += "Hovered: " + (hoveredElement_ ? hoveredElement_->GetId() : "None") + "\n";
        debugText += "Focused: " + (focusedElement_ ? focusedElement_->GetId() : "None") + "\n";
        debugText += "Mouse Pos: " + std::to_string(inputHandler_->GetMousePosition().x) + 
                    ", " + std::to_string(inputHandler_->GetMousePosition().y);
        
        renderSystem_->RenderText(debugText, 10, 10, 14, glm::vec3(1.0f, 1.0f, 0.0f));
    }

    // پیاده‌سازی UIElement
    UIElement::UIElement(const std::string& id, UIElementType type, const glm::vec2& position, const glm::vec2& size)
        : id_(id), type_(type), state_(UIState::NORMAL), position_(position), size_(size),
          originalPosition_(position), originalSize_(size), visible_(true), enabled_(true),
          interactive_(true), draggable_(false), manager_(nullptr), parent_(nullptr)
    {
        std::cout << "🔲 ایجاد المان UI: " << id_ << std::endl;
    }

    void UIElement::Update(float deltaTime)
    {
        // به‌روزرسانی انیمیشن
        if (currentAnimation_.type != UIAnimation::NONE) {
            UpdateAnimation(deltaTime);
        }
        
        // به‌روزرسانی فرزندان
        for (auto& child : children_) {
            if (child->IsVisible()) {
                child->Update(deltaTime);
            }
        }
    }

    void UIElement::Render(RenderSystem* renderer)
    {
        if (!visible_ || !renderer) return;
        
        // رندر پس‌زمینه
        glm::vec4 color = GetCurrentColor();
        glm::vec2 absPos = GetAbsolutePosition();
        
        // رندر مستطیل پس‌زمینه
        renderer->RenderQuad(absPos.x, absPos.y, size_.x, size_.y, color);
        
        // رندر حاشیه
        if (style_.borderWidth > 0.0f) {
            renderer->RenderQuad(absPos.x - style_.borderWidth, 
                               absPos.y - style_.borderWidth,
                               size_.x + style_.borderWidth * 2,
                               size_.y + style_.borderWidth * 2,
                               style_.borderColor);
        }
        
        // رندر فرزندان
        for (auto& child : children_) {
            if (child->IsVisible()) {
                child->Render(renderer);
            }
        }
    }

    void UIElement::HandleInput(InputHandler* input, const glm::vec2& mousePos)
    {
        if (!enabled_ || !interactive_ || !visible_) return;
        
        // بررسی فرزندان (اولویت با فرزندان)
        for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
            if ((*it)->IsInteractive() && (*it)->ContainsPoint(mousePos)) {
                (*it)->HandleInput(input, mousePos);
                return;
            }
        }
        
        // اگر هیچ فرزندی ورودی را نگرفت، خود المان پردازش کند
        if (ContainsPoint(mousePos)) {
            // پردازش ورودی در کلاس‌های مشتق
        }
    }

    glm::vec4 UIElement::GetCurrentColor() const
    {
        switch (state_) {
            case UIState::NORMAL:
                return style_.backgroundColor;
            case UIState::HOVERED:
                return style_.hoverColor;
            case UIState::PRESSED:
                return style_.pressedColor;
            case UIState::DISABLED:
                return style_.disabledColor;
            case UIState::FOCUSED:
                return style_.hoverColor; // یا رنگ جداگانه
            case UIState::HIDDEN:
                return glm::vec4(0.0f);
            default:
                return style_.backgroundColor;
        }
    }

    bool UIElement::ContainsPoint(const glm::vec2& point) const
    {
        glm::vec2 absPos = GetAbsolutePosition();
        return point.x >= absPos.x && point.x <= absPos.x + size_.x &&
               point.y >= absPos.y && point.y <= absPos.y + size_.y;
    }

    void UIElement::StartAnimation(UIAnimation animation, float duration, bool looping)
    {
        currentAnimation_ = UIAnimationData(animation, duration);
        currentAnimation_.looping = looping;
        currentAnimation_.elapsed = 0.0f;
    }

    void UIElement::UpdateAnimation(float deltaTime)
    {
        currentAnimation_.elapsed += deltaTime;
        float progress = currentAnimation_.elapsed / currentAnimation_.duration;
        
        if (progress >= 1.0f) {
            if (currentAnimation_.looping) {
                currentAnimation_.elapsed = 0.0f;
                progress = 0.0f;
            } else {
                progress = 1.0f;
                currentAnimation_.type = UIAnimation::NONE;
            }
        }
        
        OnAnimationUpdate(progress);
    }

    // پیاده‌سازی UIButton
    UIButton::UIButton(const std::string& id, const std::string& text, const glm::vec2& position, const glm::vec2& size)
        : UIElement(id, UIElementType::BUTTON, position, size), text_(text), 
          toggleable_(false), toggled_(false)
    {
        // استایل پیش‌فرض دکمه
        style_.backgroundColor = glm::vec4(0.2f, 0.4f, 0.8f, 1.0f);
        style_.hoverColor = glm::vec4(0.3f, 0.5f, 0.9f, 1.0f);
        style_.pressedColor = glm::vec4(0.1f, 0.3f, 0.7f, 1.0f);
        style_.borderRadius = 8.0f;
    }

    void UIButton::Render(RenderSystem* renderer)
    {
        if (!IsVisible() || !renderer) return;
        
        RenderButtonBackground(renderer);
        RenderButtonText(renderer);
        
        // رندر فرزندان
        for (auto& child : children_) {
            if (child->IsVisible()) {
                child->Render(renderer);
            }
        }
    }

    void UIButton::RenderButtonBackground(RenderSystem* renderer)
    {
        glm::vec2 absPos = GetAbsolutePosition();
        glm::vec4 color = GetCurrentColor();
        
        // اگر toggle شده، رنگ متفاوت نشان داده شود
        if (toggleable_ && toggled_) {
            color = style_.pressedColor;
        }
        
        // رندر دکمه با گوشه‌های گرد
        renderer->RenderRoundedQuad(absPos.x, absPos.y, size_.x, size_.y, 
                                  style_.borderRadius, color);
        
        // رندر حاشیه
        if (style_.borderWidth > 0.0f) {
            renderer->RenderRoundedQuad(absPos.x - style_.borderWidth,
                                      absPos.y - style_.borderWidth,
                                      size_.x + style_.borderWidth * 2,
                                      size_.y + style_.borderWidth * 2,
                                      style_.borderRadius + style_.borderWidth,
                                      style_.borderColor);
        }
    }

    void UIButton::RenderButtonText(RenderSystem* renderer)
    {
        if (text_.empty()) return;
        
        glm::vec2 absPos = GetAbsolutePosition();
        glm::vec2 center = absPos + size_ * 0.5f;
        
        // محاسبه موقعیت متن (مرکز)
        // در پیاده‌سازی کامل از UIFontManager استفاده می‌شود
        renderer->RenderText(text_, center.x, center.y, style_.fontSize, 
                           style_.textColor, true);
    }

    void UIButton::HandleInput(InputHandler* input, const glm::vec2& mousePos)
    {
        if (!IsEnabled() || !IsInteractive() || !IsVisible()) return;
        
        UIElement::HandleInput(input, mousePos);
        
        if (ContainsPoint(mousePos)) {
            if (input->IsMouseButtonJustPressed(MouseButton::LEFT)) {
                if (toggleable_) {
                    Toggle();
                }
                
                if (onClick_) {
                    onClick_();
                }
                
                // پخش صدای کلیک
                if (manager_ && manager_->GetAudioManager()) {
                    // manager_->GetAudioManager()->PlaySound2D("button_click", 0.7f);
                }
            }
        }
    }

    // پیاده‌سازی UIProgressBar
    UIProgressBar::UIProgressBar(const std::string& id, const glm::vec2& position, const glm::vec2& size)
        : UIElement(id, UIElementType::PROGRESS_BAR, position, size),
          value_(50.0f), maxValue_(100.0f), minValue_(0.0f),
          fillColor_(0.2f, 0.8f, 0.2f, 1.0f),
          backgroundColor_(0.3f, 0.3f, 0.3f, 1.0f),
          showText_(true), textFormat_("{value}/{max}")
    {
        style_.borderRadius = 3.0f;
    }

    void UIProgressBar::Render(RenderSystem* renderer)
    {
        if (!IsVisible() || !renderer) return;
        
        RenderBarBackground(renderer);
        RenderBarFill(renderer);
        
        if (showText_) {
            RenderBarText(renderer);
        }
        
        // رندر فرزندان
        for (auto& child : children_) {
            if (child->IsVisible()) {
                child->Render(renderer);
            }
        }
    }

    void UIProgressBar::RenderBarBackground(RenderSystem* renderer)
    {
        glm::vec2 absPos = GetAbsolutePosition();
        renderer->RenderRoundedQuad(absPos.x, absPos.y, size_.x, size_.y,
                                  style_.borderRadius, backgroundColor_);
    }

    void UIProgressBar::RenderBarFill(RenderSystem* renderer)
    {
        if (value_ <= minValue_) return;
        
        glm::vec2 absPos = GetAbsolutePosition();
        float fillWidth = (value_ - minValue_) / (maxValue_ - minValue_) * size_.x;
        
        renderer->RenderRoundedQuad(absPos.x, absPos.y, fillWidth, size_.y,
                                  style_.borderRadius, fillColor_);
    }

    std::string UIProgressBar::GetDisplayText() const
    {
        std::string text = textFormat_;
        
        // جایگزینی مقادیر
        size_t pos = text.find("{value}");
        if (pos != std::string::npos) {
            text.replace(pos, 7, std::to_string(static_cast<int>(value_)));
        }
        
        pos = text.find("{max}");
        if (pos != std::string::npos) {
            text.replace(pos, 5, std::to_string(static_cast<int>(maxValue_)));
        }
        
        pos = text.find("{percent}");
        if (pos != std::string::npos) {
            text.replace(pos, 9, std::to_string(static_cast<int>(GetPercentage() * 100)) + "%");
        }
        
        return text;
    }

    // پیاده‌سازی UIFactory
    UIFactory::UIFactory(UIManager* manager)
        : uiManager_(manager)
    {
        std::cout << "🏭 ایجاد فکتوری رابط کاربری" << std::endl;
    }

    std::unique_ptr<UIButton> UIFactory::CreateButton(const std::string& id, const std::string& text, 
                                                     const glm::vec2& position, const glm::vec2& size)
    {
        auto button = std::make_unique<UIButton>(id, text, position, size);
        
        // اعمال استایل پیش‌فرض
        UIStyle buttonStyle;
        buttonStyle.backgroundColor = glm::vec4(0.2f, 0.4f, 0.8f, 1.0f);
        buttonStyle.hoverColor = glm::vec4(0.3f, 0.5f, 0.9f, 1.0f);
        buttonStyle.pressedColor = glm::vec4(0.1f, 0.3f, 0.7f, 1.0f);
        buttonStyle.borderRadius = 8.0f;
        buttonStyle.fontSize = 18;
        
        button->SetStyle(buttonStyle);
        
        return button;
    }

    std::unique_ptr<UIMainMenu> UIFactory::CreateMainMenu()
    {
        auto mainMenu = std::make_unique<UIMainMenu>("main_menu", glm::vec2(0, 0), 
                                                   uiManager_->GetScreenSize());
        
        // استایل پیش‌فرض منوی اصلی
        UIStyle menuStyle;
        menuStyle.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.7f);
        mainMenu->SetStyle(menuStyle);
        
        return mainMenu;
    }

} // namespace GalacticOdyssey
