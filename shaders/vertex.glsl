#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculate the vertex position in world coordinates
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normals correctly even if the object scales
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    
    // Forward texture coordinates to the Fragment Shader
    TexCoords = aTexCoords;
    
    // Final position on your screen
    gl_Position = projection * view * vec4(FragPos, 1.0);
}