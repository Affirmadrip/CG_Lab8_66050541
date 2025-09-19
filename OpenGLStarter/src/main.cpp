#undef GLFW_DLL
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "Libs/Mesh.h"

const GLint WIDTH = 800, HEIGHT = 600;

Window mainWindow;                 
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

static const char* vShader = "Shaders/shader.vert";
static const char* fShader = "Shaders/shader.frag";


GLfloat gVertices[] = {
     0.0f, 1.0f,  0.0f,   
    -1.0f, 0.0f, -1.0f,  
     1.0f, 0.0f, -1.0f,  
     1.0f, 0.0f,  1.0f, 
    -1.0f, 0.0f,  1.0f    
};

unsigned int gIndices[] = {
    0,1,2,  0,2,3,  0,3,4,  0,4,1,   
    1,4,3,  1,3,2           
};

float deltaTime = 0.0f, lastFrame = 0.0f;
float fov = 45.0f;

glm::vec3 camPos   = glm::vec3(1.0f, 0.5f,  2.0f);
glm::vec3 camFront = glm::vec3(0.0f, -0.3f, -1.0f); 
glm::vec3 camUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float yaw = -90.0f;   
float pitch = 0.0f;

bool firstMouse = true;
double lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;

void processInput(GLFWwindow* window) {
    float speed = 3.0f * deltaTime;         
    glm::vec3 forward = glm::normalize(glm::vec3(camFront.x, 0.0f, camFront.z));
    glm::vec3 right   = glm::normalize(glm::cross(forward, camUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += forward * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= forward * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= right   * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += right   * speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)      camPos += camUp * speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= camUp * speed;
}

void mouse_callback(GLFWwindow* /*win*/, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = float(xpos - lastX);
    float yoffset = float(lastY - ypos); 
    lastX = xpos; lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    dir.y = sinf(glm::radians(pitch));
    dir.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    camFront = glm::normalize(dir);
}

void scroll_callback(GLFWwindow* /*win*/, double /*xoff*/, double yoff) {
    fov -= (float)yoff;
    if (fov < 1.0f)  fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}

void CreateObjects() {
    Mesh* pyr = new Mesh();
    pyr->CreateMesh(
        gVertices, gIndices,
        (unsigned int)(sizeof(gVertices) / sizeof(gVertices[0])),
        (unsigned int)(sizeof(gIndices)  / sizeof(gIndices[0]))
    );
    for (int i = 0; i < 10; ++i) meshList.push_back(pyr);
}

void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main() {
    mainWindow = Window(WIDTH, HEIGHT, 3, 3);
    mainWindow.initialise();

    glfwSetInputMode(mainWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(mainWindow.getWindow(), mouse_callback);
    glfwSetScrollCallback(mainWindow.getWindow(), scroll_callback);

    CreateObjects();
    CreateShaders();

    shaderList[0].UseShader();
    GLint uniformModel      = shaderList[0].GetUniformLocation("model");
    GLint uniformView       = shaderList[0].GetUniformLocation("view");
    GLint uniformProjection = shaderList[0].GetUniformLocation("projection");

    glm::vec3 objectPositions[10] = {
        { 0.0f,  0.0f,  -2.5f}, { 2.0f,  5.0f, -15.0f}, {-1.5f,  2.2f,  -2.5f},
        {-3.8f, -2.0f, -12.3f}, { 2.4f, -0.4f,  -3.5f}, {-1.7f,  3.0f,  -7.5f},
        { 1.3f, -2.0f,  -2.5f}, { 1.5f,  2.0f,  -2.5f}, { 1.5f,  0.2f,  -1.5f},
        {-1.3f,  1.0f,  -1.5f}
    };

    glEnable(GL_DEPTH_TEST);

    while (!mainWindow.getShouldClose()) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(mainWindow.getWindow());

        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].UseShader();

        glm::mat4 projection = glm::perspective(glm::radians(fov),
            (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(),
            0.1f, 100.0f);
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));

        for (int i = 0; i < 10; ++i) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::rotate(model, glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshList[i]->RenderMesh();
        }

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    for (auto* m : meshList) { delete m; }
    return 0;
}
