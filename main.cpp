#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <importer/importer.h>
#include <shader/shader.h>

#include <iostream>
#include <chrono>
#include <cmath>

#include <nanogui/screen.h>
#include <nanogui/label.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>

bool cursor = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
long long getCurrentTime();

float iTurn = 0.0f;

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
const float GLIDER_TURN = 0.5f;
const float GLIDER_ROLL = 0.5f;
const float ROLL_EPSILON = 0.01;
const float CAMERA_Z_DISTANCE = 15.0f;
const float CAMERA_Y_DISTANCE = 3.0f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 2.5f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 worldFront  = glm::vec3(0.0f, 0.0f, -1.0f);

glm::vec3 gliderPos   = glm::vec3(0.0f, 0.0f, -10.0f);
glm::vec3 gliderFront = cameraFront;
glm::vec3 gliderUp    = cameraUp;

float gliderSpeed = 10.5f;
float gliderTurnSpeed = GLIDER_TURN;
float gliderRollSpeed = GLIDER_ROLL;

bool firstMouse = true;
float yaw   =  0.0f;	
float roll =  0.0f;
float maxRoll = glm::pi<float>() / 2 - 0.4f;
float fov   =  65.0f;

float deltaTime = 0.0f;	
long long lastFrame = getCurrentTime();

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Glider Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader("shaders/vert.glsl", "shaders/frag.glsl");
    Shader cloudShader("shaders/cloudVert.glsl", "shaders/cloudFrag.glsl");
    
    unsigned int VBO, VAO;
    unsigned int VBO2, VAO2;
    unsigned int texture1;
    glm::vec3 positions[10] = {
        glm::vec3( 10.0f,  10.0f,  10.0f),
        glm::vec3( 20.0f,  50.0f, -150.0f),
        glm::vec3(-10.5f, -20.2f, -20.5f),
        glm::vec3(-30.8f, -20.0f, -120.3f),
        glm::vec3( 20.4f, -10.4f, -30.5f),
        glm::vec3(-10.7f,  30.0f, -70.5f),
        glm::vec3( 10.3f, -20.0f, -20.5f),
        glm::vec3( 10.5f,  20.0f, -20.5f),
        glm::vec3( 10.5f,  10.2f, -10.5f),
        glm::vec3(-10.3f,  10.0f, -10.5f)
    };

    float cloudPlane[18] = {
        -1.0f,  1.0f,  -1.0f, 
        -1.0f, -1.0f,  -1.0f, 
        1.0f, -1.0f,  -1.0f, 
        -1.0f,  1.0f,  -1.0f,
        1.0f, -1.0f,  -1.0f, 
        1.0f,  1.0f,  -1.0f, 
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    
    std::vector<float> vertices = Importer::importOBJ("models/glider.obj", true);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/Glider.png", &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(VAO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, cloudPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    shader.use();
    shader.setInt("texture1", 0);

    nanogui::init();
    {

    nanogui::Screen *screen = new nanogui::Screen();
    screen->initialize(window, true);
    screen->setSize(nanogui::Vector2i(800, 500));

    nanogui::Window *windowNano = new nanogui::Window(screen, "Controls");
    windowNano->setSize(nanogui::Vector2i(200, 100));
    windowNano->setLayout(new nanogui::GroupLayout());

    nanogui::Label *label = new nanogui::Label(windowNano, "", "sans-bold");
    label->setCaption("A/D - turn left/right\n");
    nanogui::Label *label2 = new nanogui::Label(windowNano, "", "sans-bold");
    label2->setCaption("ESCAPE - toggle mouse\n");   
    screen->setVisible(true);
    

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        long long currentFrame = getCurrentTime();
        deltaTime = (currentFrame - lastFrame) / 1000.0f;
        lastFrame = currentFrame;

        processInput(window);
        
        glClearColor(0.37f, 0.63f, 0.74f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        shader.use();

        gliderFront = glm::vec3(glm::sin(yaw), 0.0f, -glm::cos(yaw));

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        shader.setMat4("projection", projection);

        glm::mat4 model = glm::mat4(1.0f);

        gliderPos += gliderFront * gliderSpeed * deltaTime;
        model = glm::translate(model, gliderPos);
        model = glm::rotate(model, roll, gliderFront);
        model = glm::rotate(model, -yaw, gliderUp);
        shader.setMat4("model", model);

        glm::vec3 newCameraPos = gliderPos - gliderFront * CAMERA_Z_DISTANCE;
        newCameraPos.y += CAMERA_Y_DISTANCE;

        glm::mat4 view = glm::lookAt(newCameraPos, newCameraPos + gliderFront, gliderUp);
        shader.setMat4("view", view);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f); 
            model = glm::translate(model, positions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }

        cloudShader.use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        cloudShader.setFloat("iTime", glfwGetTime());
        
        if (roll){
            iTurn += roll * deltaTime;
        }
        cloudShader.setFloat("iTurn", iTurn);
        cloudShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 18);
        glDisable(GL_BLEND);

        windowNano->setPosition(nanogui::Vector2i(10, SCR_HEIGHT / screen->pixelRatio() - 110));
        screen->drawContents();
        screen->drawWidgets();
        screen->performLayout();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }

    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    nanogui::shutdown();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        roll -= gliderRollSpeed * deltaTime;
        yaw -= gliderTurnSpeed * deltaTime * -roll;
        if (roll < -maxRoll){
            roll = -maxRoll;
            gliderRollSpeed = 0.0f;
        }
        else{
            gliderRollSpeed = GLIDER_ROLL;
        }
    } 
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        roll += gliderRollSpeed * deltaTime;
        yaw += gliderTurnSpeed * deltaTime * roll;
        if (roll > maxRoll){
            roll = maxRoll;
            gliderRollSpeed = 0.0f;
        }
        else{
            gliderRollSpeed = GLIDER_ROLL;
        }
    }
    else{
        gliderRollSpeed = GLIDER_ROLL;
        if (roll > 0.0f){
            roll -= gliderRollSpeed * deltaTime;
            yaw += gliderTurnSpeed * deltaTime * roll;
        }
        if (roll < 0.0f){
            roll += gliderRollSpeed * deltaTime;
            yaw -= gliderTurnSpeed * deltaTime * -roll;
        }
        if (glm::abs(roll) < ROLL_EPSILON){
            roll = 0.0f;
        }
    }

    yaw = std::fmod(yaw, 2 * glm::pi<float>());
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        cursor = !cursor;
        glfwSetInputMode(window, GLFW_CURSOR, cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

long long getCurrentTime(){
    auto currentTime = std::chrono::system_clock::now();
    auto timeSinceEpoch = currentTime.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch);
    return millis.count();
}