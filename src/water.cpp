#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#define GLSL_VERSION 430

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define X_TILES 5
#define Y_TILES 5
#define TILE_SIZE 50

// Struct for Wave Properties
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
    Vector4 lightColor;
    float ambient;
    float specFactor;
    float specMult;
    float scatterFactorAdj;
    float foamFactorAdj;

    Vector3 viewPos;
    Vector3 lightPos;
    // Shader Locations
    int locations[19];
};


typedef enum {
    numWavesLoc = 0, // 0

    startAngleLoc,
    startAmpLoc,
    startFreqLoc,
    startSpeedLoc,
    angleStepLoc,

    ampMultLoc,
    freqMultLoc,
    speedMultLoc,
    warpStrengthLoc,
    warpMultLoc,

    lightColorLoc,
    ambientLoc,
    specFactorLoc,
    specMultLoc,
    scatterFactorAdjLoc,
    foamFactorAdjLoc,

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
Vector3 lightCenter = {540.0, 140.0, -30.0};
Vector3 origin = {0.0, 0.0, 0.0};

struct ShaderProperties wave = {
    // Vertex Shader
    .numWaves = 24,
    
    .startAngle = 0.67,
    .startAmp = 1.20,
    .startFreq = 0.32,
    .startSpeed = 3.5,
    .angleStep = 0.618033988749895, //Golden ratio thingy

    .ampMult = 0.795,
    .freqMult = 1.2,
    .speedMult = 1.02,
    .warpStrength = 2.1,
    .warpMult = 0.90,

    // Fragment Shader
    .lightColor = ((Vector4) {226, 175, 187, 1.0})/255.0, // Pretty Close to White
    .ambient = 0.85,
    .specFactor = 128.0,
    .specMult = 2.5,
    .scatterFactorAdj = 1.0,
    .foamFactorAdj = 1.0,

    .viewPos = camera.position,
    .lightPos = lightCenter,
    .locations = {0}
};



// UI slop
bool showUI = false;
float adjustableNumWaves = 24.0;

// Things to Add to This struct is the specular color vs the light color for the base lighting


const char* skyboxPath= "assets/Cubemap/Cubemap_Sky_05-512x512.png";

// Other Globals
float time = 0.0;

void getLocations(Shader waterShader, struct ShaderProperties *wave);
void updateWaveProperties(Shader waterShader, struct ShaderProperties *wave);
void drawUI();

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
    Mesh planeMesh = GenMeshPlane(50, 50, 255, 255);
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
    UpdateCamera(&camera, CAMERA_FREE);
    while (!WindowShouldClose()) {
        //Things To Update Per loop
        if (IsKeyPressed(KEY_M)) {
            showUI = !showUI;
            if (showUI) 
                EnableCursor();
            else 
                DisableCursor();      
        }


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
                    DrawModel(planeModel, planeCenter, 1.0, Color(45, 214, 173, 255));
                    DrawModel(planeModel, (Vector3) {50.0, 0.0, 0.0}, 1.0, Color(45, 214, 173, 255));
                    DrawModel(planeModel, (Vector3) {100.0, 0.0, 0.0}, 1.0, Color(45, 214, 173, 255));
                    DrawModel(planeModel, (Vector3) {50.0, 0.0, 50.0}, 1.0, Color(45, 214, 173, 255));
                    DrawModel(planeModel, (Vector3) {50.0, 0.0, -50.0}, 1.0, Color(45, 214, 173, 255));
                    DrawModel(planeModel, (Vector3) {0.0, 0.0, 50.0}, 1.0, Color(45, 214, 173, 255));   
                    DrawModel(planeModel, (Vector3) {0.0, 0.0, -50.0}, 1.0, Color(45, 214, 173, 255));  
                    //DrawModelWires(planeModel, planeCenter, 1.0, RAYWHITE);
                    rlEnableBackfaceCulling();
                EndShaderMode();

            EndMode3D();
        DrawFPS(5, 5);

        char cameraPosText[64] = "";
        snprintf(cameraPosText, sizeof(cameraPosText), "%.1f, %.1f, %.1f", 
        camera.position.x, camera.position.y,camera.position.z);
        DrawText(cameraPosText, 5, 25, 20, BLACK);
        DrawText("M For Menu | Esc to Exit", 5, 45, 20, BLACK);

        if (!showUI)       
            UpdateCamera(&camera, CAMERA_FREE);
        else 
            drawUI();

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

void drawUI() {
    GuiSliderBar((Rectangle) {90.0, 70.0, 150.0, 25.0}, "# Waves:", NULL, &adjustableNumWaves, 0.0, 64.0);
    wave.numWaves = (int) adjustableNumWaves;

    GuiSliderBar((Rectangle) {90.0, 100.0, 150.0, 25.0}, "Angle: ", NULL, &wave.startAngle, 0.0, 6.28);
    GuiSliderBar((Rectangle) {90.0, 130.0, 150.0, 25.0}, "Amplitude", NULL, &wave.startAmp, 0.0, 4.0);
    GuiSliderBar((Rectangle) {90.0, 160.0, 150.0, 25.0}, "Frequency", NULL, &wave.startFreq, 0.0, 1.0);
    GuiSliderBar((Rectangle) {90.0, 190.0, 150.0, 25.0}, "Speed", NULL, &wave.startSpeed, 0.0, 15.0);
    GuiSliderBar((Rectangle) {90.0, 220.0, 150.0, 25.0}, "Angle Step", NULL, &wave.angleStep, 0.0, 6.28);

    GuiSliderBar((Rectangle) {90.0, 250.0, 150.0, 25.0}, "Amplitude Mult.", NULL, &wave.ampMult, 0.0, 1.0);
    GuiSliderBar((Rectangle) {90.0, 280.0, 150.0, 25.0}, "Freq Multiplier", NULL, &wave.freqMult, 0.0, 5.0);
    GuiSliderBar((Rectangle) {90.0, 310.0, 150.0, 25.0}, "Speed Multiplier", NULL, &wave.speedMult, 0.0, 5.0);
    GuiSliderBar((Rectangle) {90.0, 340.0, 150.0, 25.0}, "Sharpness", NULL, &wave.warpStrength, 0.0, 15.0);
    GuiSliderBar((Rectangle) {90.0, 370.0, 150.0, 25.0}, "Warp Mult.", NULL, &wave.warpMult, 0.0, 1.1);

    GuiSliderBar((Rectangle) {90.0, 400.0, 150.0, 25.0}, "Ambient Light", NULL, &wave.ambient, 0.0, 5.0);
    GuiSliderBar((Rectangle) {90.0, 430.0, 150.0, 25.0}, "Specular Mult.", NULL, &wave.specMult, 0.0, 10.0);

    GuiSliderBar((Rectangle) {90.0, 460.0, 150.0, 25.0}, "Scatter Factor", NULL, &wave.scatterFactorAdj, 0.0, 5.0);
    GuiSliderBar((Rectangle) {90.0, 490.0, 150.0, 25.0}, "Foam Factor", NULL, &wave.foamFactorAdj, 0.0, 15.0);

    
}

void getLocations(Shader waterShader, struct ShaderProperties *wave) { //probably shouldve used &wave but im C pilled
    wave->locations[numWavesLoc] = GetShaderLocation(waterShader, "numWaves");

    wave->locations[startAngleLoc] = GetShaderLocation(waterShader, "startAngle");
    wave->locations[startAmpLoc] = GetShaderLocation(waterShader, "startAmp");
    wave->locations[startFreqLoc] = GetShaderLocation(waterShader, "startFreq");
    wave->locations[startSpeedLoc] = GetShaderLocation(waterShader, "startSpeed");
    wave->locations[angleStepLoc] = GetShaderLocation(waterShader, "angleStep");

    wave->locations[ampMultLoc] = GetShaderLocation(waterShader, "ampMult");
    wave->locations[freqMultLoc] = GetShaderLocation(waterShader, "freqMult");
    wave->locations[speedMultLoc] = GetShaderLocation(waterShader, "speedMult");
    wave->locations[warpStrengthLoc] = GetShaderLocation(waterShader, "warpStrength");
    wave->locations[warpMultLoc] = GetShaderLocation(waterShader, "warpMult");

    wave->locations[lightColorLoc] = GetShaderLocation(waterShader, "lightColor");
    wave->locations[ambientLoc] = GetShaderLocation(waterShader, "ambient");
    wave->locations[specFactorLoc] = GetShaderLocation(waterShader, "specFactor");
    wave->locations[specMultLoc] = GetShaderLocation(waterShader, "specMult");
    wave->locations[scatterFactorAdjLoc] = GetShaderLocation(waterShader, "scatterFactorAdj");
    wave->locations[foamFactorAdjLoc] = GetShaderLocation(waterShader, "foamFactorAdj");

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
    SetShaderValue(waterShader, wave->locations[angleStepLoc], &wave->angleStep, SHADER_UNIFORM_FLOAT);

    SetShaderValue(waterShader, wave->locations[ampMultLoc], &wave->ampMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[freqMultLoc], &wave->freqMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[speedMultLoc], &wave->speedMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[warpStrengthLoc], &wave->warpStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[warpMultLoc], &wave->warpMult, SHADER_UNIFORM_FLOAT);

    SetShaderValue(waterShader, wave->locations[lightColorLoc], &wave->lightColor, SHADER_UNIFORM_VEC4);
    SetShaderValue(waterShader, wave->locations[ambientLoc], &wave->ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[specFactorLoc], &wave->specFactor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[specMultLoc], &wave->specMult, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[scatterFactorAdjLoc], &wave->scatterFactorAdj, SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader, wave->locations[foamFactorAdjLoc], &wave->foamFactorAdj, SHADER_UNIFORM_FLOAT);


    SetShaderValue(waterShader, wave->locations[viewPosLoc], &wave->viewPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(waterShader, wave->locations[lightPosLoc], &wave->lightPos, SHADER_UNIFORM_VEC3);
}

