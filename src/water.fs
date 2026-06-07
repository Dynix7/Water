#version 430

// Inputs and Outputs
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec2 startUV;

out vec4 finalColor;

uniform float time;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform mat4 matNormal; // For per pixel normal caluclation

uniform int numWaves;

uniform float startAngle;
uniform float startAmp;
uniform float startFreq;
uniform float startSpeed;

uniform float ampMult;
uniform float freqMult;
uniform float speedMult;
uniform float warpStrength;
uniform float warpMult;

//Fragment Shader
uniform vec4 lightColor;
uniform float ambient;
uniform float specFactor;
uniform float specMult;

uniform vec3 viewPos;
uniform vec3 lightPos;

 // Skybox
uniform samplerCube environmentMap;

#define GOLDEN_STEP 0.618033988749895

// Wave Properties
struct ShaderProperties {
    // Vertex Shader
    int numWaves;

    float startAngle;
    float startAmp;
    float startFreq;
    float startSpeed;

    float ampMult;
    float freqMult;
    float speedMult;
    float warpStrength;
    float warpMult;

    //Fragment Shader
    vec4 lightColor;
    float ambient;
    float specFactor;
    float specMult;

    vec3 viewPos;
    vec3 lightPos;
};

ShaderProperties wave = ShaderProperties(
    numWaves,

    startAngle,
    startAmp,
    startFreq,
    startSpeed,

    ampMult,
    freqMult,
    speedMult,
    warpStrength,
    warpMult,

    lightColor,
    ambient,
    specFactor,
    specMult,

    viewPos,
    lightPos
);

float innerWave(float X, float freq, float speed, float time);


void main() {
    // Calculates Normal Per Pixel
    vec2 UV = startUV;

    float currentAngle = wave.startAngle;
    float X = 0.0; // Base Input

    float sinAngle = 0.0;
    float cosAngle = 0.0;
    //Calculation Of Wave 
    float currentWave = 0.0;

    float innerPart = 0.0; //freq(X + time*speed)
    float sinePart = 0.0; //a * sin(freq(X + time*speed))
    float sharedDevPart = 0.0; //e^((a*sin(b((cos(theta)*x+sin(theta)*y)+t))-1) * a*cos(b((cos(theta)*x+sin(theta)*y)+t)) * b

    // Partial Derivatives for Wave
    float ddx = 0.0; 
    float ddy = 0.0; 

    for (int i = 1; i <= numWaves; i++) {
        sinAngle = sin(currentAngle);
        cosAngle = cos(currentAngle);
    
        X = UV.x * cosAngle + UV.y * sinAngle; // Calculates the Rotation

        innerPart = innerWave(X, wave.startFreq, wave.startSpeed, time);
        sinePart = wave.startAmp * sin(innerPart);
        currentWave = exp(sinePart - 1);  //Full Wave Function

        //Calculating the partial derivatives
        // for X: e^((a*sin(b((cos(theta)*x+sin(theta)*y)+t))-1) * a*cos(b((cos(theta)*x+sin(theta)*y)+t)) * b * cos(theta)
        // for Y: e^((a*sin(b((cos(theta)*x+sin(theta)*y)+t))-1) * a*cos(b((cos(theta)*x+sin(theta)*y)+t)) * b * sin(theta)
        sharedDevPart = currentWave * (wave.startAmp * cos(innerPart)) * wave.startFreq;

  
        ddx += sharedDevPart * cosAngle;
        ddy += sharedDevPart * sinAngle; 

        // Domain Warping thingy where it looks like the waves are pushing eachother
        UV.x -= sharedDevPart * cosAngle * wave.warpStrength;
        UV.y -= sharedDevPart * sinAngle * wave.warpStrength;
        
        wave.warpStrength *= wave.warpMult;
        // Adjusts Angle and Makes Waves Smaller
        wave.startFreq *= wave.freqMult;
        wave.startAmp *= wave.ampMult;
        wave.startSpeed *= wave.speedMult;
        currentAngle += GOLDEN_STEP;
   }

    vec3 calcNormal = normalize(vec3(-ddx, 1.0, -ddy));
    calcNormal = vec3(matNormal * vec4(calcNormal, 0.0));

    vec3 normal = normalize(calcNormal);
    vec3 viewDir = normalize(wave.viewPos - fragPosition);
    vec3 lightDir = normalize(wave.lightPos - fragPosition);

    // Diffuse Factor Calculation
    float NdotL = dot(normal, lightDir); // 0 to 1
    float diffuseFactor = max(NdotL, 0.0);
    //diffuseFactor = pow(diffuseFactor, 1.3);

    // Environment Reflections
    // Reflect Vector = viewDir - 2(dot(normal, viewDir))  * normal

    vec3 I = -viewDir; // Flips the viewDir so points correctly
    vec3 reflectDir = reflect(I, normal);
    vec3 reflectColor = texture(environmentMap, reflectDir).rgb;


    // Fresnel Calculation wikipedia.org/wiki/Schlick's_approximation
    float R0 = 0.020332; // From Refractive Indices of Air and Water. 1.0 vs 1.333
    float normalDotView = max(dot(normal, viewDir), 0.0);
    float fresnel = R0 + (1.0 - R0) * pow((1.0 - normalDotView), 5); 
    // pow((1.0 - normalDotView), 5) can also be used by itself since like the other terms r basically just 1 
    vec3 fresnelColor = vec3(fresnel) * reflectColor; // Not used btw i'll probably remove if i remember to lol

    //Specular Reflection Calculation
    float specularIntensity  = 0.0;
    vec3 specularColor = {0.0, 0.0, 0.0};
    if (NdotL > 0.0) {
        vec3 halfVector = normalize(viewDir + lightDir);
        specularIntensity = max(dot(normal, halfVector), 0.0);
        specularIntensity = pow(specularIntensity, wave.specFactor) * specMult * fresnel;

        specularColor = specularIntensity * wave.lightColor.rgb;
    }

    // Combining Everything
    vec3 baseColor = colDiffuse.rgb * fragColor.rgb;

    vec3 diffuse = diffuseFactor * baseColor;
    vec3 ambientColor = baseColor * ambient;
    ambientColor *= wave.lightColor.rgb;

    vec3 waveBaseColor = diffuse + ambientColor;


    vec3 finalRGB = mix(waveBaseColor, reflectColor, fresnel);
    finalRGB += specularColor;

    vec3 result = clamp(finalRGB, 0.0, 1.0);
    vec4 scaledResult = vec4(result, 1.0);

    //finalColor = vec4(normal * 0.5 + 0.5, 1.0);// For testing normals
    //finalColor = colDiffuse;
    finalColor = scaledResult;
}


float innerWave(float X, float freq, float speed, float time) {
    // This calculates the freq(X + time*speed) Part
    float sineResult = freq * (X + (time * speed));
    return sineResult;
}
