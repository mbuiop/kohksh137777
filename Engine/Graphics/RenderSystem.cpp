#include "RenderSystem.h"
#include "Engine/Core/GameEngine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// کتابخانه‌های مدل‌سازی
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace GalacticOdyssey;

// پیاده‌سازی ساختارهای ریاضی
glm::mat4 Transform::ToMatrix() const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), 
        glm::vec3(position.x, position.y, position.z));
    glm::mat4 rotationMat = rotation.ToMatrix();
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), 
        glm::vec3(scale.x, scale.y, scale.z));
    
    return translation * rotationMat * scaleMat;
}

void Transform::LookAt(const Vector3& target, const Vector3& up) {
    glm::vec3 glmPos(position.x, position.y, position.z);
    glm::vec3 glmTarget(target.x, target.y, target.z);
    glm::vec3 glmUp(up.x, up.y, up.z);
    
    glm::mat4 lookAtMat = glm::lookAt(glmPos, glmTarget, glmUp);
    rotation = Quaternion(0, 0, 0, 1); // ساده‌سازی
}

Quaternion Quaternion::FromEuler(float pitch, float yaw, float roll) {
    // ساده‌سازی - پیاده‌سازی کامل نیازمند محاسبات مثلثاتی
    return Quaternion(0, 0, 0, 1);
}

glm::mat4 Quaternion::ToMatrix() const {
    // ساده‌سازی - پیاده‌سازی کامل نیازمند محاسبات کواترنیون
    return glm::mat4(1.0f);
}

// پیاده‌سازی کلاس Shader
Shader::Shader(const std::string& name) : name_(name), programID_(0) {}

Shader::~Shader() {
    if (programID_ != 0) {
        glDeleteProgram(programID_);
    }
}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = LoadShaderSource(vertexPath);
    std::string fragmentSource = LoadShaderSource(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "❌ خطا در بارگذاری سورس شیدر" << std::endl;
        return false;
    }
    
    return LoadFromSource(vertexSource, fragmentSource);
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return false;
    }
    
    programID_ = glCreateProgram();
    glAttachShader(programID_, vertexShader);
    glAttachShader(programID_, fragmentShader);
    glLinkProgram(programID_);
    
    // بررسی خطای لینک
    GLint success;
    glGetProgramiv(programID_, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(programID_, 512, nullptr, infoLog);
        std::cerr << "❌ خطای لینک شیدر: " << infoLog << std::endl;
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    std::cout << "✅ شیدر '" << name_ << "' با موفقیت ایجاد شد" << std::endl;
    return true;
}

void Shader::Use() const {
    glUseProgram(programID_);
}

void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(GetUniformLocation(name), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVector2(const std::string& name, const glm::vec2& value) const {
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void Shader::SetVector3(const std::string& name, const glm::vec3& value) const {
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void Shader::SetVector4(const std::string& name, const glm::vec4& value) const {
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void Shader::SetMatrix4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

GLuint Shader::CompileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    // بررسی خطای کامپایل
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "❌ خطای کامپایل شیدر: " << infoLog << std::endl;
        return 0;
    }
    
    return shader;
}

std::string Shader::LoadShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "❌ خطا در باز کردن فایل شیدر: " << filePath << std::endl;
        return "";
    }
    
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    
    return stream.str();
}

GLint Shader::GetUniformLocation(const std::string& name) const {
    auto it = uniformLocations_.find(name);
    if (it != uniformLocations_.end()) {
        return it->second;
    }
    
    GLint location = glGetUniformLocation(programID_, name.c_str());
    uniformLocations_[name] = location;
    return location;
}

// پیاده‌سازی کلاس Texture
Texture::Texture(const std::string& name) : name_(name), textureID_(0), width_(0), height_(0), 
                                           format_(GL_RGBA), hasMipmaps_(false) {}

Texture::~Texture() {
    if (textureID_ != 0) {
        glDeleteTextures(1, &textureID_);
    }
}

bool Texture::LoadFromFile(const std::string& filePath) {
    // در اینجا از کتابخانه‌ای مانند SOIL یا stb_image استفاده می‌شود
    // برای سادگی، یک بافت ساده ایجاد می‌کنیم
    return CreateEmpty(256, 256);
}

bool Texture::CreateEmpty(int width, int height, GLenum format) {
    width_ = width;
    height_ = height;
    format_ = format;
    
    glGenTextures(1, &textureID_);
    glBindTexture(GL_TEXTURE_2D, textureID_);
    
    GLenum internalFormat = GetInternalFormat(format);
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    
    SetWrapMode(GL_REPEAT, GL_REPEAT);
    SetFilterMode(GL_LINEAR, GL_LINEAR);
    
    std::cout << "✅ بافت '" << name_ << "' ایجاد شد: " << width << "x" << height << std::endl;
    return true;
}

void Texture::Bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID_);
}

void Texture::SetWrapMode(GLenum sWrap, GLenum tWrap) {
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
}

void Texture::SetFilterMode(GLenum minFilter, GLenum magFilter) {
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

GLenum Texture::GetInternalFormat(GLenum format) const {
    switch (format) {
        case GL_RED: return GL_R8;
        case GL_RG: return GL_RG8;
        case GL_RGB: return GL_RGB8;
        case GL_RGBA: return GL_RGBA8;
        default: return GL_RGBA8;
    }
}

// پیاده‌سازی کلاس Mesh
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const Material& material)
    : vertices_(vertices), indices_(indices), material_(material) {
    SetupMesh();
}

Mesh::~Mesh() {
    if (VAO_ != 0) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
    }
}

void Mesh::SetupMesh() {
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);
    
    glBindVertexArray(VAO_);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), &indices_[0], GL_STATIC_DRAW);
    
    ConfigureVertexAttributes();
    
    glBindVertexArray(0);
}

void Mesh::ConfigureVertexAttributes() {
    // موقعیت
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // نرمال
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // مختصات بافت
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    
    // مماس
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    
    // دو مماس
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
}

void Mesh::Render(const Shader& shader) const {
    // تنظیم متریال
    shader.SetMaterial("material", material_);
    
    // رندر مش
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// پیاده‌سازی کلاس Model3D
Model3D::Model3D() {}

Model3D::~Model3D() {
    meshes_.clear();
}

bool Model3D::LoadFromFile(const std::string& filePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "❌ خطا در بارگذاری مدل: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    directory_ = filePath.substr(0, filePath.find_last_of('/'));
    ProcessNode(scene->mRootNode, scene);
    
    std::cout << "✅ مدل بارگذاری شد: " << filePath << " (" << meshes_.size() << " مش)" << std::endl;
    return true;
}

void Model3D::ProcessNode(aiNode* node, const aiScene* scene) {
    // پردازش تمام مش‌های گره
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(ProcessMesh(mesh, scene));
    }
    
    // پردازش گره‌های فرزند
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> Model3D::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    Material material;
    
    // پردازش رأس‌ها
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // موقعیت
        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );
        
        // نرمال
        if (mesh->HasNormals()) {
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }
        
        // مختصات بافت
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        } else {
            vertex.texCoords = glm::vec2(0.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    // پردازش اندیس‌ها
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    
    // پردازش متریال
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
        
        aiColor3D color(0.0f, 0.0f, 0.0f);
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        material.diffuse = glm::vec3(color.r, color.g, color.b);
        
        aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
        material.specular = glm::vec3(color.r, color.g, color.b);
        
        float shininess;
        aiMat->Get(AI_MATKEY_SHININESS, shininess);
        material.shininess = shininess;
    }
    
    return std::make_unique<Mesh>(vertices, indices, material);
}

void Model3D::Render(const Shader& shader) const {
    for (const auto& mesh : meshes_) {
        mesh->Render(shader);
    }
}

// پیاده‌سازی کلاس Camera
Camera::Camera(const std::string& name, float fov, float aspect, float near, float far)
    : name_(name), fov_(fov), aspectRatio_(aspect), nearPlane_(near), farPlane_(far), isDirty_(true) {
    RecalculateMatrices();
}

void Camera::Update() {
    if (isDirty_) {
        RecalculateMatrices();
    }
}

void Camera::SetPosition(const Vector3& position) {
    transform_.position = position;
    isDirty_ = true;
}

void Camera::SetRotation(const Quaternion& rotation) {
    transform_.rotation = rotation;
    isDirty_ = true;
}

void Camera::LookAt(const Vector3& target, const Vector3& up) {
    transform_.LookAt(target, up);
    isDirty_ = true;
}

void Camera::RecalculateMatrices() {
    // ماتریس view
    glm::vec3 position(transform_.position.x, transform_.position.y, transform_.position.z);
    glm::vec3 forward = GetForward();
    glm::vec3 up = GetUp();
    
    viewMatrix_ = glm::lookAt(position, position + forward, up);
    
    // ماتریس projection
    projectionMatrix_ = glm::perspective(glm::radians(fov_), aspectRatio_, nearPlane_, farPlane_);
    
    isDirty_ = false;
}

Vector3 Camera::GetForward() const {
    // ساده‌سازی - در پیاده‌سازی کامل از کواترنیون استفاده می‌شود
    return Vector3(0, 0, -1);
}

Vector3 Camera::GetRight() const {
    return Vector3(1, 0, 0);
}

Vector3 Camera::GetUp() const {
    return Vector3(0, 1, 0);
}

// پیاده‌سازی کلاس RenderSystem
RenderSystem::RenderSystem(GameEngine* engine) 
    : engine_(engine), mainCamera_(nullptr), clearColor_(0.1f, 0.1f, 0.1f),
      wireframeMode_(false), depthTesting_(true), faceCulling_(true) {
    std::cout << "🎨 ایجاد سیستم رندر" << std::endl;
}

RenderSystem::~RenderSystem() {
    Cleanup();
}

bool RenderSystem::Initialize() {
    std::cout << "🔧 در حال راه‌اندازی سیستم رندر..." << std::endl;
    
    // تنظیمات OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // ایجاد شیدرهای پیش‌فرض
    SetupDefaultShaders();
    
    // ایجاد دوربین پیش‌فرض
    SetupDefaultCamera();
    
    // ایجاد سیستم پس‌پردازش
    postProcessor_ = std::make_unique<PostProcessor>();
    auto settings = engine_->GetGraphicsSettings();
    postProcessor_->Initialize(settings.screenWidth, settings.screenHeight);
    
    // تنظیم UBO نورها
    SetupLightsUBO();
    
    std::cout << "✅ سیستم رندر با موفقیت راه‌اندازی شد" << std::endl;
    return true;
}

void RenderSystem::Cleanup() {
    std::cout << "🧹 پاکسازی سیستم رندر..." << std::endl;
    
    shaders_.clear();
    textures_.clear();
    cameras_.clear();
    particleSystems_.clear();
    
    if (lightsUBO_ != 0) {
        glDeleteBuffers(1, &lightsUBO_);
    }
    
    if (postProcessor_) {
        postProcessor_.reset();
    }
    
    std::cout << "✅ سیستم رندر پاکسازی شد" << std::endl;
}

void RenderSystem::SetupDefaultShaders() {
    // شیدر پایه
    const char* basicVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoords = aTexCoords;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
    
    const char* basicFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        
        uniform vec3 viewPos;
        uniform sampler2D texture_diffuse1;
        
        struct Material {
            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
            float shininess;
        };
        
        struct Light {
            vec3 position;
            vec3 color;
            float intensity;
        };
        
        uniform Material material;
        uniform Light light;
        
        void main() {
            // نور محیطی
            vec3 ambient = material.ambient * light.color;
            
            // نور منتشر
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(light.position - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = light.color * diff * material.diffuse;
            
            // نور specular
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
            vec3 specular = light.color * spec * material.specular;
            
            vec3 result = (ambient + diffuse + specular) * light.intensity;
            FragColor = vec4(result, 1.0) * texture(texture_diffuse1, TexCoords);
        }
    )";
    
    auto basicShader = std::make_unique<Shader>("basic");
    if (basicShader->LoadFromSource(basicVertexShader, basicFragmentShader)) {
        shaders_["basic"] = std::move(basicShader);
    }
}

void RenderSystem::SetupDefaultCamera() {
    mainCamera_ = CreateCamera("MainCamera", 60.0f, 16.0f/9.0f);
    mainCamera_->SetPosition(Vector3(0, 0, 10));
}

void RenderSystem::SetupLightsUBO() {
    glGenBuffers(1, &lightsUBO_);
    glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * 16, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsUBO_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Shader* RenderSystem::CreateShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    auto shader = std::make_unique<Shader>(name);
    if (shader->LoadFromFiles(vertexPath, fragmentPath)) {
        Shader* result = shader.get();
        shaders_[name] = std::move(shader);
        return result;
    }
    return nullptr;
}

Shader* RenderSystem::GetShader(const std::string& name) {
    auto it = shaders_.find(name);
    return it != shaders_.end() ? it->second.get() : nullptr;
}

Texture* RenderSystem::CreateTexture(const std::string& name, const std::string& filePath) {
    auto texture = std::make_unique<Texture>(name);
    if (texture->LoadFromFile(filePath)) {
        Texture* result = texture.get();
        textures_[name] = std::move(texture);
        return result;
    }
    return nullptr;
}

Texture* RenderSystem::GetTexture(const std::string& name) {
    auto it = textures_.find(name);
    return it != textures_.end() ? it->second.get() : nullptr;
}

Camera* RenderSystem::CreateCamera(const std::string& name, float fov, float aspect) {
    auto camera = std::make_unique<Camera>(name, fov, aspect);
    Camera* result = camera.get();
    cameras_[name] = std::move(camera);
    return result;
}

Camera* RenderSystem::GetCamera(const std::string& name) {
    auto it = cameras_.find(name);
    return it != cameras_.end() ? it->second.get() : nullptr;
}

void RenderSystem::BeginFrame() {
    if (postProcessor_) {
        postProcessor_->Begin();
    }
    
    glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (wireframeMode_) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glEnable(GL_DEPTH_TEST);
}

void RenderSystem::EndFrame() {
    RenderParticleSystems();
    
    if (postProcessor_) {
        postProcessor_->End();
        postProcessor_->Render();
    }
}

void RenderSystem::RenderModel(Model3D* model, const Transform& transform, Shader* shader) {
    if (!model || !mainCamera_) return;
    
    Shader* renderShader = shader ? shader : GetShader("basic");
    if (!renderShader) return;
    
    renderShader->Use();
    
    // تنظیم ماتریس‌ها
    glm::mat4 modelMatrix = transform.ToMatrix();
    renderShader->SetMatrix4("model", modelMatrix);
    renderShader->SetMatrix4("view", mainCamera_->GetViewMatrix());
    renderShader->SetMatrix4("projection", mainCamera_->GetProjectionMatrix());
    
    // تنظیم موقعیت دوربین
    Vector3 camPos = mainCamera_->GetTransform().position;
    renderShader->SetVector3("viewPos", glm::vec3(camPos.x, camPos.y, camPos.z));
    
    // رندر مدل
    model->Render(*renderShader);
}

void RenderSystem::RenderParticleSystems() {
    for (auto& system : particleSystems_) {
        if (system && mainCamera_) {
            system->Render(*mainCamera_);
        }
    }
}

void RenderSystem::OnWindowResize(int width, int height) {
    glViewport(0, 0, width, height);
    
    if (mainCamera_) {
        mainCamera_->SetAspectRatio(static_cast<float>(width) / height);
    }
    
    if (postProcessor_) {
        postProcessor_->Resize(width, height);
    }
}

// پیاده‌سازی stub برای سایر کلاس‌ها
ParticleSystem::ParticleSystem() : particleTexture_(nullptr), emissionRate_(10.0f), timeSinceLastEmission_(0.0f) {}
ParticleSystem::~ParticleSystem() {}
bool ParticleSystem::Initialize() { return true; }
void ParticleSystem::Update(float deltaTime) {}
void ParticleSystem::Render(const Camera& camera) {}

PostProcessor::PostProcessor() : bloomEnabled_(true), exposure_(1.0f), gamma_(2.2f) {}
PostProcessor::~PostProcessor() {}
bool PostProcessor::Initialize(int width, int height) { return true; }
void PostProcessor::Begin() {}
void PostProcessor::End() {}
void PostProcessor::Render() {}
void PostProcessor::Resize(int width, int height) {}

} // namespace GalacticOdyssey
