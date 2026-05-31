#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

void main() {             
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // Add the glowing bloom to the scene
    hdrColor += bloomColor; 
    
    // HDR Tone Mapping (Reinhard)
    vec3 result = hdrColor / (hdrColor + vec3(1.0));
    
    // Gamma Correction
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}