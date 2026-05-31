#ifndef GLTFMODEL_H
#define GLTFMODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cgltf.h"
#include "Shaders.h"
#include "stb_image.hpp"
#include <vector>
#include <string>
#include <iostream>

unsigned int TextureFromFile(const char *path, const std::string &directory) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "GLTF Texture failed to load: " << filename << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

struct MeshPrimitive {
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    bool hasIndices;
    unsigned int textureID;
};

class GLTFModel {
public:
    std::vector<MeshPrimitive> primitives;
    std::string directory;
    glm::vec3 minAABB;
    glm::vec3 maxAABB;

    // Helper to get the exact center of the model (Replaces Three.js Box3.getCenter)
    glm::vec3 getCenter() {
        return (minAABB + maxAABB) / 2.0f;
    }

    GLTFModel(const char* path) {
        std::string pathStr(path);
        directory = pathStr.substr(0, pathStr.find_last_of('/'));
        minAABB = glm::vec3(999999.0f);
        maxAABB = glm::vec3(-999999.0f);

        cgltf_options options = {};
        cgltf_data* data = NULL;
        cgltf_result result = cgltf_parse_file(&options, path, &data);
        
        if (result == cgltf_result_success) {
            cgltf_load_buffers(&options, data, path);
            
            // Fix: Traverse the scene nodes to get internal scales/rotations
            if (data->scene) {
                for (size_t i = 0; i < data->scene->nodes_count; ++i) {
                    processNode(data->scene->nodes[i], glm::mat4(1.0f));
                }
            }
            cgltf_free(data);
        } else {
            std::cout << "Failed to load GLTF: " << path << std::endl;
        }
    }

    void Draw(Shader& shader) {
        for (auto& p : primitives) {
            if (p.textureID != 0) {
                glActiveTexture(GL_TEXTURE0);
                shader.setInt("texture_diffuse1", 0);
                glBindTexture(GL_TEXTURE_2D, p.textureID);
            }

            glBindVertexArray(p.VAO);
            if (p.hasIndices) {
                glDrawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_INT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, p.indexCount);
            }
        }
        glBindVertexArray(0);
    }

private:
    void processNode(cgltf_node* node, glm::mat4 parentTransform) {
        glm::mat4 localTransform(1.0f);
        
        // Extract internal GLTF transformations
        if (node->has_matrix) {
            localTransform = glm::make_mat4(node->matrix);
        } else {
            if (node->has_translation) {
                localTransform = glm::translate(localTransform, glm::vec3(node->translation[0], node->translation[1], node->translation[2]));
            }
            if (node->has_rotation) {
                glm::quat q(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]);
                localTransform *= glm::mat4_cast(q);
            }
            if (node->has_scale) {
                localTransform = glm::scale(localTransform, glm::vec3(node->scale[0], node->scale[1], node->scale[2]));
            }
        }

        glm::mat4 globalTransform = parentTransform * localTransform;

        // Apply transformations to meshes attached to this node
        if (node->mesh) {
            for (size_t i = 0; i < node->mesh->primitives_count; ++i) {
                setupPrimitive(&node->mesh->primitives[i], globalTransform);
            }
        }

        // Recursively process children
        for (size_t i = 0; i < node->children_count; ++i) {
            processNode(node->children[i], globalTransform);
        }
    }

    void setupPrimitive(cgltf_primitive* prim, glm::mat4 transform) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        cgltf_accessor* posAcc = NULL;
        cgltf_accessor* normAcc = NULL;
        cgltf_accessor* uvAcc = NULL;

        for (cgltf_size i = 0; i < prim->attributes_count; ++i) {
            if (prim->attributes[i].type == cgltf_attribute_type_position) posAcc = prim->attributes[i].data;
            else if (prim->attributes[i].type == cgltf_attribute_type_normal) normAcc = prim->attributes[i].data;
            else if (prim->attributes[i].type == cgltf_attribute_type_texcoord) uvAcc = prim->attributes[i].data;
        }

        if (!posAcc) return; 

        // Create normal matrix to properly rotate normals without warping them
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

        size_t vertexCount = posAcc->count;
        for (size_t i = 0; i < vertexCount; ++i) {
            float pos[3] = {0}, norm[3] = {0}, uv[2] = {0};
            cgltf_accessor_read_float(posAcc, i, pos, 3);
            if (normAcc) cgltf_accessor_read_float(normAcc, i, norm, 3);
            if (uvAcc) cgltf_accessor_read_float(uvAcc, i, uv, 2);

            // Multiply the raw vertex positions by the GLTF internal transformations
            glm::vec4 worldPos = transform * glm::vec4(pos[0], pos[1], pos[2], 1.0f);
            glm::vec3 worldNorm = glm::normalize(normalMatrix * glm::vec3(norm[0], norm[1], norm[2]));

            // --- NEW: CALCULATE BOUNDING BOX (For Centering) ---
            minAABB.x = std::min(minAABB.x, worldPos.x);
            minAABB.y = std::min(minAABB.y, worldPos.y);
            minAABB.z = std::min(minAABB.z, worldPos.z);
            maxAABB.x = std::max(maxAABB.x, worldPos.x);
            maxAABB.y = std::max(maxAABB.y, worldPos.y);
            maxAABB.z = std::max(maxAABB.z, worldPos.z);

            vertices.push_back(worldPos.x); vertices.push_back(worldPos.y); vertices.push_back(worldPos.z);
            vertices.push_back(worldNorm.x); vertices.push_back(worldNorm.y); vertices.push_back(worldNorm.z);
            vertices.push_back(uv[0]); vertices.push_back(uv[1]);
        }

        if (prim->indices) {
            for (size_t i = 0; i < prim->indices->count; ++i) {
                indices.push_back(cgltf_accessor_read_index(prim->indices, i));
            }
        }

        unsigned int VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        if (prim->indices) {
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        MeshPrimitive mp;
        mp.VAO = VAO; mp.VBO = VBO;
        mp.EBO = prim->indices ? EBO : 0;
        mp.hasIndices = prim->indices != NULL;
        mp.indexCount = prim->indices ? prim->indices->count : vertexCount;
        
        mp.textureID = 0;
        if (prim->material && prim->material->has_pbr_metallic_roughness) {
            cgltf_texture_view& texView = prim->material->pbr_metallic_roughness.base_color_texture;
            if (texView.texture && texView.texture->image && texView.texture->image->uri) {
                mp.textureID = TextureFromFile(texView.texture->image->uri, directory);
            }
        }

        primitives.push_back(mp);
    }
};
#endif