#include "AudioManager.h"
#include <iostream>
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

namespace GalacticOdyssey {

    // پیاده‌سازی AudioManager (Singleton)
    AudioManager* AudioManager::instance_ = nullptr;

    AudioManager::AudioManager()
        : audioDevice_(0), initialized_(false),
          masterVolume_(1.0f), musicVolume_(0.7f), effectsVolume_(0.8f), voiceVolume_(1.0f),
          muted_(false), paused_(false),
          activeSources_(0), totalSources_(0), cpuUsage_(0.0f)
    {
        std::cout << "🎵 ایجاد مدیر صدا" << std::endl;
    }

    AudioManager::~AudioManager()
    {
        Cleanup();
    }

    AudioManager& AudioManager::GetInstance()
    {
        if (!instance_) {
            instance_ = new AudioManager();
        }
        return *instance_;
    }

    void AudioManager::DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    bool AudioManager::Initialize()
    {
        std::cout << "🔊 در حال راه‌اندازی سیستم صوتی..." << std::endl;
        
        if (initialized_) {
            std::cout << "⚠️ سیستم صوتی قبلاً راه‌اندازی شده است" << std::endl;
            return true;
        }
        
        // راه‌اندازی SDL Audio
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            std::cerr << "❌ خطا در راه‌اندازی SDL Audio: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // ایجاد کانال‌های پیش‌فرض
        CreateChannel("Master");
        CreateChannel("Music");
        CreateChannel("Effects");
        CreateChannel("Voice");
        CreateChannel("Ambient");
        
        // راه‌اندازی افکت‌ها
        globalReverb_.InitializePresets();
        
        std::cout << "✅ سیستم صوتی با موفقیت راه‌اندازی شد" << std::endl;
        initialized_ = true;
        return true;
    }

    void AudioManager::Cleanup()
    {
        std::cout << "🧹 پاکسازی سیستم صوتی..." << std::endl;
        
        if (audioDevice_ != 0) {
            SDL_CloseAudioDevice(audioDevice_);
            audioDevice_ = 0;
        }
        
        UnloadAllSounds();
        channels_.clear();
        
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        
        initialized_ = false;
        std::cout << "✅ سیستم صوتی پاکسازی شد" << std::endl;
    }

    void AudioManager::Update(float deltaTime)
    {
        if (!initialized_ || paused_) return;
        
        // به‌روزرسانی کانال‌ها
        for (auto& channel : channels_) {
            channel.second->Update(deltaTime);
        }
        
        // محاسبه اثرات سه بعدی
        Calculate3DEffects();
        
        // به‌روزرسانی آمار
        UpdateStatistics(deltaTime);
    }

    bool AudioManager::InitializeSDLAudio()
    {
        SDL_AudioSpec desired, obtained;
        
        SDL_zero(desired);
        desired.freq = 44100;
        desired.format = AUDIO_F32;
        desired.channels = 2;
        desired.samples = 4096;
        desired.callback = AudioManager::SDLAudioCallback;
        desired.userdata = this;
        
        audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
        if (audioDevice_ == 0) {
            std::cerr << "❌ خطا در باز کردن دستگاه صوتی: " << SDL_GetError() << std::endl;
            return false;
        }
        
        audioSpec_ = obtained;
        SDL_PauseAudioDevice(audioDevice_, 0); // شروع پخش
        
        std::cout << "🎛️ دستگاه صوتی راه‌اندازی شد: " 
                  << obtained.freq << "Hz, " 
                  << obtained.channels << " channels, "
                  << obtained.samples << " samples" << std::endl;
        
        return true;
    }

    void AudioManager::SDLAudioCallback(void* userdata, Uint8* stream, int len)
    {
        AudioManager* manager = static_cast<AudioManager*>(userdata);
        manager->AudioCallback(stream, len);
    }

    void AudioManager::AudioCallback(Uint8* stream, int len)
    {
        // پاکسازی استریم خروجی
        SDL_memset(stream, 0, len);
        
        // میکس تمام منابع صوتی فعال
        for (auto& sourcePair : audioSources_) {
            auto& source = sourcePair.second;
            if (source->GetState() == AudioState::PLAYING) {
                // در پیاده‌سازی کامل، داده‌های صوتی میکس می‌شوند
            }
        }
        
        // اعمال افکت‌های جهانی
        if (globalReverb_.IsEnabled()) {
            // globalReverb_.Process(reinterpret_cast<float*>(stream), len / sizeof(float) / audioSpec_.channels, audioSpec_.channels);
        }
    }

    std::shared_ptr<AudioSource> AudioManager::LoadSound(const std::string& name, const std::string& filePath)
    {
        if (audioSources_.find(name) != audioSources_.end()) {
            std::cout << "⚠️ صدا از قبل بارگذاری شده است: " << name << std::endl;
            return audioSources_[name];
        }
        
        auto sound = std::make_shared<WaveAudio>(name);
        if (sound->LoadFromFile(filePath)) {
            audioSources_[name] = sound;
            totalSources_++;
            std::cout << "✅ صدا بارگذاری شد: " << name << std::endl;
            return sound;
        }
        
        std::cerr << "❌ خطا در بارگذاری صدا: " << name << std::endl;
        return nullptr;
    }

    std::shared_ptr<AudioSource> AudioManager::LoadMusic(const std::string& name, const std::string& filePath)
    {
        if (audioSources_.find(name) != audioSources_.end()) {
            std::cout << "⚠️ موسیقی از قبل بارگذاری شده است: " << name << std::endl;
            return audioSources_[name];
        }
        
        auto music = std::make_shared<StreamingAudio>(name);
        if (music->LoadFromFile(filePath)) {
            audioSources_[name] = music;
            totalSources_++;
            std::cout << "✅ موسیقی بارگذاری شد: " << name << std::endl;
            return music;
        }
        
        std::cerr << "❌ خطا در بارگذاری موسیقی: " << name << std::endl;
        return nullptr;
    }

    std::shared_ptr<AudioSource> AudioManager::GetSound(const std::string& name)
    {
        auto it = audioSources_.find(name);
        return it != audioSources_.end() ? it->second : nullptr;
    }

    void AudioManager::UnloadSound(const std::string& name)
    {
        auto it = audioSources_.find(name);
        if (it != audioSources_.end()) {
            it->second->Stop();
            audioSources_.erase(it);
            totalSources_--;
            std::cout << "🗑️ صدا حذف شد: " << name << std::endl;
        }
    }

    void AudioManager::UnloadAllSounds()
    {
        for (auto& source : audioSources_) {
            source.second->Stop();
        }
        audioSources_.clear();
        totalSources_ = 0;
        std::cout << "🗑️ تمام صداها حذف شدند" << std::endl;
    }

    AudioChannel* AudioManager::CreateChannel(const std::string& name)
    {
        if (channels_.find(name) != channels_.end()) {
            std::cout << "⚠️ کانال از قبل وجود دارد: " << name << std::endl;
            return channels_[name].get();
        }
        
        auto channel = std::make_unique<AudioChannel>(name);
        AudioChannel* result = channel.get();
        channels_[name] = std::move(channel);
        
        std::cout << "✅ کانال ایجاد شد: " << name << std::endl;
        return result;
    }

    AudioChannel* AudioManager::GetChannel(const std::string& name)
    {
        auto it = channels_.find(name);
        return it != channels_.end() ? it->second.get() : nullptr;
    }

    void AudioManager::PlaySound2D(const std::string& name, float volume)
    {
        auto sound = GetSound(name);
        if (sound) {
            sound->Set3DEnabled(false);
            sound->SetVolume(volume * effectsVolume_ * masterVolume_);
            sound->Play();
        }
    }

    void AudioManager::PlaySound3D(const std::string& name, const glm::vec3& position, float volume)
    {
        auto sound = GetSound(name);
        if (sound) {
            sound->Set3DEnabled(true);
            sound->SetPosition(position);
            sound->SetVolume(volume * effectsVolume_ * masterVolume_);
            sound->Play();
        }
    }

    void AudioManager::PlayMusic(const std::string& name, bool loop)
    {
        auto music = GetSound(name);
        if (music) {
            music->SetPlaybackMode(loop ? PlaybackMode::LOOP : PlaybackMode::ONCE);
            music->SetVolume(musicVolume_ * masterVolume_);
            music->Play();
        }
    }

    void AudioManager::SetListenerPosition(const glm::vec3& position)
    {
        listener_.position = position;
    }

    void AudioManager::SetListenerVelocity(const glm::vec3& velocity)
    {
        listener_.velocity = velocity;
    }

    void AudioManager::SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up)
    {
        listener_.SetOrientation(forward, up);
    }

    void AudioManager::SetMasterVolume(float volume)
    {
        masterVolume_ = glm::clamp(volume, 0.0f, 1.0f);
        std::cout << "🔊 حجم اصلی تنظیم شد: " << masterVolume_ << std::endl;
    }

    void AudioManager::SetMusicVolume(float volume)
    {
        musicVolume_ = glm::clamp(volume, 0.0f, 1.0f);
        std::cout << "🎵 حجم موسیقی تنظیم شد: " << musicVolume_ << std::endl;
    }

    void AudioManager::SetEffectsVolume(float volume)
    {
        effectsVolume_ = glm::clamp(volume, 0.0f, 1.0f);
        std::cout << "💥 حجم افکت‌ها تنظیم شد: " << effectsVolume_ << std::endl;
    }

    void AudioManager::SetMuted(bool muted)
    {
        muted_ = muted;
        SDL_PauseAudioDevice(audioDevice_, muted_);
        std::cout << (muted_ ? "🔇 صدا خاموش شد" : "🔊 صدا روشن شد") << std::endl;
    }

    void AudioManager::SetPaused(bool paused)
    {
        paused_ = paused;
        std::cout << (paused_ ? "⏸️ صدا متوقف شد" : "▶️ صدا ادامه یافت") << std::endl;
    }

    void AudioManager::Calculate3DEffects()
    {
        for (auto& sourcePair : audioSources_) {
            auto& source = sourcePair.second;
            if (source->Is3D() && source->GetState() == AudioState::PLAYING) {
                source->Calculate3DEffects(listener_.position);
            }
        }
    }

    void AudioManager::UpdateStatistics(float deltaTime)
    {
        // شمارش منابع فعال
        activeSources_ = 0;
        for (auto& source : audioSources_) {
            if (source.second->GetState() == AudioState::PLAYING) {
                activeSources_++;
            }
        }
        
        // محاسبه استفاده از CPU (ساده‌سازی)
        cpuUsage_ = (activeSources_ / 10.0f) * 100.0f;
        cpuUsage_ = glm::clamp(cpuUsage_, 0.0f, 100.0f);
    }

    // پیاده‌سازی AudioSource
    AudioSource::AudioSource(const std::string& name)
        : name_(name), format_(AudioFormat::WAV), mode_(PlaybackMode::ONCE),
          state_(AudioState::STOPPED), volume_(1.0f), pan_(0.0f), is3D_(false),
          length_(0), position_(0) {}

    AudioSource::~AudioSource() {}

    // پیاده‌سازی WaveAudio
    WaveAudio::WaveAudio(const std::string& name) 
        : AudioSource(name), audioBuffer_(nullptr), audioLength_(0),
          currentPosition_(0), deviceId_(0) {}

    WaveAudio::~WaveAudio()
    {
        Stop();
        if (audioBuffer_) {
            SDL_FreeWAV(audioBuffer_);
            audioBuffer_ = nullptr;
        }
    }

    bool WaveAudio::LoadFromFile(const std::string& filePath)
    {
        if (SDL_LoadWAV(filePath.c_str(), &audioSpec_, &audioBuffer_, &audioLength_) == nullptr) {
            std::cerr << "❌ خطا در بارگذاری فایل WAV: " << SDL_GetError() << std::endl;
            return false;
        }
        
        length_ = (audioLength_ * 1000) / (audioSpec_.freq * audioSpec_.channels * (SDL_AUDIO_BITSIZE(audioSpec_.format) / 8));
        
        std::cout << "✅ فایل WAV بارگذاری شد: " << filePath 
                  << " (" << length_ << "ms)" << std::endl;
        return true;
    }

    bool WaveAudio::LoadFromMemory(const void* data, size_t size)
    {
        // در پیاده‌سازی کامل از SDL_LoadWAV_RW استفاده می‌شود
        std::cout << "⚠️ بارگذاری از حافظه هنوز پیاده‌سازی نشده است" << std::endl;
        return false;
    }

    void WaveAudio::Play()
    {
        if (state_ == AudioState::PLAYING) return;
        
        state_ = AudioState::PLAYING;
        currentPosition_ = 0;
        
        std::cout << "▶️ پخش صدا: " << name_ << std::endl;
    }

    void WaveAudio::Pause()
    {
        if (state_ == AudioState::PLAYING) {
            state_ = AudioState::PAUSED;
            std::cout << "⏸️ صدا متوقف شد: " << name_ << std::endl;
        }
    }

    void WaveAudio::Stop()
    {
        state_ = AudioState::STOPPED;
        currentPosition_ = 0;
        std::cout << "⏹️ صدا متوقف شد: " << name_ << std::endl;
    }

    void WaveAudio::Update(float deltaTime)
    {
        if (state_ != AudioState::PLAYING) return;
        
        // به‌روزرسانی موقعیت پخش
        Uint32 samplesPerMs = audioSpec_.freq * audioSpec_.channels / 1000;
        currentPosition_ += static_cast<Uint32>(deltaTime * 1000 * samplesPerMs);
        
        if (currentPosition_ >= audioLength_) {
            if (mode_ == PlaybackMode::LOOP) {
                currentPosition_ = 0;
            } else {
                Stop();
            }
        }
        
        position_ = (currentPosition_ * 1000) / samplesPerMs;
    }

    void WaveAudio::AudioCallback(void* userdata, Uint8* stream, int len)
    {
        WaveAudio* audio = static_cast<WaveAudio*>(userdata);
        if (audio->state_ != AudioState::PLAYING) return;
        
        Uint32 bytesToCopy = std::min(static_cast<Uint32>(len), 
                                     audio->audioLength_ - audio->currentPosition_);
        
        if (bytesToCopy > 0) {
            SDL_MixAudioFormat(stream, 
                              audio->audioBuffer_ + audio->currentPosition_,
                              audio->audioSpec_.format,
                              bytesToCopy,
                              static_cast<int>(SDL_MIX_MAXVOLUME * audio->volume_));
            
            audio->currentPosition_ += bytesToCopy;
        }
        
        if (audio->currentPosition_ >= audio->audioLength_) {
            if (audio->mode_ == PlaybackMode::LOOP) {
                audio->currentPosition_ = 0;
            } else {
                audio->Stop();
            }
        }
    }

    void WaveAudio::ApplyEffects()
    {
        // در پیاده‌سازی کامل، افکت‌ها روی بافر صدا اعمال می‌شوند
    }

    void WaveAudio::Calculate3DEffects(const glm::vec3& listenerPos)
    {
        if (!is3D_) return;
        
        // محاسبه فاصله از شنونده
        float distance = glm::length(spatialSettings_.position - listenerPos);
        
        // محاسبه حجم بر اساس فاصله
        float distanceVolume = 1.0f;
        if (distance > spatialSettings_.minDistance) {
            float t = (distance - spatialSettings_.minDistance) / 
                     (spatialSettings_.maxDistance - spatialSettings_.minDistance);
            distanceVolume = 1.0f - glm::clamp(t, 0.0f, 1.0f);
        }
        
        // اعمال کاهش حجم
        volume_ *= distanceVolume;
        
        // محاسبه پان بر اساس موقعیت نسبی
        glm::vec3 relativePos = spatialSettings_.position - listenerPos;
        pan_ = relativePos.x / spatialSettings_.maxDistance;
        pan_ = glm::clamp(pan_, -1.0f, 1.0f);
    }

    // پیاده‌سازی AudioChannel
    AudioChannel::AudioChannel(const std::string& name)
        : name_(name), volume_(1.0f), muted_(false) {}

    AudioChannel::~AudioChannel()
    {
        RemoveAllSources();
    }

    void AudioChannel::AddSource(std::shared_ptr<AudioSource> source)
    {
        sources_.push_back(source);
    }

    void AudioChannel::RemoveSource(const std::string& sourceName)
    {
        sources_.erase(
            std::remove_if(sources_.begin(), sources_.end(),
                [&](const std::shared_ptr<AudioSource>& source) {
                    return source->GetName() == sourceName;
                }),
            sources_.end()
        );
    }

    void AudioChannel::RemoveAllSources()
    {
        sources_.clear();
    }

    void AudioChannel::SetVolume(float volume)
    {
        volume_ = glm::clamp(volume, 0.0f, 1.0f);
        
        // اعمال حجم جدید به تمام منابع
        for (auto& source : sources_) {
            source->SetVolume(volume_);
        }
    }

    void AudioChannel::Update(float deltaTime)
    {
        for (auto& source : sources_) {
            source->Update(deltaTime);
        }
    }

    // پیاده‌سازی ReverbEffect
    ReverbEffect::ReverbEffect() 
        : enabled_(false), level_(0.5f) 
    {
        InitializePresets();
    }

    void ReverbEffect::InitializePresets()
    {
        presets_["SmallRoom"] = {0.5f, 0.3f, 0.2f, 0.5f, 0.5f};
        presets_["LargeHall"] = {2.0f, 0.7f, 0.8f, 0.8f, 0.7f};
        presets_["Cave"] = {4.0f, 0.9f, 0.9f, 0.9f, 0.8f};
        presets_["Space"] = {6.0f, 0.5f, 0.3f, 0.3f, 0.2f};
    }

    void ReverbEffect::Process(float* samples, int sampleCount, int channels)
    {
        if (!enabled_ || level_ <= 0.0f) return;
        
        ApplyReverb(samples, sampleCount, channels);
    }

} // namespace GalacticOdyssey
