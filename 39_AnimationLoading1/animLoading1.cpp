//
//  39_AnimationLoading1
//      : Loading a single mesh object with a single texture image
//      : support .dae (COLLADA) file
//
//  Created by iklee on 5/26/24.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <mesh2.h>

// for keyframe Animation
struct KeyFrame {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    double time;
};

struct Animation {
    std::vector<KeyFrame> keyFrames;
    double duration;
    double ticksPerSecond;
};

// Source and Data directories
std::string sourceDirStr = "/Users/iklee/Library/CloudStorage/Dropbox/Lecture/Graphics/Codes/Mac2024/39_AnimationLoading1/39_AnimationLoading1";
std::string modelDirStr = "/Users/iklee/Library/CloudStorage/Dropbox/Lecture/Graphics/Codes/Mac2024/data";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

unsigned int loadTexture(const char *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else if (nrComponents != 3) {
            fprintf(stderr,"ERROR: WRONG IMAGE FILE FORMAT nrComponents: %d\n", nrComponents);
            exit(-1);
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return ID;
}

Animation loadAnimation(const aiScene* scene) {
    Animation animation;

    if (scene->HasAnimations()) {
        aiAnimation* aiAnim = scene->mAnimations[0];
        animation.duration = aiAnim->mDuration;
        animation.ticksPerSecond = aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0;

        for (unsigned int i = 0; i < aiAnim->mChannels[0]->mNumPositionKeys; i++) {
            KeyFrame keyFrame;
            keyFrame.time = aiAnim->mChannels[0]->mPositionKeys[i].mTime;
            keyFrame.position = glm::vec3(aiAnim->mChannels[0]->mPositionKeys[i].mValue.x,
                                          aiAnim->mChannels[0]->mPositionKeys[i].mValue.y,
                                          aiAnim->mChannels[0]->mPositionKeys[i].mValue.z);
            keyFrame.rotation = glm::quat(aiAnim->mChannels[0]->mRotationKeys[i].mValue.w,
                                          aiAnim->mChannels[0]->mRotationKeys[i].mValue.x,
                                          aiAnim->mChannels[0]->mRotationKeys[i].mValue.y,
                                          aiAnim->mChannels[0]->mRotationKeys[i].mValue.z);
            keyFrame.scale = glm::vec3(aiAnim->mChannels[0]->mScalingKeys[i].mValue.x,
                                       aiAnim->mChannels[0]->mScalingKeys[i].mValue.y,
                                       aiAnim->mChannels[0]->mScalingKeys[i].mValue.z);

            animation.keyFrames.push_back(keyFrame);
        }
    }

    return animation;
}

glm::mat4 interpolateTransform(const Animation& animation, double time) {
    KeyFrame prevKeyFrame, nextKeyFrame;
    for (size_t i = 0; i < animation.keyFrames.size() - 1; ++i) {
        if (time < animation.keyFrames[i + 1].time) {
            prevKeyFrame = animation.keyFrames[i];
            nextKeyFrame = animation.keyFrames[i + 1];
            break;
        }
    }

    double deltaTime = nextKeyFrame.time - prevKeyFrame.time;
    double factor = (time - prevKeyFrame.time) / deltaTime;

    glm::vec3 interpolatedPosition = glm::mix(prevKeyFrame.position, nextKeyFrame.position, factor);
    glm::quat interpolatedRotation = glm::slerp(prevKeyFrame.rotation, nextKeyFrame.rotation, static_cast<float>(factor));
    glm::vec3 interpolatedScale = glm::mix(prevKeyFrame.scale, nextKeyFrame.scale, factor);

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), interpolatedPosition);
    glm::mat4 rotation = glm::toMat4(interpolatedRotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), interpolatedScale);

    return translation * rotation * scale;
}

Mesh* loadMesh(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    aiMesh* mesh = scene->mMeshes[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    std::string texPath = modelDirStr + "/cube/dae/pexels-jonathanborba-4431922.jpg";
    unsigned int textureID = loadTexture(texPath.c_str());
    return new Mesh(vertices, indices, textureID);
}


int main() {
    // GLFW 초기화
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 버전 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(800, 600, "Animation Loading 1 - Single Mesh & Animation", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // OpenGL 초기 설정
    glEnable(GL_DEPTH_TEST);

    // 쉐이더 프로그램 설정
    std::string vs = sourceDirStr + "/animLoading1.vs";
    std::string fs = sourceDirStr + "/animLoading1.fs";
    unsigned int shaderProgram = createShaderProgram(vs.c_str(), fs.c_str());

    // 메쉬 및 애니메이션 데이터 로드
    std::string modelPath = modelDirStr + "/cube/dae/cube.dae";
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return -1;
    }

    Mesh* mesh = loadMesh(modelPath);
    if (!mesh) {
        return -1;
    }

    Animation animation = loadAnimation(scene);

    // 변환 행렬 설정
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(13.0f, 8.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // 렌더링 루프
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // 애니메이션 타임 계산
        double time = glfwGetTime();
        double animationTime = fmod(time * animation.ticksPerSecond, animation.duration);

        // 애니메이션 매트릭스 업데이트
        glm::mat4 model = interpolateTransform(animation, animationTime);
        
        // 렌더링 코드
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mesh->Draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


