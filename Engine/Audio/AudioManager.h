#pragma once
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <atomic>

namespace GalacticOdyssey {

    // فرمت‌های صوتی پشتیبانی شده
    enum class AudioFormat {
        WAV,
        OGG,
        MP3,
        FLAC
    };

    // حالت‌های پخش
    enum class PlaybackMode {
        ONCE,           // پخش یکبار
        LOOP,           // تکرار
        STREAM          // استریمینگ
    };

    // وضعیت صدا
    enum class AudioState {
        STOPPED,
        PLAYING,
        PAUSED,
        FADING_IN,
        FADING_OUT
    };

    // ساختار تنظیمات صوتی سه بعدی
    struct Audio3DSettings {
        glm::vec3 position;         // موقعیت در فضای سه بعدی
        glm::vec3 velocity;         // سرعت برای اثر دوپلر
        float minDistance;          // حداقل فاصله برای حداکثر حجم
        float maxDistance;          // حداکثر فاصله برای حداقل حجم
        float rolloffFactor;        // عامل کاهش صدا با فاصله
        float dopplerFactor;        // عامل اثر دوپلر
        
        Audio3DSettings() 
            : position(0.0f), velocity(0.0f),
              minDistance(1.0f), maxDistance(100.0f),
              rolloffFactor(1.0f), dopplerFactor(1.0f) {}
    };

    // ساختار افکت‌های صوتی
    struct AudioEffects {
        float reverbLevel;          // سطح اکو
        float lowPass;              // فیلتر پایین‌گذر
        float highPass;             // فیلتر بالاگذر
        float pitch;                // زیر و بمی
        float distortion;           // اعوجاج
        float chorus;               // کوروس
        
        AudioEffects() 
            : reverbLevel(0.0f), lowPass(1.0f), highPass(1.0f),
              pitch(1.0f), distortion(0.0f), chorus(0.0f) {}
    };

    // کلاس صوتی پایه
    class AudioSource {
    protected:
        std::string name_;
        AudioFormat format_;
        PlaybackMode mode_;
        AudioState state_;
        
        float volume_;
        float pan_;
        bool is3D_;
        
        Audio3DSettings spatialSettings_;
        AudioEffects effects_;
        
        Uint32 length_;             // طول به میلی‌ثانیه
        Uint32 position_;           // موقعیت فعلی
        
    public:
        AudioSource(const std::string& name);
        virtual ~AudioSource();
        
        virtual bool LoadFromFile(const std::string& filePath) = 0;
        virtual bool LoadFromMemory(const void* data, size_t size) = 0;
        
        virtual void Play() = 0;
        virtual void Pause() = 0;
        virtual void Stop() = 0;
        virtual void Update(float deltaTime) = 0;
        
        // تنظیمات پایه
        void SetVolume(float volume) { volume_ = glm::clamp(volume, 0.0f, 1.0f); }
        void SetPan(float pan) { pan_ = glm::clamp(pan, -1.0f, 1.0f); }
        void SetPlaybackMode(PlaybackMode mode) { mode_ = mode; }
        void SetPitch(float pitch) { effects_.pitch = glm::max(pitch, 0.1f); }
        
        // تنظیمات سه بعدی
        void Set3DEnabled(bool enabled) { is3D_ = enabled; }
        void SetPosition(const glm::vec3& position) { spatialSettings_.position = position; }
        void SetVelocity(const glm::vec3& velocity) { spatialSettings_.velocity = velocity; }
        void SetDistanceRange(float minDist, float maxDist) {
            spatialSettings_.minDistance = minDist;
            spatialSettings_.maxDistance = maxDist;
        }
        
        // افکت‌ها
        void SetReverb(float level) { effects_.reverbLevel = glm::clamp(level, 0.0f, 1.0f); }
        void SetLowPass(float cutoff) { effects_.lowPass = glm::clamp(cutoff, 0.0f, 1.0f); }
        void SetDistortion(float amount) { effects_.distortion = glm::clamp(amount, 0.0f, 1.0f); }
        
        // اطلاعات
        const std::string& GetName() const { return name_; }
        AudioState GetState() const { return state_; }
        float GetVolume() const { return volume_; }
        Uint32 GetLength() const { return length_; }
        Uint32 GetPosition() const { return position_; }
        bool Is3D() const { return is3D_; }
        
    protected:
        virtual void ApplyEffects() = 0;
        virtual void Calculate3DEffects(const glm::vec3& listenerPos) = 0;
    };

    // کلاس صوتی برای فایل‌های WAV
    class WaveAudio : public AudioSource {
    private:
        SDL_AudioSpec audioSpec_;
        Uint8* audioBuffer_;
        Uint32 audioLength_;
        
        // برای پخش
        Uint32 currentPosition_;
        SDL_AudioDeviceID deviceId_;
        
    public:
        WaveAudio(const std::string& name);
        ~WaveAudio();
        
        bool LoadFromFile(const std::string& filePath) override;
        bool LoadFromMemory(const void* data, size_t size) override;
        
        void Play() override;
        void Pause() override;
        void Stop() override;
        void Update(float deltaTime) override;
        
    private:
        void ApplyEffects() override;
        void Calculate3DEffects(const glm::vec3& listenerPos) override;
        static void AudioCallback(void* userdata, Uint8* stream, int len);
    };

    // کلاس صوتی استریمینگ برای موسیقی طولانی
    class StreamingAudio : public AudioSource {
    private:
        struct StreamBuffer {
            Uint8* data;
            size_t size;
            size_t position;
        };
        
        std::vector<StreamBuffer> buffers_;
        SDL_AudioStream* audioStream_;
        SDL_AudioDeviceID deviceId_;
        
        bool isStreaming_;
        int bufferSize_;
        
    public:
        StreamingAudio(const std::string& name);
        ~StreamingAudio();
        
        bool LoadFromFile(const std::string& filePath) override;
        bool LoadFromMemory(const void* data, size_t size) override;
        
        void Play() override;
        void Pause() override;
        void Stop() override;
        void Update(float deltaTime) override;
        
        void QueueData(const Uint8* data, size_t size);
        
    private:
        void ApplyEffects() override;
        void Calculate3DEffects(const glm::vec3& listenerPos) override;
        bool FillBuffers();
    };

    // ساختار شنونده صوتی
    struct AudioListener {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 forward;
        glm::vec3 up;
        
        AudioListener() 
            : position(0.0f), velocity(0.0f),
              forward(0.0f, 0.0f, -1.0f), 
              up(0.0f, 1.0f, 0.0f) {}
        
        void SetOrientation(const glm::vec3& newForward, const glm::vec3& newUp) {
            forward = glm::normalize(newForward);
            up = glm::normalize(newUp);
        }
    };

    // گروه صوتی برای مدیریت دسته‌ای
    class AudioChannel {
    private:
        std::string name_;
        float volume_;
        bool muted_;
        std::vector<std::shared_ptr<AudioSource>> sources_;
        
    public:
        AudioChannel(const std::string& name);
        ~AudioChannel();
        
        void AddSource(std::shared_ptr<AudioSource> source);
        void RemoveSource(const std::string& sourceName);
        void RemoveAllSources();
        
        void SetVolume(float volume);
        void SetMuted(bool muted);
        void FadeTo(float targetVolume, float duration);
        
        void Update(float deltaTime);
        
        const std::string& GetName() const { return name_; }
        float GetVolume() const { return volume_; }
        bool IsMuted() const { return muted_; }
        size_t GetSourceCount() const { return sources_.size(); }
    };

    // سیستم اکو و محیط
    class ReverbEffect {
    private:
        struct ReverbPreset {
            float decayTime;
            float earlyReflections;
            float lateReverb;
            float density;
            float diffusion;
        };
        
        std::unordered_map<std::string, ReverbPreset> presets_;
        bool enabled_;
        float level_;
        
    public:
        ReverbEffect();
        ~ReverbEffect();
        
        void InitializePresets();
        void SetPreset(const std::string& presetName);
        void SetEnabled(bool enabled) { enabled_ = enabled; }
        void SetLevel(float level) { level_ = glm::clamp(level, 0.0f, 1.0f); }
        
        void Process(float* samples, int sampleCount, int channels);
        
    private:
        void ApplyReverb(float* samples, int sampleCount, int channels);
    };

    // سیستم فیلتر صوتی
    class AudioFilter {
    private:
        enum class FilterType {
            LOW_PASS,
            HIGH_PASS,
            BAND_PASS,
            NOTCH
        };
        
        FilterType type_;
        float cutoff_;
        float resonance_;
        
        // ضرایب فیلتر
        float a0_, a1_, a2_, b1_, b2_;
        float x1_, x2_, y1_, y2_;
        
    public:
        AudioFilter(FilterType type = FilterType::LOW_PASS);
        ~AudioFilter();
        
        void SetType(FilterType type);
        void SetCutoff(float cutoff);
        void SetResonance(float resonance);
        
        void Process(float* samples, int sampleCount);
        void CalculateCoefficients(float sampleRate);
        
    private:
        float ProcessSample(float sample);
    };

    // کلاس اصلی مدیریت صدا
    class AudioManager {
    private:
        static AudioManager* instance_;
        
        // دستگاه صوتی
        SDL_AudioSpec audioSpec_;
        SDL_AudioDeviceID audioDevice_;
        bool initialized_;
        
        // منابع صوتی
        std::unordered_map<std::string, std::shared_ptr<AudioSource>> audioSources_;
        std::unordered_map<std::string, std::unique_ptr<AudioChannel>> channels_;
        
        // شنونده
        AudioListener listener_;
        
        // افکت‌های جهانی
        ReverbEffect globalReverb_;
        AudioFilter globalFilter_;
        
        // تنظیمات
        float masterVolume_;
        float musicVolume_;
        float effectsVolume_;
        float voiceVolume_;
        
        bool muted_;
        bool paused_;
        
        // آمار
        int activeSources_;
        int totalSources_;
        float cpuUsage_;
        
    public:
        AudioManager();
        ~AudioManager();
        
        static AudioManager& GetInstance();
        static void DestroyInstance();
        
        bool Initialize();
        void Cleanup();
        void Update(float deltaTime);
        
        // مدیریت منابع صوتی
        std::shared_ptr<AudioSource> LoadSound(const std::string& name, const std::string& filePath);
        std::shared_ptr<AudioSource> LoadMusic(const std::string& name, const std::string& filePath);
        std::shared_ptr<AudioSource> GetSound(const std::string& name);
        void UnloadSound(const std::string& name);
        void UnloadAllSounds();
        
        // مدیریت کانال‌ها
        AudioChannel* CreateChannel(const std::string& name);
        AudioChannel* GetChannel(const std::string& name);
        void RemoveChannel(const std::string& name);
        
        // پخش سریع
        void PlaySound2D(const std::string& name, float volume = 1.0f);
        void PlaySound3D(const std::string& name, const glm::vec3& position, float volume = 1.0f);
        void PlayMusic(const std::string& name, bool loop = true);
        
        // تنظیمات شنونده
        void SetListenerPosition(const glm::vec3& position);
        void SetListenerVelocity(const glm::vec3& velocity);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
        
        // تنظیمات حجم
        void SetMasterVolume(float volume);
        void SetMusicVolume(float volume);
        void SetEffectsVolume(float volume);
        void SetVoiceVolume(float volume);
        void SetMuted(bool muted);
        void SetPaused(bool paused);
        
        // افکت‌های جهانی
        void SetGlobalReverb(bool enabled, float level = 0.5f);
        void SetGlobalReverbPreset(const std::string& presetName);
        void SetGlobalFilter(float cutoff, float resonance = 0.5f);
        
        // اطلاعات
        const AudioListener& GetListener() const { return listener_; }
        float GetMasterVolume() const { return masterVolume_; }
        float GetCPUUsage() const { return cpuUsage_; }
        int GetActiveSourceCount() const { return activeSources_; }
        bool IsInitialized() const { return initialized_; }
        
    private:
        bool InitializeSDLAudio();
        void Calculate3DEffects();
        void UpdateStatistics(float deltaTime);
        void AudioCallback(Uint8* stream, int len);
        static void SDLAudioCallback(void* userdata, Uint8* stream, int len);
    };

    // سیستم مدیریت صداهای محیطی
    class AmbientSoundSystem {
    private:
        struct AmbientZone {
            glm::vec3 center;
            float radius;
            std::string soundName;
            float volume;
            bool enabled;
        };
        
        std::vector<AmbientZone> zones_;
        std::unordered_map<std::string, std::shared_ptr<AudioSource>> ambientSounds_;
        AudioManager* audioManager_;
        
    public:
        AmbientSoundSystem(AudioManager* audioManager);
        ~AmbientSoundSystem();
        
        void AddZone(const glm::vec3& center, float radius, const std::string& soundName, float volume = 1.0f);
        void RemoveZone(const glm::vec3& center);
        void Update(const glm::vec3& listenerPosition);
        void SetZoneEnabled(const glm::vec3& center, bool enabled);
        
    private:
        float CalculateZoneVolume(const AmbientZone& zone, const glm::vec3& listenerPosition);
    };

    // سیستم صداهای تعاملی
    class InteractiveAudioSystem {
    private:
        struct AudioEvent {
            std::string soundName;
            glm::vec3 position;
            float volume;
            float delay;
            bool is3D;
        };
        
        std::vector<AudioEvent> eventQueue_;
        AudioManager* audioManager_;
        
    public:
        InteractiveAudioSystem(AudioManager* audioManager);
        ~InteractiveAudioSystem();
        
        void QueueSound(const std::string& soundName, const glm::vec3& position = glm::vec3(0.0f), 
                       float volume = 1.0f, float delay = 0.0f, bool is3D = true);
        void PlayImmediate(const std::string& soundName, const glm::vec3& position = glm::vec3(0.0f), 
                          float volume = 1.0f, bool is3D = true);
        void Update(float deltaTime);
        void ClearQueue();
        
    private:
        void ProcessEvent(const AudioEvent& event);
    };

    // سیستم دیالوگ و صداهای گفتاری
    class DialogueSystem {
    private:
        struct DialogueLine {
            std::string soundName;
            std::string text;
            float duration;
            bool isPlaying;
            float timer;
        };
        
        std::vector<DialogueLine> dialogueQueue_;
        AudioManager* audioManager_;
        std::function<void(const std::string&)> onDialogueStart_;
        std::function<void(const std::string&)> onDialogueEnd_;
        
    public:
        DialogueSystem(AudioManager* audioManager);
        ~DialogueSystem();
        
        void QueueDialogue(const std::string& soundName, const std::string& text, float duration);
        void PlayDialogue(const std::string& soundName, const std::string& text, float duration);
        void Update(float deltaTime);
        void SkipCurrent();
        void ClearQueue();
        
        void SetDialogueStartCallback(std::function<void(const std::string&)> callback) {
            onDialogueStart_ = callback;
        }
        
        void SetDialogueEndCallback(std::function<void(const std::string&)> callback) {
            onDialogueEnd_ = callback;
        }
        
        bool IsPlaying() const;
        
    private:
        void StartDialogue(DialogueLine& line);
        void EndDialogue(DialogueLine& line);
    };

    // سیستم میکس صدا برای افکت‌های پیشرفته
    class AudioMixer {
    private:
        struct MixerChannel {
            std::string name;
            float volume;
            float pan;
            std::vector<float*> inputs;
            std::vector<float> output;
        };
        
        std::vector<MixerChannel> channels_;
        int sampleRate_;
        int channels_;
        
    public:
        AudioMixer(int sampleRate = 44100, int channels = 2);
        ~AudioMixer();
        
        void AddChannel(const std::string& name, float volume = 1.0f, float pan = 0.0f);
        void RemoveChannel(const std::string& name);
        void SetChannelVolume(const std::string& name, float volume);
        void SetChannelPan(const std::string& name, float pan);
        
        void Process(float* output, int sampleCount);
        
    private:
        void MixSamples(float* output, int sampleCount);
        void ApplyPan(float* samples, int sampleCount, float pan);
    };

    // سیستم ضبط و پخش صدا
    class VoiceChatSystem {
    private:
        SDL_AudioDeviceID captureDevice_;
        std::vector<float> captureBuffer_;
        bool isRecording_;
        bool isPlaying_;
        
        AudioManager* audioManager_;
        std::string currentUser_;
        
    public:
        VoiceChatSystem(AudioManager* audioManager);
        ~VoiceChatSystem();
        
        bool StartRecording(const std::string& user);
        void StopRecording();
        void PlayRecordedAudio();
        bool IsRecording() const { return isRecording_; }
        bool IsPlaying() const { return isPlaying_; }
        
    private:
        static void CaptureCallback(void* userdata, Uint8* stream, int len);
        void ProcessCapturedAudio(const float* samples, int sampleCount);
    };

} // namespace GalacticOdyssey

#endif // AUDIO_MANAGER_H
