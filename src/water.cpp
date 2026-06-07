#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#define GLSL_VERSION 430

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Struct for Wave Properties
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
    Vector4 lightColor;
    float ambient;
    float specFactor;
    float specMult;

    Vector3 viewPos;
    Vector3 lightPos;
    // Shader Locations
    int locations[16];
};

typedef enum {
    numWavesLoc = 0, // 0

    startAngleLoc,
    startAmpLoc,
    startFreqLoc,
    startSpeedLoc,

    ampMultLoc,
    freqMultLoc,
    speedMultLoc,
    warpStrengthLoc,
    warpMultLoc,

    lightColorLoc,
    ambientLoc,
    specFactorLoc,
    specMultLoc,

    viewPosLoc,
    lightPosLoc,
} ShaderLocations;

//Camera Setup
Camera camera = {
    .position = (Vector3) {-40.0, 15.0, 0.0},
    .target = (Vector3) {0.0, 0.0, 0.0},
    .up = (Vector3) {0.0, 1.0, 0.0}, //X, Y, Z with Y up
    .fovy = 50.0,
    .projection = CAMERA_PERSPECTIVE   
};

//Positions
Vector3 planeCenter = {0.0, 0.0, 0.0};
Vector3 lightCenter = {270.0, 70.0, -15.0};
Vector3 origin = {0.0, 0.0, 0.0};

struct ShaderProperties wave = {
    // Vertex Shader
    .numWaves = 24,
    
    .startAngle = 0.67,
    .startAmp = 1.35,
    .startFreq = 0.3,
    .startSpeed = 4.5,

    .ampMult = 0.78,
    .freqMult = 1.2,
    .speedMult = 1.02,
    .warpStrength = 2.5,
    .warpMult = 0.90,

    // Fragment Shader
    .lightColor = (Vector4) {0.606, 0.6098, 0.7, 1.0}, // Pretty Close to White
    .ambient = 0.85,
    .specFactor = 128.0,
    .specMult = 2.5,
    
    .viewPos = camera.position,
    .lightPos = lightCenter,
    .locations = {0}
};

// Things to Add to This struct is the specular color vs the light color for the base lighting


const char* skyboxPath= "assets/Cubemap/Cubemap_Sky_05-512x512.png";

// Other Globals
float time = 0.0;

void getLocations(Shader waterShader, struct ShaderProperties *wave);
void updateWaveProperties(Shader waterShader, struct ShaderProperties *wave);

int main() {
    // Setup Window
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chill Water fr");
    SetTargetFPS(240);
    DisableCursor();

    // Shader Setup
    Shader waterShader = LoadShader("src/water.vs", "src/water.fs");
    Shader skyboxShader = LoadShader("src/sky.vs", "src/sky.fs");

    int timeLocation = GetShaderLocation(waterShader, "time");
    getLocations(waterShader, &wave);

    int cubemapType = MATERIAL_MAP_CUBEMAP;
    int environmentMapLocSky = GetShaderLocation(skyboxShader, "environmentMap");
    int environmentMapLocWater = GetShaderLocation(waterShader, "environmentMap");

    //Skybox Model and shader
    Mesh cube = GenMeshCube(1.0, 1.0, 1.0);
    Model skybox = LoadModelFromMesh(cube);
    skybox.materials[0].shader = skyboxShader;


    // Load Plane and assign shader
    Mesh planeMesh = GenMeshPlane(75, 75, 255, 255);
    Model planeModel = LoadModelFromMesh(planeMesh);
    planeModel.materials[0].shader = waterShader;


    //Load Skybox
    Image skyboxImage = LoadImage(skyboxPath);
    TextureCubemap skyboxCubemap = LoadTextureCubemap(skyboxImage, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(skyboxImage);


    // Assigns Cubemaps
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = skyboxCubemap;
    planeModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = skyboxCubemap;

    // Tells shaders that the skybox is a cubemap
    SetShaderValue(skyboxShader, environmentMapLocSky, &cubemapType, SHADER_UNIFORM_INT);
    SetShaderValue(waterShader, environmentMapLocWater, &cubemapType, SHADER_UNIFORM_INT);

    // Main Loop
    while (!WindowShouldClose()) {
        //Things To Update Per loop
        UpdateCamera(&camera, CAMERA_FREE);
        time = (float) GetTime();

        SetShaderValue(waterShader, timeLocation, &time, SHADER_UNIFORM_FLOAT);
        updateWaveProperties(waterShader, &wave);
        
        //Any Rendering Stuff
        BeginDrawing();
            ClearBackground(BLACK);
            
            BeginMode3D(camera);
                //Draws SkyBox
                BeginShaderMode(skyboxShader);
                    rlDisableBackfaceCulling();
                    rlDisableDepthMask();
                    DrawModel(skybox, origin, 50.0, WHITE);
                    rlEnableBackfaceCulling();
                    rlEnableDepthMask();
                EndShaderMode();

                // Draws Light and Water
                DrawSphere(lightCenter, 0.3, WHITE); // just to show location of light

                BeginShaderMode(waterShader);
                    rlDisableBackfaceCulling();
                    DrawModel(planeModel, planeCenter, 1.0, DARKBLUE);     
                    //DrawModelWires(planeModel, planeCenter, 1.0, RAYWHITE);
                    rlEnableBackfaceCulling();
                EndShaderMode();

            EndMode3D();
            DrawFPS(5, 5);

            // char cameraPosText[64] = "";
            // snprintf(cameraPosText, sizeof(cameraPosText), "%.1f, %.1f, %.1f", 
            // camera.position.x, camera.position.y,camera.position.z);
            // DrawText(cameraPosText, 1280/2, 720/2, 20, BLACK);
        EndDrawing();
    }

    //Unload stuf and close window
    UnloadModel(planeModel);
    UnloadModel(skybox);
    UnloadShader(waterShader);
    UnloadShader(skyboxShader);
    CloseWindow();
    return 0;
}

void getLocations(Shader waterShader, struct ShaderProperties *wave) { //probably shouldve used &wave but im C pilled
    wave->locations[numWavesLoc] = GetShaderLocation(waterShader, "numWaves");

    wave->locations[startAngleLoc] = GetShaderLocation(waterShader, "startAngle");
    wave->locations[startAmpLoc] = GetShaderLocation(waterShader, "startAmp");
    wave->locations[startFreqLoc] = GetShaderLocation(waterShader, "startFreq");
    wave->locations[startSpeedLoc] = GetShaderLocation(waterShader, "startSpeed");

    wave->locations[ampMultLoc] = GetShaderLocation(waterShader, "ampMult");
    wave->locations[freqMultLoc] = GetShaderLocation(waterShader, "freqMult");
    wave->locations[speedMultLoc] = GetShaderLocation(waterShader, "speedMult");
    wave->locations[warpStrengthLoc] = GetShaderLocation(waterShader, "warpStrength");
    wave->locations[warpMultLoc] = GetShaderLocation(waterShader, "warpMult");

    wave->locations[lightColorLoc] = GetShaderLocation(waterShader, "lightColor");
    wave->locations[ambientLoc] = GetShaderLocation(waterShader, "ambient");
    wave->locations[specFactorLoc] = GetShaderLocation(waterShader, "specFactor");
    wave->locations[specMultLoc] = GetShaderLocation(waterShader, "specMult");

    wave->locations[viewPosLoc] = GetShaderLocation(waterShader, "viewPos");
    wave->locations[lightPosLoc] = GetShaderLocation(waterShader, "lightPos");

    waterShader.locs[SHADER_LOC_VECTOR_VIEW] = wave->locations[viewPosLoc];
}

void updateWaveProperties(Shader waterShader, struct ShaderProperties *wave) {
    wave->viewPos = camera.position;
    wave->lightPos = lightCenter;

    SetShaderValue(waterShader, wave->locations[numWavesLoc], &wave->numWaves, SHADER_UNIFORM_INT);
    
    SetShaderValue(waterShader, wave->locations[startAngleLoc], &wave->startAngle, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[startAmpLoc], &wave->startAmp, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[startFreqLoc], &wave->startFreq, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[startSpeedLoc], &wave->startSpeed, SHADER_UNIFORM_FLOAT);

    SetShaderValue(waterShader, wave->locations[ampMultLoc], &wave->ampMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[freqMultLoc], &wave->freqMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[speedMultLoc], &wave->speedMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[warpStrengthLoc], &wave->warpStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[warpMultLoc], &wave->warpMult, SHADER_UNIFORM_FLOAT);

    SetShaderValue(waterShader, wave->locations[lightColorLoc], &wave->lightColor, SHADER_UNIFORM_VEC4);
    SetShaderValue(waterShader, wave->locations[ambientLoc], &wave->ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[specFactorLoc], &wave->specFactor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[specMultLoc], &wave->specMult, SHADER_UNIFORM_FLOAT);

    SetShaderValue(waterShader, wave->locations[viewPosLoc], &wave->viewPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(waterShader, wave->locations[lightPosLoc], &wave->lightPos, SHADER_UNIFORM_VEC3);
}

