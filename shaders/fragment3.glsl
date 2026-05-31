#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D floorTex;
uniform vec3 cameraPos;
uniform float time;
uniform vec3 houseSize;

// ==========================================
// PROCEDURAL NOISE GENERATOR (For Puddles)
// ==========================================
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f); // Smooth interpolation
    return mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// Fractal Brownian Motion (Generates organic, cloudy shapes)
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}
// ==========================================

void main() {
    // 1. GENERATE PUDDLE MASK
    // We use FragPos.xz so the puddles are locked to the world floor.
    // Tweak the '0.2' to make the puddles larger or smaller!
    float rawNoise = fbm(FragPos.xz * 0.2); 
    
    // Smoothstep creates a hard cutoff, isolating the noise into distinct "pools" of water
    float puddleMask = smoothstep(0.45, 0.65, rawNoise); 

    // 2. BASE MATERIAL
    vec3 ambientNight = vec3(0.002, 0.005, 0.01); 
    vec3 albedo = texture(floorTex, TexCoords).rgb * 0.4; 
    
    // Darken the concrete where it is wet (Water absorbs diffuse light)
    albedo = albedo * mix(1.0, 0.3, puddleMask);
    
    vec3 surfaceNormal = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - FragPos);
    
    float surfaceBrightness = (abs(surfaceNormal.y) < 0.5) ? 0.15 : 1.0; 
    
    // --- LIGHT POSITIONS ---
    vec3 lamp1Pos = vec3(5.0, 0.0, (houseSize.z / 2.0) + 0.2); 
    vec3 lamp2Pos = vec3(-5.0, 0.0, (houseSize.z / 2.0) + 0.2);
    vec3 lamppostPos = vec3(4.5, 7.5, -(houseSize.z / 2.0) - 3.0); 
    vec3 smallNeonPos = vec3((houseSize.x / 2.0) + houseSize.x - 15.9, houseSize.y - 9.8, 0.0);
    vec3 bigNeonPos = vec3(-(houseSize.x / 2.0) - 5.0, houseSize.y - 6.0, 5.0);

    float neonFlicker = 0.7 + 0.3 * sin(time * 50.0) * cos(time * 23.0);
    if (fract(sin(floor(time * 8.0)) * 43758.5453) > 0.85) neonFlicker *= 0.2;

    vec3 lights[5] = vec3[](lamp1Pos, lamp2Pos, lamppostPos, smallNeonPos, bigNeonPos);
    
    vec3 lightColors[5] = vec3[](
        vec3(1.2, 0.8, 0.5) * 0.6,       
        vec3(1.2, 0.8, 0.5) * 0.6,       
        vec3(1.3, 1.3, 1.1) * 0.8,       
        vec3(3.0, 0.2, 1.0) * neonFlicker * 0.6, 
        vec3(0.2, 0.8, 3.0) * 1.5        
    );

    vec2 falloff[5] = vec2[](
        vec2(0.045, 0.0075), 
        vec2(0.045, 0.0075), 
        vec2(0.045, 0.0075), 
        vec2(0.3, 0.15),     
        vec2(0.18, 0.07)    
    );

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    // 3. CALCULATE LIGHTING
    for(int i = 0; i < 5; i++) {
        vec3 lightDir = normalize(lights[i] - FragPos);
        float distance = length(lights[i] - FragPos);
        
        float attenuation = 1.0 / (1.0 + falloff[i].x * distance + falloff[i].y * (distance * distance));
        
        // DIFFUSE
        float diff = max(dot(surfaceNormal, lightDir), 0.0);
        vec3 diffuse = diff * lightColors[i] * albedo * attenuation * surfaceBrightness;
        
        // --- DYNAMIC PUDDLE SPECULARITY ---
        // If it's a wall, no puddles. 
        // If it's the floor, fade smoothly between dry (0.05 brightness) and wet puddle (2.5 brightness)
        float specMultiplier = (abs(surfaceNormal.y) > 0.5) ? mix(0.05, 2.5, puddleMask) : 0.02;
        
        // Dry concrete has a wide, rough highlight (shininess 16.0). 
        // Puddles have a tight, mirror-like highlight (shininess 256.0).
        float shininess = (abs(surfaceNormal.y) > 0.5) ? mix(16.0, 256.0, puddleMask) : 16.0;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(surfaceNormal, halfwayDir), 0.0), shininess); 
        
        vec3 specular = spec * lightColors[i] * attenuation * specMultiplier; 
        
        totalDiffuse += diffuse;
        totalSpecular += specular;
    }

    // 4. FINAL COMPOSITION
    vec3 finalColor = (albedo * ambientNight) + totalDiffuse + totalSpecular;
    
    // Tone Mapping & Gamma
    finalColor = finalColor / (finalColor + vec3(1.0)); 
    finalColor = pow(finalColor, vec3(1.0 / 2.2)); 

    FragColor = vec4(finalColor, 1.0);
}