#pragma once
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace GalacticOdyssey {

    // ساختارهای ریاضی پیشرفته
    struct Vector3 {
        float x, y, z;
        Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
        
        Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
        Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
        Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
        float Length() const { return sqrt(x*x + y*y + z*z); }
        Vector3 Normalized() const { float len = Length(); return len > 0 ? Vector3(x/len, y/len, z/len) : Vector3(); }
    };

    struct Quaternion {
        float x, y, z, w;
        Quaternion(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
        
        static Quaternion FromEuler(float pitch, float yaw, float roll);
        glm::mat4 ToMatrix() const;
    };

    struct Transform {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
        
        Transform() : position(0,0,0), rotation(), scale(1,1,1) {}
        
        glm::mat4 ToMatrix() const;
        void LookAt(const Vector3& target, const Vector3& up = Vector3(0,1,0));
    };

    // ساختارهای گرافیکی
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        
        Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex) 
            : position(pos), normal(norm), texCoords(tex), tangent(0), bitangent(0) {}
    };

    struct Material {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shininess;
        float opacity;
        
        std::string diffuseMap;
        std::string normalMap;
        std::string specularMap;
        std::string emissionMap;
        
        Material() : ambient(0.1f), diffuse(0.8f), specular(1.0f), shininess(32.0f), opacity(1.0f) {}
    };

    struct Light {
        enum Type { DIRECTIONAL, POINT, SPOT };
        
        Type type;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        float range;
        float spotAngle;
        
        Light() : type(DIRECTIONAL), position(0), direction(0, -1, 0), color(1), 
                 intensity(1.0f), range(10.0f), spotAngle(45.0f) {}
    };

    // کلاس شیدر پیشرفته
    class Shader {
    private:
        GLuint programID_;
        std::string name_;
        std::unordered_map<std::string, GLint> uniformLocations_;
        
    public:
        Shader(const std::string& name);
        ~Shader();
        
        bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
        bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
        
        void Use() const;
        void SetBool(const std::string& name, bool value) const;
        void SetInt(const std::string& name, int value) const;
        void SetFloat(const std::string& name, float value) const;
        void SetVector2(const std::string& name, const glm::vec2& value) const;
        void SetVector3(const std::string& name, const glm::vec3& value) const;
        void SetVector4(const std::string& name, const glm::vec4& value) const;
        void SetMatrix4(const std::string& name, const glm::mat4& value) const;
        void SetMaterial(const std::string& prefix, const Material& material) const;
        void SetLight(const std::string& prefix, const Light& light) const;
        
        GLuint GetProgramID() const { return programID_; }
        const std::string& GetName() const { return name_; }
        
    private:
        GLint GetUniformLocation(const std::string& name) const;
        GLuint CompileShader(const std::string& source, GLenum type);
        std::string LoadShaderSource(const std::string& filePath);
    };

    // کلاس بافت پیشرفته
    class Texture {
    private:
        GLuint textureID_;
        std::string name_;
        int width_, height_;
        GLenum format_;
        bool hasMipmaps_;
        
    public:
        Texture(const std::string& name);
        ~Texture();
        
        bool LoadFromFile(const std::string& filePath);
        bool LoadFromMemory(const unsigned char* data, int width, int height, GLenum format);
        bool CreateEmpty(int width, int height, GLenum format = GL_RGBA);
        bool CreateCubemap(const std::vector<std::string>& facePaths);
        
        void Bind(GLuint unit = 0) const;
        void GenerateMipmaps();
        void SetWrapMode(GLenum sWrap, GLenum tWrap);
        void SetFilterMode(GLenum minFilter, GLenum magFilter);
        
        GLuint GetID() const { return textureID_; }
        const std::string& GetName() const { return name_; }
        int GetWidth() const { return width_; }
        int GetHeight() const { return height_; }
        
    private:
        GLenum GetInternalFormat(GLenum format) const;
    };

    // کلاس مدل سه بعدی
    class Mesh {
    private:
        GLuint VAO_, VBO_, EBO_;
        std::vector<Vertex> vertices_;
        std::vector<GLuint> indices_;
        Material material_;
        
    public:
        Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const Material& material);
        ~Mesh();
        
        void Render(const Shader& shader) const;
        void SetupMesh();
        
        const std::vector<Vertex>& GetVertices() const { return vertices_; }
        const std::vector<GLuint>& GetIndices() const { return indices_; }
        const Material& GetMaterial() const { return material_; }
        
    private:
        void ConfigureVertexAttributes();
    };

    class Model3D {
    private:
        std::vector<std::unique_ptr<Mesh>> meshes_;
        std::string directory_;
        Transform transform_;
        
    public:
        Model3D();
        ~Model3D();
        
        bool LoadFromFile(const std::string& filePath);
        void Render(const Shader& shader) const;
        void SetTransform(const Transform& transform) { transform_ = transform; }
        
        const Transform& GetTransform() const { return transform_; }
        const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return meshes_; }
        
    private:
        void ProcessNode(struct aiNode* node, const struct aiScene* scene);
        std::unique_ptr<Mesh> ProcessMesh(struct aiMesh* mesh, const struct aiScene* scene);
        std::vector<Texture> LoadMaterialTextures(struct aiMaterial* mat, int aiTextureType, const std::string& typeName);
    };

    // کلاس دوربین پیشرفته
    class Camera {
    private:
        std::string name_;
        Transform transform_;
        
        float fov_;
        float aspectRatio_;
        float nearPlane_;
        float farPlane_;
        
        glm::mat4 viewMatrix_;
        glm::mat4 projectionMatrix_;
        bool isDirty_;
        
    public:
        Camera(const std::string& name, float fov = 60.0f, float aspect = 16.0f/9.0f, float near = 0.1f, float far = 1000.0f);
        
        void Update();
        void SetPosition(const Vector3& position);
        void SetRotation(const Quaternion& rotation);
        void LookAt(const Vector3& target, const Vector3& up = Vector3(0,1,0));
        
        void SetFOV(float fov) { fov_ = fov; isDirty_ = true; }
        void SetAspectRatio(float aspect) { aspectRatio_ = aspect; isDirty_ = true; }
        void SetClippingPlanes(float near, float far) { nearPlane_ = near; farPlane_ = far; isDirty_ = true; }
        
        const glm::mat4& GetViewMatrix() const { return viewMatrix_; }
        const glm::mat4& GetProjectionMatrix() const { return projectionMatrix_; }
        const Transform& GetTransform() const { return transform_; }
        const std::string& GetName() const { return name_; }
        
        Vector3 GetForward() const;
        Vector3 GetRight() const;
        Vector3 GetUp() const;
        
    private:
        void RecalculateMatrices();
    };

    // سیستم ذرات پیشرفته
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float size;
        float life;
        float maxLife;
        float rotation;
        float rotationSpeed;
        
        Particle() : position(0), velocity(0), color(1), size(1), life(0), maxLife(1), rotation(0), rotationSpeed(0) {}
    };

    class ParticleSystem {
    private:
        std::vector<Particle> particles_;
        GLuint VAO_, VBO_;
        std::unique_ptr<Shader> particleShader_;
        Texture* particleTexture_;
        
        glm::vec3 emitterPosition_;
        float emissionRate_;
        float timeSinceLastEmission_;
        
    public:
        ParticleSystem();
        ~ParticleSystem();
        
        bool Initialize();
        void Update(float deltaTime);
        void Render(const Camera& camera);
        
        void SetEmitterPosition(const glm::vec3& position) { emitterPosition_ = position; }
        void SetEmissionRate(float rate) { emissionRate_ = rate; }
        void SetParticleTexture(Texture* texture) { particleTexture_ = texture; }
        
        void EmitParticle(const Particle& particle);
        void EmitBurst(int count, const glm::vec3& position, const glm::vec4& color = glm::vec4(1));
        
    private:
        void SetupRenderData();
        void EmitNewParticles(float deltaTime);
    };

    // سیستم پس‌پردازش
    class PostProcessor {
    private:
        GLuint FBO_, RBO_, colorBuffer_;
        GLuint quadVAO_, quadVBO_;
        std::unique_ptr<Shader> postProcessShader_;
        
        int screenWidth_, screenHeight_;
        bool bloomEnabled_;
        float exposure_;
        float gamma_;
        
    public:
        PostProcessor();
        ~PostProcessor();
        
        bool Initialize(int width, int height);
        void Begin();
        void End();
        void Render();
        
        void SetBloomEnabled(bool enabled) { bloomEnabled_ = enabled; }
        void SetExposure(float exposure) { exposure_ = exposure; }
        void SetGamma(float gamma) { gamma_ = gamma; }
        
        void Resize(int width, int height);
        
    private:
        void SetupFrameBuffer();
        void SetupScreenQuad();
    };

    // کلاس اصلی سیستم رندر
    class RenderSystem {
    private:
        class GameEngine* engine_;
        
        // شیدرهای اصلی
        std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
        std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
        
        // دوربین‌ها
        Camera* mainCamera_;
        std::unordered_map<std::string, std::unique_ptr<Camera>> cameras_;
        
        // نورها
        std::vector<Light> lights_;
        GLuint lightsUBO_;
        
        // سیستم‌های ویژه
        std::unique_ptr<PostProcessor> postProcessor_;
        std::vector<std::unique_ptr<ParticleSystem>> particleSystems_;
        
        // تنظیمات
        glm::vec3 clearColor_;
        bool wireframeMode_;
        bool depthTesting_;
        bool faceCulling_;
        
    public:
        RenderSystem(class GameEngine* engine);
        ~RenderSystem();
        
        bool Initialize();
        void Cleanup();
        
        // مدیریت شیدرها
        Shader* CreateShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        Shader* GetShader(const std::string& name);
        
        // مدیریت بافت‌ها
        Texture* CreateTexture(const std::string& name, const std::string& filePath);
        Texture* GetTexture(const std::string& name);
        
        // مدیریت دوربین‌ها
        Camera* CreateCamera(const std::string& name, float fov = 60.0f, float aspect = 16.0f/9.0f);
        Camera* GetCamera(const std::string& name);
        void SetMainCamera(Camera* camera) { mainCamera_ = camera; }
        Camera* GetMainCamera() const { return mainCamera_; }
        
        // مدیریت نورها
        void AddLight(const Light& light);
        void ClearLights();
        void UpdateLightsUBO();
        
        // سیستم ذرات
        ParticleSystem* CreateParticleSystem();
        
        // رندرینگ
        void BeginFrame();
        void EndFrame();
        void RenderModel(Model3D* model, const Transform& transform, Shader* shader = nullptr);
        void RenderSkybox(Texture* cubemapTexture);
        void RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
        
        // تنظیمات
        void SetClearColor(const glm::vec3& color) { clearColor_ = color; }
        void SetWireframeMode(bool enabled);
        void SetDepthTesting(bool enabled);
        void SetFaceCulling(bool enabled);
        
        void OnWindowResize(int width, int height);
        void ApplySettings(const struct GraphicsSettings& settings);
        
    private:
        void SetupDefaultShaders();
        void SetupDefaultCamera();
        void SetupLightsUBO();
        void RenderParticleSystems();
    };

    // کلاس رابط کاربری
    class UIRenderer {
    private:
        GLuint VAO_, VBO_, EBO_;
        std::unique_ptr<Shader> uiShader_;
        std::unique_ptr<Texture> fontTexture_;
        
        struct Character {
            GLuint textureID;
            glm::ivec2 size;
            glm::ivec2 bearing;
            unsigned int advance;
        };
        std::unordered_map<char, Character> characters_;
        
    public:
        UIRenderer();
        ~UIRenderer();
        
        bool Initialize();
        void RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
        void RenderQuad(float x, float y, float width, float height, const glm::vec4& color);
        void RenderTexture(float x, float y, float width, float height, Texture* texture);
        
    private:
        bool LoadFont(const std::string& fontPath, int fontSize);
        void SetupRenderData();
    };

    // افکت‌های ویژه
    class SpecialEffects {
    private:
        RenderSystem* renderSystem_;
        
    public:
        SpecialEffects(RenderSystem* renderSystem);
        
        void CreateExplosionEffect(const glm::vec3& position, float scale = 1.0f);
        void CreateTrailEffect(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
        void CreateShockwave(const glm::vec3& center, float radius, float duration);
        void CreateLightning(const glm::vec3& start, const glm::vec3& end, int segments = 10);
        void CreateNebulaField(const glm::vec3& center, float radius, int particleCount);
        
    private:
        void CreateParticleRing(const glm::vec3& center, float radius, int count, const glm::vec4& color);
    };

} // namespace GalacticOdyssey

#endif // RENDER_SYSTEM_H
