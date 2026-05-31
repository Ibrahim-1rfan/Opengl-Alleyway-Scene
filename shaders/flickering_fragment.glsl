#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor; // ADD THIS FOR BLOOM

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1; // Maps automatically from your GLTF loader
uniform float time;

// Simple pseudo-random function based on time
float random(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    // Sample the base texture of the sign
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 1. Continuous organic buzzing/flicker using multi-frequency sine waves
    float buzz = 0.7 + 0.3 * sin(time * 5.0) * cos(time * 25.0) * sin(time * 12.0);
    
    // 2. Sudden, erratic sharp drops (the "faulty neon wire" effect)
    float suddenDrop = 1.0;
    // We check a step based on a discretized time block so the drop holds for a fraction of a second
    if (random(floor(time * 8.0)) > 0.90) {
        suddenDrop = random(time) * 0.2; // Drop brightness significantly down to 0% - 20%
    }
    
    // Combine both effects
    float totalFlicker = buzz * suddenDrop;
    
    // Multiply the RGB output to push it past 1.0 
    // This makes it bright now, and will act as an intense emissive source for your Bloom later!
    vec3 brightColor = texColor.rgb * totalFlicker * 2.5;
    
    FragColor = vec4(brightColor, texColor.a);

    // ADD THIS TO THE BOTTOM: Send the glowing sign to the Bloom Blur pass
    float brightness = dot(brightColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 2.0) BrightColor = vec4(brightColor, texColor.a);
    else BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}