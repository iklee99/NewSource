// 38_KeyframeAnimation
//          : Mouse left button: arcball control for the object
//          : Keyboard 'r': to reset the arcball
//          : Keyboard 'space' : to start/stop/init the animation

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <cmath>
#include <cstring>

#include <shader.h>
#include <cube.h>
#include <arcball.h>
#include <keyframe.h>

using namespace std;

// Function Prototypes
GLFWwindow *glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action , int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
void render();
void initKeyframes();
void updateAnimData();

// Global variables

// your source directory (folder) name (use '/' not '\' or '\\' in Windows)
string sourceDirStr =
"/Users/iklee/Library/CloudStorage/Dropbox/Lecture/Graphics/Codes/Mac2024/38_KeyframeAnimation/38_KeyframeAnimation";

GLFWwindow *mainWindow = NULL;
Shader *globalShader = NULL;
unsigned int SCR_WIDTH = 600;
unsigned int SCR_HEIGHT = 600;
Cube *cube;
glm::mat4 projection, view, model;

// for arcball
float arcballSpeed = 0.2f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true );

// for lighting
glm::vec3 lightSize(0.2f, 0.2f, 0.2f);
glm::vec3 lightPos(1.2f, 1.0f, 7.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
float ambientStrenth = 0.2f;

// for camera
glm::vec3 cameraPos(0.0f, 0.0f, 20.0f);
glm::vec3 cameraAt(0.0f, 0.0f, 0.0f);

// for animation
enum RenderMode { INIT, ANIM, STOP };
RenderMode renderMode;                  // current rendering mode
float beginT;                           // animation beginning time (in sec)
float timeT;                            // current time (in sec)
float animEndTime = 5.0f;               // ending time of animation (in sec)
float xTrans, yTrans, zTrans;           // current translation factors
float xAngle, yAngle, zAngle;           // current rotation factors
KeyFraming xTKF(4), yTKF(4), zTKF(4);   // translation keyframes
KeyFraming xRKF(4), yRKF(4), zRKF(4);   // rotation keyframes


int main()
{
    mainWindow = glAllInit();
    
    // shader loading and compile (by calling the constructor)
    string vs = sourceDirStr + "/basic_lighting.vs";
    string fs = sourceDirStr + "/basic_lighting.fs";
    globalShader = new Shader(vs.c_str(), fs.c_str());
    
    // lighting parameters to fragment shader
    globalShader->use();
    globalShader->setVec3("objectColor", objectColor);
    globalShader->setVec3("lightColor", lightColor);
    globalShader->setVec3("lightPos", lightPos);
    globalShader->setFloat("ambientStrenth", ambientStrenth);
    
    // projection matrix
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    globalShader->setMat4("projection", projection);
    
    // cube initialization
    cube = new Cube();
    
    // initialize animation data
    initKeyframes();
    timeT = 0.0f;
    updateAnimData();
    renderMode = INIT;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(mainWindow)) {
        render();
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

void initKeyframes() {
    
    // x-translation keyframes
    xTKF.setKey(0, 0, -5.0);
    xTKF.setKey(1, 1.5, -2.0);
    xTKF.setKey(2, 3.0, 2.0);
    xTKF.setKey(3, animEndTime, 5.0);
    
    // y-translation keyframes
    yTKF.setKey(0, 0, 3.0);
    yTKF.setKey(1, 1.5, -2.0);
    yTKF.setKey(2, 3.0, 2.0);
    yTKF.setKey(3, animEndTime, -3.0);
    
    // z-translation keyframes
    zTKF.setKey(0, 0, 1.0);
    zTKF.setKey(1, 1.5, -2.0);
    zTKF.setKey(2, 3.0, 2.0);
    zTKF.setKey(3, animEndTime, 0.0);
    
    // x-rotation keyframes
    xRKF.setKey(0, 0, 0.0);
    xRKF.setKey(1, 1.5, 20.0);
    xRKF.setKey(2, 3.0, 80.0);
    xRKF.setKey(3, animEndTime, 0.0);
    
    // y-rotation keyframes
    yRKF.setKey(0, 0, 0.0);
    yRKF.setKey(1, 1.5, -30.0);
    yRKF.setKey(2, 3.0, 50.0);
    yRKF.setKey(3, animEndTime, 0.0);
    
    // z-rotation keyframes
    zRKF.setKey(0, 0, 0.0);
    zRKF.setKey(1, 1.5, 90.0);
    zRKF.setKey(2, 3.0, 180.0);
    zRKF.setKey(3, animEndTime, 200.0);
}

void updateAnimData() {
    if (timeT > animEndTime) {
        renderMode = STOP;
        timeT = animEndTime;
    }
    xTrans = xTKF.getValLinear(timeT);
    yTrans = yTKF.getValLinear(timeT);
    zTrans = zTKF.getValLinear(timeT);
    xAngle = xRKF.getValLinear(timeT);
    yAngle = yRKF.getValLinear(timeT);
    zAngle = zRKF.getValLinear(timeT);
}

void render() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    view = glm::lookAt(cameraPos, cameraAt, glm::vec3(0.0f, 1.0f, 0.0f));
    view = view * camArcBall.createRotationMatrix();
    
    if (renderMode == ANIM) {
        float cTime = (float)glfwGetTime(); // current time
        timeT = cTime - beginT;
        updateAnimData();
    }
    
    // view matrix to fragment shader
    globalShader->use();
    globalShader->setMat4("view", view);
    
    // cube object
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans, yTrans, zTrans));
    glm::vec3 eulerAngles(glm::radians(xAngle), glm::radians(yAngle), glm::radians(zAngle));
    glm::quat q(eulerAngles);
    glm::mat4 rotMatrix = q.operator glm::mat4x4();
    model = model * rotMatrix;
    globalShader->setMat4("model", model);
    cube->draw(globalShader);
    
    // swap buffers
    glfwSwapBuffers(mainWindow);
}


GLFWwindow *glAllInit()
{
    GLFWwindow *window;
    
    // glfw: initialize and configure
    if (!glfwInit()) {
        printf("GLFW initialisation failed!");
        glfwTerminate();
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Keyframe Animation", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(0);
    }
    
    // OpenGL states
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    return window;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    globalShader->use();
    globalShader->setMat4("projection", projection);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (renderMode == INIT) {
            renderMode = ANIM;
            beginT = glfwGetTime();
        }
        else if (renderMode == STOP) {
            if (timeT == animEndTime) renderMode = INIT;
            else {
                renderMode = ANIM;
            }
        }
        else if (renderMode == ANIM) renderMode = STOP;
        
        if (renderMode == INIT) {
            timeT = 0.0;
            updateAnimData();
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    camArcBall.mouseButtonCallback( window, button, action, mods );
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    camArcBall.cursorCallback( window, x, y );
}
