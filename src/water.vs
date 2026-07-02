#version 430

//In and Out Data
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
in vec3 vertexNormal; 

//Data sent to fragment shader
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragPosition;
out vec2 startUV; // For pixel perfect normals

// Data thats from cpu
uniform mat4 mvp; // Model->World, World->View, View->Projection
uniform mat4 matModel; //Model->World Used For Lighting
uniform mat4 matNormal; //Local Normal to World Normal

uniform float time;

uniform int numWaves;

uniform float startAngle;
uniform float startAmp;
uniform float startFreq;
uniform float startSpeed;
uniform float angleStep;

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


// Wave Properties
struct ShaderProperties {
    // Vertex Shader
    int numWaves;

    float startAngle;
    float startAmp;
    float startFreq;
    float startSpeed;
    float angleStep;

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
    angleStep,

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


//Declartions
float innerWave(float X, float freq, float speed, float time);


void main() {
    vec3 finalPos = vertexPosition;
    vec2 UV = vertexPosition.xz;
    startUV = UV;

    float currentAngle = wave.startAngle;
    float X = 0.0; // Base Input

    float sinAngle = 0.0;
    float cosAngle = 0.0; //I WILL DO THIS LATER SINCE I'M LAZY FOR OPTIMIZATION TO SAVE CALUCLATING COS AND SIN MULTIPLE TIMES

    //Calculation Of Wave 
    float waveSum = 0.0;
    float currentWave = 0.0;

    float innerPart = 0.0; //freq(X + time*speed)
    float sinePart = 0.0; //a * sin(freq(X + time*speed))
    float sharedDevPart = 0.0; //e^((a*sin(b((cos(theta)*x+sin(theta)*y)+t))-1) * a*cos(b((cos(theta)*x+sin(theta)*y)+t)) * b

    // Partial Derivatives for Wave
    float ddx = 0.0; 
    float ddy = 0.0; 

    //Full Function is e^((a*sin(b((cos(theta)*x+sin(theta)*y)+t))-1)
    // Original function from GPU gems is the Gernster Wave but like idk its complicated
    //Calculation is split to prevent recalculating when doing the deritvatives
    for (int i = 1; i <= numWaves; i++) {
        sinAngle = sin(currentAngle);
        cosAngle = cos(currentAngle);
    
        X = UV.x * cosAngle + UV.y * sinAngle;
        
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
        currentAngle += wave.angleStep;

        waveSum += currentWave;
   }

    // NOT USED, WILL DELETE LATER SINCE MOVED TO FRAGMENT SHADER
    //Calculates the normal (basically cross product) then normalizes it
    vec3 calcNormal = normalize(vec3(-ddx, 1.0, -ddy));
    calcNormal = vec3(matNormal * vec4(calcNormal, 0.0)); // Turns Normal into World Space

    
    finalPos.y += waveSum;
    finalPos.y -= numWaves * 0.35;

    // Finalize data stuff 
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragPosition = vec3(matModel * vec4(finalPos, 1.0));

    gl_Position = mvp * vec4(finalPos, 1.0);
}


float innerWave(float X, float freq, float speed, float time) {
    // This calculates the freq(X + time*speed) Part
    float sineResult = freq * (X + (time * speed));
    return sineResult;
}
