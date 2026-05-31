#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.hpp>
#include <cgltf.h>

#include <string>
#include <iostream>
#include <vector>

// 1. Structural Definitions
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

// Helper function to load raw images into GPU Textures
unsigned int TextureFromFile(const char* path, const std::string& directory) {
    std::string filename = directory + '/' + std::string(path);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // Flips texture vertically if they appear upside down (common in glTF)
    // stbi_set_flip_vertically_on_load(true); 
    
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
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
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

// 2. The Mesh Class (Unchanged - safely handles GPU drawing)
class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;

    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> inds, std::vector<Texture> texs) {
        this->vertices = verts;
        this->indices = inds;
        this->textures = texs;
        setupMesh();
    }

    void Draw(unsigned int shaderProgram) {
        unsigned int diffuseNr = 1;

        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse") number = std::to_string(diffuseNr++);

            glUniform1i(glGetUniformLocation(shaderProgram, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    unsigned int VBO, EBO;
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glBindVertexArray(0);
    }
};

// 3. The Master Model Class (glTF specific via cgltf)
class Model {
public:
    std::vector<Mesh> meshes;
    std::string directory;

    Model(std::string const& path) {
        loadModel(path);
    }

    void Draw(unsigned int shaderProgram) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shaderProgram);
    }

private:
    void loadModel(std::string const& path) {
        this->directory = path.substr(0, path.find_last_of('/'));
        
        cgltf_options options = {};
        cgltf_data* data = NULL;
        cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
        
        if (result != cgltf_result_success) {
            std::cerr << "CGLTF Error: Failed to parse GLTF file " << path << std::endl;
            return;
        }

        // Load external .bin buffer files linked in the glTF
        result = cgltf_load_buffers(&options, data, path.c_str());
        if (result != cgltf_result_success) {
            std::cerr << "CGLTF Error: Failed to load .bin buffers for " << path << std::endl;
            cgltf_free(data);
            return;
        }

        // Process every mesh and sub-mesh (primitive) in the file
        for (size_t m = 0; m < data->meshes_count; ++m) {
            cgltf_mesh* mesh = &data->meshes[m];

            for (size_t p = 0; p < mesh->primitives_count; ++p) {
                cgltf_primitive* primitive = &mesh->primitives[p];
                
                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;
                std::vector<Texture> textures;

                // --- 1. Extract Vertices ---
                size_t vertexCount = 0;
                
                // Find total vertex count by looking at the Position attribute
                for (size_t i = 0; i < primitive->attributes_count; ++i) {
                    if (primitive->attributes[i].type == cgltf_attribute_type_position) {
                        vertexCount = primitive->attributes[i].data->count;
                        break;
                    }
                }
                vertices.resize(vertexCount);

                // Unpack binary data into our C++ Vertex structs
                for (size_t i = 0; i < primitive->attributes_count; ++i) {
                    cgltf_attribute* attribute = &primitive->attributes[i];
                    cgltf_accessor* accessor = attribute->data;

                    if (attribute->type == cgltf_attribute_type_position) {
                        for (size_t v = 0; v < vertexCount; ++v) {
                            cgltf_accessor_read_float(accessor, v, &vertices[v].Position.x, 3);
                        }
                    } else if (attribute->type == cgltf_attribute_type_normal) {
                        for (size_t v = 0; v < vertexCount; ++v) {
                            cgltf_accessor_read_float(accessor, v, &vertices[v].Normal.x, 3);
                        }
                    } else if (attribute->type == cgltf_attribute_type_texcoord) {
                        for (size_t v = 0; v < vertexCount; ++v) {
                            cgltf_accessor_read_float(accessor, v, &vertices[v].TexCoords.x, 2);
                        }
                    }
                }

             // --- 2. Extract Indices ---
                if (primitive->indices != nullptr) {
                    cgltf_accessor* accessor = primitive->indices;
                    indices.resize(accessor->count);
                    for (size_t i = 0; i < accessor->count; ++i) {
                        // FIXED: Using cgltf_accessor_read_index instead
                        indices[i] = (unsigned int)cgltf_accessor_read_index(accessor, i);
                    }
                } else {
                    // Generate manual indices if the mesh doesn't provide them
                    for(size_t i = 0; i < vertexCount; ++i) {
                        indices.push_back((unsigned int)i);
                    }
                }

                // --- 3. Extract Materials (Textures) ---
                if (primitive->material != nullptr && primitive->material->has_pbr_metallic_roughness) {
                    cgltf_texture_view* baseColorTex = &primitive->material->pbr_metallic_roughness.base_color_texture;
                    if (baseColorTex->texture != nullptr && baseColorTex->texture->image != nullptr) {
                        
                        // Extract relative image path directly from glTF
                        std::string imgUri = baseColorTex->texture->image->uri;
                        
                        Texture texture;
                        texture.id = TextureFromFile(imgUri.c_str(), this->directory);
                        texture.type = "texture_diffuse";
                        texture.path = imgUri;
                        textures.push_back(texture);
                    }
                }

                meshes.push_back(Mesh(vertices, indices, textures));
            }
        }
        
        // Free the memory buffer when complete
        cgltf_free(data);
    }
};