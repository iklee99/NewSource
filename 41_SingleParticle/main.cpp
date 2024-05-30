// SingleParticle
//          : Mouse left button: arcball control for the camera
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

#include <shader.h>
#include <arcball.h>
#include <mass.h>
#include <plane.h>

using namespace std;

// Function Prototypes
GLFWwindow *glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action , int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
void render();
void particleInit();
void updateAnimData();

// Global variables
string sourceDirStr = "/Users/iklee/Library/CloudStorage/Dropbox/Lecture/Graphics/Codes/Mac2024/41_SingleParticle/41_SingleParticle";
GLFWwindow *mainWindow = NULL;
Shader *groundShader = NULL;
Shader *particleShader = NULL;
unsigned int SCR_WIDTH = 600;
unsigned int SCR_HEIGHT = 600;
glm::mat4 projection, view, model;

// for particle
Mass *particle;                                 // particle
float massM = 1.0f;                             // mass of the particle
float particleInitY = 10.0f;                     // initial particle's Y

// for ground
Plane *ground;                                  // ground
float groundY = 0.0f;                            // ground's y coordinates
float groundScale = 20.0f;                        // ground's scale (x and z)

// for arcball
float arcballSpeed = 0.2f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true );

// for camera
glm::vec3 cameraPos(0.0f, 10.0f, 40.0f);
glm::vec3 cameraAt(0.0f, 0.0f, 0.0f);

// for animation
enum RenderMode { INIT, ANIM, STOP };
RenderMode renderMode;                  // current rendering mode
float timeT = 0.0f;                     // current time (in sec)
float deltaT = 1.0f/30.0f;              // time interval between two consecutive frames (in sec)
int nFrame = 0;                         // current frame number


int main()
{
    mainWindow = glAllInit();
    
    // shader loading and compile (by calling the constructor)
    string vs = sourceDirStr + "/basic_lighting.vs";
    string fs = sourceDirStr + "/basic_lighting.fs";
    groundShader = new Shader(vs.c_str(), fs.c_str());
    vs = sourceDirStr + "/particle.vs";
    fs = sourceDirStr + "/particle.fs";
    particleShader = new Shader(vs.c_str(), fs.c_str());
    
    // projection matrix
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    groundShader->use();
    groundShader->setMat4("projection", projection);
    particleShader->use();
    particleShader->setMat4("projection", projection);
    
    // particle initialization
    particle = new Mass(massM);
    
    // ground initialization
    ground = new Plane(0.0f, 0.0f, 0.0f, groundScale);
    
    // initialize animation data
    particleInit();
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

void particleInit() {
    particle->setPosition(0.0, particleInitY, 0.0);
    particle->setVelocity(0.0, 0.0, 0.0);
    particle->setAcceleration(0.0, 0.0, 0.0);
}

void updateAnimData() {
    
    if (renderMode == ANIM) {
        
        if (nFrame == 0) particle->euler(timeT, deltaT, 30.0, 200.0, 0.0);
        else if (nFrame == 90) particle->euler(timeT, deltaT, 10.0, 400.0, 0.0);
        else particle->euler(timeT, deltaT, 0.0, 0.0, 0.0);
    
        timeT = timeT + deltaT;
        nFrame++;
        
        if (particle->p[1] < groundY) {
            particle->p[1] = groundY;
            renderMode = STOP;
        }
    }
}

void render() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    view = glm::lookAt(cameraPos, cameraAt, glm::vec3(0.0f, 1.0f, 0.0f));
    view = view * camArcBall.createRotationMatrix();
    model = glm::mat4(1.0);
    
    if (renderMode == ANIM) {
        updateAnimData();
    }
    
    // draw ground
    groundShader->use();
    groundShader->setMat4("view", view);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, groundY));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    groundShader->setMat4("model", model);
    ground->draw(groundShader);
    
    // draw particle
    particleShader->use();
    particleShader->setMat4("view", view);
    model = glm::mat4(1.0);
    particleShader->setMat4("model", model); 
    particle->draw(particleShader, 1.0f, 1.0f, 1.0f);
    
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
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Single Particle", NULL, NULL);
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
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
    groundShader->use();
    groundShader->setMat4("projection", projection);
    particleShader->use();
    particleShader->setMat4("projection", projection);
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
            timeT = 0.0f;
            nFrame = 0;
        }
        else if (renderMode == STOP) {
            if (particle->p[1] > groundY) renderMode = ANIM;
            else {
                renderMode = INIT;
                timeT = 0.0;
                nFrame = 0;
                particleInit();
                updateAnimData();
            }
        }
        else renderMode = STOP; // if renderMode == ANIM
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    camArcBall.mouseButtonCallback( window, button, action, mods );
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    camArcBall.cursorCallback( window, x, y );
}
