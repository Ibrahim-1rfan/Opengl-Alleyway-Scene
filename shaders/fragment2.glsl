#version 430 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


// Matches the naming output inside Model.h
uniform sampler2D texture_diffuse1; 

void main() {
    // Sample color from your model's texture file
    vec3 objectColor = texture(texture_diffuse1, TexCoords).rgb;

    // 1. Ambient Lighting (Low background light so shadows aren't pitch black)
    vec3 ambient = 0.25 * objectColor;

    // 2. Simple Overhead Directional Light (Simulating a dim moon/sky light)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.2, 1.0, 0.2)); // Points down into the alley
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * objectColor * 0.7;

    // Combine lighting calculations
    vec3 finalResult = ambient + diffuse;

    FragColor = vec4(finalResult, 1.0);
}