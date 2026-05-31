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

unsigned int quadScreenVAO = 0;
unsigned int quadScreenVBO;
void RenderQuad() {
    if (quadScreenVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadScreenVAO);
        glGenBuffers(1, &quadScreenVBO);
        glBindVertexArray(quadScreenVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadScreenVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadScreenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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

    Shader modelShader("shaders/vertex.glsl", "shaders/wet_fragment.glsl");
    Shader neonShader("shaders/vertex.glsl", "shaders/flickering_fragment.glsl");
    // Add this where your other shaders (modelShader, neonShader) are declared:
    Shader shadowShader("shaders/shadow_vertex.glsl", "shaders/shadow_frag.glsl", "shaders/shadow_geom.glsl");
    Shader blurShader("shaders/screen_vertex.glsl", "shaders/blur_fragment.glsl");
    Shader bloomFinalShader("shaders/screen_vertex.glsl", "shaders/bloom_final.glsl");
    stbi_set_flip_vertically_on_load(false);

        // Set up the static uniforms for the bloom composition
    bloomFinalShader.use();
    bloomFinalShader.setInt("scene", 0);
    bloomFinalShader.setInt("bloomBlur", 1);

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

    unsigned int floorTex = loadTexture("assets/textures/textures/Wet3.jpg");
    unsigned int brickTex = loadTexture("assets/textures/textures/brickwall.jpg");
    unsigned int redBrickTex = loadTexture("assets/models/textures/redbrick.jpg"); 

    // CALCULATE DYNAMIC HOUSE BOUNDS
    glm::vec3 houseSize = house.maxAABB - house.minAABB;
    glm::vec3 houseCenter = house.getCenter();


    // 1. SETUP OMNIDIRECTIONAL SHADOW MAP FBO
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. SETUP HDR MULTIPLE RENDER TARGET (MRT) FBO
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        // GL_RGBA16F is crucial for HDR so colors can exceed 1.0 for bloom
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // Add depth buffer for HDR FBO
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. SETUP PING-PONG FBOs FOR BLUR
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
    }
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ==========================================
        // CAMERA MOVEMENT & COLLISIONS
        // ==========================================
        glm::vec3 prevPos = camera.Position;
        processInput(window);

        // Clamp to outer alley bounds
        if (camera.Position.x < -1.75f) camera.Position.x = -1.75f;
        if (camera.Position.x > 12.5f)  camera.Position.x = 12.5f;
        camera.Position.y = -2.0f; // Force head-height
        if (camera.Position.z < -3.0f) camera.Position.z = -3.0f;
        if (camera.Position.z > 10.0f) camera.Position.z = 10.0f;

        // Check house collisions
        bool isInsideHouseX = (camera.Position.x > -3.0f && camera.Position.x < 9.0f);
        bool isInsideHouseZ = (camera.Position.z > -8.0f && camera.Position.z < 6.2f);
        if (isInsideHouseX && isInsideHouseZ) {
            camera.Position.x = prevPos.x;
            camera.Position.z = prevPos.z;
        }

        // ==========================================
        // MATRICES & LIGHT POSITIONS
        // ==========================================
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model;
        
        // This MUST match the bigNeonPos inside wet_fragment.glsl exactly!
        glm::vec3 bigNeonPos = glm::vec3(-(houseSize.x / 2.0f) - 5.0f, houseSize.y - 6.0f, 5.0f);
        
        // Shadow Map Matrices (6 directions for the point light)
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(bigNeonPos, bigNeonPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(bigNeonPos, bigNeonPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(bigNeonPos, bigNeonPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(bigNeonPos, bigNeonPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(bigNeonPos, bigNeonPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(bigNeonPos, bigNeonPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

        // LAMBDA TO DRAW THE ENTIRE SCENE GEOMETRY
        // We use a lambda so we don't have to write the 100+ lines of drawing code twice!
       // LAMBDA TO DRAW THE ENTIRE SCENE GEOMETRY
        auto DrawSceneGeometry = [&](Shader& defaultShader, bool isShadowPass) {
            // FLOOR
            glBindVertexArray(floorVAO); // MUST BE OUTSIDE THE IF STATEMENT!
            if(!isShadowPass) { 
                glActiveTexture(GL_TEXTURE0); 
                glBindTexture(GL_TEXTURE_2D, floorTex); 
            }
            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.9f, 0.0f));
            model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(150.0f, 150.0f, 1.0f));
            defaultShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // WALL 1
            glBindVertexArray(wallVAO); // MUST BE OUTSIDE THE IF STATEMENT!
            if(!isShadowPass) { 
                glBindTexture(GL_TEXTURE_2D, brickTex); 
            }
            model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 5.0f, 0.0f));
            model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(100.0f, 30.0f, 1.0f));
            defaultShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // WALL 2
            glBindVertexArray(wallVAO); // Ensure wall is bound
            if(!isShadowPass) {
                glBindTexture(GL_TEXTURE_2D, redBrickTex);
            }
            model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 5.0f, 12.5f));
            model = glm::rotate(model, -glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(100.0f, 30.0f, 1.0f));
            defaultShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // WALL 3 (Small Back Wall)
            glBindVertexArray(wallVAO); // Ensure wall is bound
            if(!isShadowPass) { 
                glBindTexture(GL_TEXTURE_2D, brickTex); 
            }
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, -3.0f, 9.0f));
            model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(8.0f, 6.0f, 1.0f));
            defaultShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // HOUSE
            model = glm::translate(glm::mat4(1.0f), -houseCenter);
            defaultShader.setMat4("model", model);
            house.Draw(defaultShader);
            // ------------------------------------------
            // THE GLOWING SIGNS
            // ------------------------------------------
            if(!isShadowPass) {
                neonShader.use();
                neonShader.setMat4("projection", projection);
                neonShader.setMat4("view", view);
                neonShader.setFloat("time", currentFrame);
            }
            
            model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 16.9f, houseSize.y - 9.0f, 0.0f));
            model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, -0.05f, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.004f));
            if(!isShadowPass) neonShader.setMat4("model", model); else defaultShader.setMat4("model", model);
            signModel.Draw(isShadowPass ? defaultShader : neonShader);

            model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 15.9f, houseSize.y - 9.8f, 0.0f));
            model = glm::scale(model, glm::vec3(0.3f));
            if(!isShadowPass) neonShader.setMat4("model", model); else defaultShader.setMat4("model", model);
            openModel.Draw(isShadowPass ? defaultShader : neonShader);

            if(!isShadowPass) defaultShader.use(); // SWITCH BACK TO MAIN SHADER

            // ------------------------------------------
            // REST OF MODELS
            // ------------------------------------------
            model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 15.9f, houseSize.y - 12.5f, 2.0f));
            model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.03f));
            defaultShader.setMat4("model", model);
            phoneModel.Draw(defaultShader);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, (houseSize.z / 2.0f) - 1.0f));
            model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.5f));
            defaultShader.setMat4("model", model);
            lampModel.Draw(defaultShader);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, (houseSize.z / 2.0f) - 1.0f));
            model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.5f));
            defaultShader.setMat4("model", model);
            lampModel.Draw(defaultShader);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.7f, -4.9f, 9.0f));
            model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.5f));
            defaultShader.setMat4("model", model);
            dumpsterModel.Draw(defaultShader);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.2f, -4.9f, 8.9f));
            model = glm::scale(model, glm::vec3(1.8f));
            defaultShader.setMat4("model", model);
            corpseModel.Draw(defaultShader);
            
            // ... (You can add your Tape, Barrier, Lamppost here following the exact same format!)

            // DOOR
            model = glm::translate(glm::mat4(1.0f), glm::vec3((houseSize.x / 2.0f) + houseSize.x - 15.9f, houseSize.y - 10.6f, 0.0f));
            model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.018f));
            defaultShader.setMat4("model", model);
            doorModel.Draw(defaultShader);

            // TAPE
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-63.5f, -21.7f, 43.5f));
            model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(210.0f));
            defaultShader.setMat4("model", model);
            tapeModel.Draw(defaultShader);

            // BARRIER
            model = glm::translate(glm::mat4(1.0f), glm::vec3(12.0f, -4.9f, -(houseSize.z / 2.0f)));
            model = glm::rotate(model, -glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 0.6f, 0.5f));
            defaultShader.setMat4("model", model);
            barrierModel.Draw(defaultShader);

            // LAMPPOST
            model = glm::translate(glm::mat4(1.0f), glm::vec3(4.5f, -3.1f, -(houseSize.z / 2.0f) - 3.0f));
            model = glm::scale(model, glm::vec3(1.5f));
            defaultShader.setMat4("model", model);
            lamppostModel.Draw(defaultShader);
        };

        // ==========================================
        // PHASE 1: RENDER SHADOW MAP
        // ==========================================
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        shadowShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            shadowShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        shadowShader.setFloat("far_plane", far_plane);
        shadowShader.setVec3("lightPos", bigNeonPos);
        
        DrawSceneGeometry(shadowShader, true); // true = this is the shadow pass

        // ==========================================
        // PHASE 2: RENDER HDR SCENE (MRT)
        // ==========================================
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setVec3("cameraPos", camera.Position);
        modelShader.setFloat("time", currentFrame);
        modelShader.setVec3("houseSize", houseSize);
        modelShader.setFloat("far_plane", far_plane);
        
        // Pass shadow map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        modelShader.setInt("depthMap", 1); 

        DrawSceneGeometry(modelShader, false); // false = this is the main color pass

        // ==========================================
        // PHASE 3: BLUR THE BRIGHT NEON (PING-PONG)
        // ==========================================
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            
            glActiveTexture(GL_TEXTURE0);
            // Bind the extracted bright spots first, then bounce between ping pong buffers
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]); 
            
            RenderQuad(); // Draw the fullscreen blur rect
            
            horizontal = !horizontal;
            if (first_iteration) first_iteration = false;
        }

        // ==========================================
        // PHASE 4: FINAL COMPOSITION (SCREEN)
        // ==========================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render back to the default window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        bloomFinalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]); // The gritty scene
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]); // The blurred glowing neon
        bloomFinalShader.setInt("scene", 0);
        bloomFinalShader.setInt("bloomBlur", 1);
        
        RenderQuad(); // Draw the final combined, tone-mapped image to the screen
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}