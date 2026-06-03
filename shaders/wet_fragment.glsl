#version 330 core
// MULTIPLE RENDER TARGETS (MRT) FOR BLOOM
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D floorTex;
uniform samplerCube depthMap; // The Shadow Map
uniform vec3 cameraPos;
uniform float time;
uniform vec3 houseSize;
uniform float far_plane; // Maximum distance for shadows

// ==========================================
// PROCEDURAL NOISE GENERATOR (For Puddles)
// ==========================================
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// Fractal Brownian Motion
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
// SHADOW CALCULATION
// ==========================================
float ShadowCalculation(vec3 fragPos, vec3 lightPos) {
    vec3 fragToLight = fragPos - lightPos;
    
    // Get closest depth from light's perspective
    float closestDepth = texture(depthMap, fragToLight).r * far_plane;
    
    // Get current depth of the pixel
    float currentDepth = length(fragToLight);
    
    // Bias to prevent shadow acne (blocky artifacts)
    float bias = 0.15; 
    
    // If the pixel is further than the closest depth, it's in shadow
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

// ==========================================
// MAIN RENDERING LOOP
// ==========================================
void main() {
    // 1. GENERATE PUDDLE MASK
    float rawNoise = fbm(FragPos.xz * 0.2); 
    float puddleMask = smoothstep(0.45, 0.65, rawNoise); 

    // 2. BASE MATERIAL (Gritty & Dark)
    vec3 ambientNight = vec3(0.002, 0.005, 0.01); 
    vec3 albedo = texture(floorTex, TexCoords).rgb * 0.4; 
    
    // Darken wet concrete
    albedo = albedo * mix(1.0, 0.3, puddleMask);
    
    vec3 surfaceNormal = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - FragPos);
    
    // Walls are rougher and darker
    float surfaceBrightness = (abs(surfaceNormal.y) < 0.5) ? 0.15 : 1.0; 
    
    // --- LIGHT POSITIONS ---
    // --- LIGHT POSITIONS ---
    vec3 lamp1Pos = vec3(5.0, 0.0, (houseSize.z / 2.0) + 0.2); 
    vec3 lamp2Pos = vec3(-5.0, 0.0, (houseSize.z / 2.0) + 0.2);
    vec3 lamppostPos = vec3(4.5, 7.5, -(houseSize.z / 2.0) - 3.0); 
    vec3 smallNeonPos = vec3((houseSize.x / 2.0) + houseSize.x - 15.9, houseSize.y - 9.8, 0.0);
    
    // THE BIG NEON SIGN (Blue/Cyan)
    // TWEAK THIS: Adjust these coordinates to match where it is in your JS!
    vec3 bigNeonPos = vec3((houseSize.x / 2.0f) + houseSize.x - 16.9f, houseSize.y - 6.0f, 0.0f);

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
    
    // CUSTOM LIGHT RANGES: vec2(Linear, Quadratic)
    vec2 falloff[5] = vec2[](
        // Lamps 1 & 2: Drastically increased falloff. 
        // This is calibrated to naturally die out at around 13-15 units.
        vec2(0.25, 0.35), 
        vec2(0.35, 0.44), 
        
        // Lamppost: Slightly wider reach than the wall lamps
        vec2(0.22, 0.20), 
        
        // Small Pink Neon (Dies very fast)
        vec2(0.3, 0.15),     
        
        // Big Blue Neon
        vec2(0.18, 0.07)    
    );


    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    // 3. CALCULATE LIGHTING & SHADOWS
    for(int i = 0; i < 5; i++) {
        vec3 lightDir = normalize(lights[i] - FragPos);
        float distance = length(lights[i] - FragPos);
         

        //Fixing the Yellow Tint & Bleeding Faux Shadows
        // Adjust the 10.0 value based on how big your scene is.
        float maxRadius = 10.0;
        if(distance > maxRadius) {
            continue; // Completely kill the light calculation here
        }
        
        float attenuation = 1.0 / (1.0 + falloff[i].x * distance + falloff[i].y * (distance * distance));

        float edgeFade = smoothstep(maxRadius, maxRadius - 2.0, distance);
        attenuation *= edgeFade;

        // Force clamp the attenuation so it never goes negative or bleeds yellow
        attenuation = clamp(attenuation, 0.0, 1.0);
        
        // Compute shadow (Currently bound to Big Neon Light at index 4)
        // If you add more depth maps later, you can pass an array of them!
        float shadow = (i == 4) ? ShadowCalculation(FragPos, lights[i]) : 0.0;
        
        // DIFFUSE
        float diff = max(dot(surfaceNormal, lightDir), 0.0);
        // Multiply by (1.0 - shadow) so shadows physically block diffuse light
        vec3 diffuse = diff * lightColors[i] * albedo * attenuation * surfaceBrightness * (1.0 - shadow);
        
        // DYNAMIC PUDDLE SPECULARITY
        float specMultiplier = (abs(surfaceNormal.y) > 0.5) ? mix(0.05, 2.5, puddleMask) : 0.02;
        float shininess = (abs(surfaceNormal.y) > 0.5) ? mix(16.0, 256.0, puddleMask) : 16.0;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(surfaceNormal, halfwayDir), 0.0), shininess); 
        
        // Multiply by (1.0 - shadow) so puddles don't glow in the dark!
        vec3 specular = spec * lightColors[i] * attenuation * specMultiplier * (1.0 - shadow); 
        
        totalDiffuse += diffuse;
        totalSpecular += specular;
    }

    // 4. FINAL COMPOSITION
    vec3 finalColor = (albedo * ambientNight) + totalDiffuse + totalSpecular;
    
    // --------------------------------------------------------
    // DO NOT ADD TONE MAPPING OR GAMMA CORRECTION HERE!
    // Bloom requires raw linear HDR math. Tone mapping and Gamma 
    // are now handled at the very end in `bloom_final.glsl`.
    // --------------------------------------------------------

    // OUTPUT 1: The normal scene 
    FragColor = vec4(finalColor, 1.0);
    
    // OUTPUT 2: Extract bright areas for Bloom blurring
    float brightness = dot(finalColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        BrightColor = vec4(finalColor, 1.0);
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}