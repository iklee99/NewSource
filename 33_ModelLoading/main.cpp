// 33_ModelLoading
//     Mouse: left mouse dragging - arcball rotation
//            wheel - zooming
//     Keyboards:  r - reset camera and object position
//                 a - toggle camera/object rotations for arcball
//                 arrow left, right, up, down: panning object position

// Std. Includes
#include <string>
#include <iostream>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <shader.h>
#include <arcball.h>
#include <Model.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Function prototypes
GLFWwindow *glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action , int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void render();

// Global variables

// your source directory (folder) name (use '/' not '\' or '\\' in Windows)
string sourceDirStr = "W:/Lecture/Graphics/Codes/Mac2024/33_ModelLoading/33_ModelLoading";
string modelDirStr = "W:/Lecture/Graphics/Codes/Mac2024/data";

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWwindow *mainWindow = NULL;
glm::mat4 projection;
Shader *shader = NULL;

// For model
Model *ourModel = NULL;

// for arcball
float arcballSpeed = 0.2f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true );
static Arcball modelArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
bool arcballCamRot = true;

// for camera
glm::vec3 cameraOrigPos(0.0f, 0.0f, 9.0f);
glm::vec3 cameraPos; // current position of camera
glm::vec3 modelPan(0.0f, 0.0f, 0.0f);  // model panning vector


int main( )
{
    mainWindow = glAllInit();

    // Create shader program object
    string vs = sourceDirStr + "/modelLoading.vs";
    string fs = sourceDirStr + "/modelLoading.fs";
    shader = new Shader(vs.c_str(), fs.c_str());
    
    // Load a model
    //string modelPath = modelDirStr + "/benz/gltf/benz.gltf";
    //string modelPath = modelDirStr + "/cyborg/cyborg.obj";
    string modelPath = modelDirStr + "/gyroscope/gltf/scene.gltf";
    //string modelPath = modelDirStr + "/nanosuit/nanosuit.obj";
    //string modelPath = modelDirStr + "/planet/planet.obj";
    //string modelPath = modelDirStr + "/press1/gltf/scene.gltf";
    //string modelPath = modelDirStr + "/rock/rock.obj";
    ourModel = new Model(modelPath);
    
    // Initializing projection transformation
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
    shader->use();
    shader->setMat4("projection", projection);
    cameraPos = cameraOrigPos;
    
    // Game loop
    while( !glfwWindowShouldClose( mainWindow ) )
    {
        glfwPollEvents( );
        
        render();
        
        glfwSwapBuffers( mainWindow );
    }
    
    glfwTerminate( );
    
    return 0;
}

void render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    shader->use();
    
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    view = view * camArcBall.createRotationMatrix();
    shader->setMat4("view", view);
    
    // Draw the loaded model
    glm::mat4 model(1.0);
    
    // Rotate model by arcball and panning
    model = glm::translate( model, modelPan);
    model = model * modelArcBall.createRotationMatrix();
    
    // It's a bit too big for our scene, so scale it down
    //model = glm::scale( model, glm::vec3( 0.2f, 0.2f, 0.2f ) );
    
    shader->setMat4("model", model);
    
    ourModel->Draw(*shader );
}

GLFWwindow *glAllInit()
{
    // Initialize GLFW
    glfwInit( );
    
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loading", NULL, NULL);
    
    if ( nullptr == window )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );
        exit(-1);
    }
    
    glfwMakeContextCurrent( window );
    
    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(0);
    }
    
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);
    
    // OpenGL initialization stuffs
    glViewport( 0, 0, SCR_WIDTH, SCR_HEIGHT );
    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
    glEnable( GL_DEPTH_TEST );
    
    return(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->use();
    shader->setMat4("projection", projection); 
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        modelArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        cameraPos = cameraOrigPos;
        modelPan[0] = modelPan[1] = modelPan[2] = 0.0f;
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        arcballCamRot = !arcballCamRot;
        if (arcballCamRot) {
            cout << "ARCBALL: Camera rotation mode" << endl;
        }
        else {
            cout << "ARCBALL: Model  rotation mode" << endl;
        }
    }
    else if (key == GLFW_KEY_LEFT) {
        modelPan[0] -= 0.1;
    }
    else if (key == GLFW_KEY_RIGHT) {
        modelPan[0] += 0.1;
    }
    else if (key == GLFW_KEY_DOWN) {
        modelPan[1] -= 0.1;
    }
    else if (key == GLFW_KEY_UP) {
        modelPan[1] += 0.1;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (arcballCamRot)
        camArcBall.mouseButtonCallback( window, button, action, mods );
    else
        modelArcBall.mouseButtonCallback( window, button, action, mods );
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    if (arcballCamRot)
        camArcBall.cursorCallback( window, x, y );
    else
        modelArcBall.cursorCallback( window, x, y );
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    cameraPos[2] -= (yoffset * 0.5);
}
