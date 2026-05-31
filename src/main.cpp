#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.hpp"

#define CGLTF_IMPLEMENTATION
// #include "cgltf.h"

#include "headers/Shaders.h"
#include "headers/Camera.h"
#include "headers/GLTFModel.h"

#include <iostream>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Spawn camera exactly at the requested position
Camera camera(glm::vec3(10.0f, -2.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Final CGLTF Alleyway", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    Shader modelShader("shaders/vertex.glsl", "shaders/fragment3.glsl");
    Shader neonShader("shaders/vertex.glsl", "shaders/flickering_frag.glsl");
    stbi_set_flip_vertically_on_load(false);

    // LOAD ALL MODELS
    GLTFModel house("assets/models/house_scene.gltf");
    GLTFModel signModel("assets/models/sign_scene.gltf");
    GLTFModel phoneModel("assets/models/telephone_scene.gltf");
    GLTFModel doorModel("assets/models/door_scene.gltf");
    GLTFModel openModel("assets/models/neon_open_scene.gltf");
    GLTFModel lampModel("assets/models/lamp_scene.gltf");
    GLTFModel dumpsterModel("assets/models/dumpster_scene.gltf");
    GLTFModel corpseModel("assets/models/corpse_scene.gltf");
    GLTFModel tapeModel("assets/models/tape_scene.gltf");
    GLTFModel barrierModel("assets/models/barrier_scene.gltf");
    GLTFModel lamppostModel("assets/models/lamppost_scene.gltf");

    // ENVIRONMENT QUADS
   // ==========================================
    // ENVIRONMENT QUADS (With Texture Repeats Baked In)
    // ==========================================
    
    // FLOOR VERTICES (15x15 Repeat)
    float floorVertices[] = {
         // Positions         // Normals         // UVs (Repeated 15x15)
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  15.0f, 15.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  15.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f,  15.0f
    };

    // WALL VERTICES (10x3 Repeat)
    float wallVertices[] = {
         // Positions         // Normals         // UVs (Repeated 10x3)
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  10.0f, 3.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  10.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f,  3.0f
    };
    
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

    // Setup Floor VAO
    unsigned int floorVAO, floorVBO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // Setup Wall VAO
    unsigned int wallVAO, wallVBO, wallEBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    unsigned int floorTex = loadTexture("assets/textures/textures/pavement.jpg");
    unsigned int brickTex = loadTexture("assets/textures/textures/brickwall.jpg");
    unsigned int redBrickTex = loadTexture("assets/models/textures/redbrick.jpg"); 

    // CALCULATE DYNAMIC HOUSE BOUNDS
    glm::vec3 houseSize = house.maxAABB - house.minAABB;
    glm::vec3 houseCenter = house.getCenter();
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 1. SAVE PREVIOUS POSITION BEFORE MOVING
        glm::vec3 prevPos = camera.Position;

        // 2. PROCESS MOVEMENT
        processInput(window);

        // 3. CLAMP TO OUTER ALLEY BOUNDS
        if (camera.Position.x < -1.75f) camera.Position.x = -1.75f;
        if (camera.Position.x > 12.5f)  camera.Position.x = 12.5f;

        camera.Position.y = -2.0f; // Force camera head-height to stay exactly at -2.0

        if (camera.Position.z < -3.0f) camera.Position.z = -3.0f;
        if (camera.Position.z > 10.0f) camera.Position.z = 10.0f;

        // 4. CHECK HOUSE COLLISIONS (The Solid Object)
        // If the camera's new X and Z are BOTH inside the house box...
        bool isInsideHouseX = (camera.Position.x > -3.0f && camera.Position.x < 9.0f);
        bool isInsideHouseZ = (camera.Position.z > -8.0f && camera.Position.z < 6.2f);

        if (isInsideHouseX && isInsideHouseZ) {
            // ...we bumped into the house! Revert back to the previous safe position.
            camera.Position.x = prevPos.x;
            camera.Position.z = prevPos.z;
        }

        // --- RENDER SCENE ---
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // ... (The rest of your rendering code stays exactly the same)

        modelShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", camera.GetViewMatrix());

        glm::mat4 model;
        modelShader.setInt("hasTexture", 1); 


        // ==========================================
        // 1. SCENE ENVIRONMENT (Walls & Floor)
        // ==========================================

        
        // FLOOR

        modelShader.use();
        modelShader.setVec3("cameraPos", camera.Position);
        modelShader.setFloat("time", static_cast<float>(glfwGetTime()));
        modelShader.setVec3("houseSize", houseSize);

        glBindVertexArray(floorVAO); // Bind the 15x15 repeat VAO
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.9f, 0.0f));
        model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(150.0f, 150.0f, 1.0f));
        modelShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, floorTex);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // WALL 1 (Right Wall)
        glBindVertexArray(wallVAO); // Bind the 10x3 repeat VAO for all walls
        model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 5.0f, 0.0f));
        model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f, 30.0f, 1.0f));
        modelShader.setMat4("model", model);
        glBindTexture(GL_TEXTURE_2D, brickTex);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // WALL 2 (Left Wall)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 5.0f, 12.5f));
        model = glm::rotate(model, -glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f, 30.0f, 1.0f));
        modelShader.setMat4("model", model);
        glBindTexture(GL_TEXTURE_2D, redBrickTex);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // WALL 3 (Small Back Wall)
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, -3.0f, 9.0f));
        model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 6.0f, 1.0f));
        modelShader.setMat4("model", model);
        glBindTexture(GL_TEXTURE_2D, brickTex);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        // ==========================================
        // 2. SCENE MODELS
        // ==========================================

        // HOUSE
        model = glm::translate(glm::mat4(1.0f), -houseCenter);
        modelShader.setMat4("model", model);
        house.Draw(modelShader);

        // ------------------------------------------
        // SWITCH TO NEON FLICKER SHADER FOR SIGNS
        // ------------------------------------------
        neonShader.use();
        neonShader.setMat4("projection", projection);
        neonShader.setMat4("view", camera.GetViewMatrix());
        neonShader.setFloat("time", static_cast<float>(glfwGetTime())); // Pass time uniform
        // SIGN
        model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 16.9f, houseSize.y - 9.0f, 0.0f));
        model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, -0.05f, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.004f));
        neonShader.setMat4("model", model);
        signModel.Draw(neonShader);

        // ------------------------------------------
        // SWITCH BACK TO STANDARD SHADER FOR REST
        // ------------------------------------------
        modelShader.use();

        // TELEPHONE
        model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 15.9f, houseSize.y - 12.5f, 2.0f));
        model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.03f));
        modelShader.setMat4("model", model);
        phoneModel.Draw(modelShader);

        // DOOR
        model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 15.9f, houseSize.y - 10.6f, 0.0f));
        model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.018f));
        modelShader.setMat4("model", model);
        doorModel.Draw(modelShader);

        // NEON OPEN SIGN
        model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 15.9f, houseSize.y - 9.8f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f));
        modelShader.setMat4("model", model);
        openModel.Draw(modelShader);

        // LAMP 1
        model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, (houseSize.z / 2.0f) - 1.0f));
        model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        modelShader.setMat4("model", model);
        lampModel.Draw(modelShader);

        // LAMP 2
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, (houseSize.z / 2.0f) - 1.0f));
        model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        modelShader.setMat4("model", model);
        lampModel.Draw(modelShader);

        // DUMPSTER
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.7f, -4.9f, 9.0f));
        model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        modelShader.setMat4("model", model);
        dumpsterModel.Draw(modelShader);

        // CORPSE
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.2f, -4.9f, 8.9f));
        model = glm::scale(model, glm::vec3(1.5f));
        modelShader.setMat4("model", model);
        corpseModel.Draw(modelShader);

        // TAPE
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, -17.7f, 35.0f));
        model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(160.0f));
        modelShader.setMat4("model", model);
        tapeModel.Draw(modelShader);

        // BARRIER
        model = glm::translate(glm::mat4(1.0f), glm::vec3(12.0f, -4.9f, -(houseSize.z / 2.0f)));
        model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f));
        modelShader.setMat4("model", model);
        barrierModel.Draw(modelShader);

        // LAMPPOST
        model = glm::translate(glm::mat4(1.0f), glm::vec3(4.5f, -3.1f, -(houseSize.z / 2.0f) - 3.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        modelShader.setMat4("model", model);
        lamppostModel.Draw(modelShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}