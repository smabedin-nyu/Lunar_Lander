/**
 * Author: Shahzaib
 * Assignment: Lunar Lander!
 * Date due: 10/26/2026
 *
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 */

#include "CS3113/Entity.h"

// Screen Constants
constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int FPS = 60;
constexpr int PLATFORM_COUNT = 3;

// Delta Time, Physics, Tie Fighter Fuel Constants
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr float GRAVITY     = 70.0f;
constexpr float THRUST      = 100.0f;
constexpr float SIDE_THRUST = 70.0f;
constexpr float FUEL_MAX = 100.0f;
constexpr float FUEL_RATE = 13.0f;
constexpr float LAVA_TOP = 653.0f;
constexpr float PLATFORM_SPEED = 85.0f;

// GameStates
enum GameState { PLAYING, MISSION_ACCOMPLISHED, MISSION_FAILED };

// Delta Time, Physics, Tie Fighter Fuel
AppStatus gAppStatus = RUNNING;
GameState gGameState = PLAYING;
float gPreviousTicks = 0.0f;
float gTimeAccumulator = 0.0f;
float gFuel = FUEL_MAX;
bool gThrusting = false;
int gPlatformDirection = 1;

// Entities
Entity *gPlayer = nullptr;
Entity *gFlame = nullptr;
Entity *gPlatforms = nullptr;
Texture2D gBackground = {};

// Forward Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void restart();

// restart: A helper I made to restart the game after the game ends
void restart()
{
    gPlayer->setPosition({230.0f, 110.0f});
    gPlayer->resetVelocity();
    gPlayer->setAcceleration({0.0f, GRAVITY});
    gFuel = FUEL_MAX;
    gGameState = PLAYING;
    gThrusting = false;
    gPlatformDirection = 1;
    gPlatforms[2].setPosition({640.0f, 335.0f});
    gPreviousTicks = static_cast<float>(GetTime());
    gTimeAccumulator = 0.0f;
}

// initialise
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Landing");
    SetTargetFPS(FPS);
    gBackground = LoadTexture("assets/background.png"); // Created this background myself on Illustrator

    gPlayer = new Entity({230.0f, 110.0f}, {65.0f, 65.0f}, "assets/tiefighter.png"); // Created this myself on Illustrator
    gPlayer->setColliderDimensions({49.0f, 49.0f});

    std::map<Direction, std::vector<int>> flameAtlas = {
        { RIGHT, {0, 1, 2} }
    };
    gFlame = new Entity({230.0f, 160.0f}, {48.0f, 39.0f},
        "assets/rocket_fire_animation.png", ATLAS, {1.0f, 3.0f}, flameAtlas);  // Modified this from an image online: https://www.istockphoto.com/vector/fire-emoji-icon-gm1328395235-412466405
    gFlame->setFrameSpeed(10);

    gPlatforms = new Entity[PLATFORM_COUNT];
    gPlatforms[0].setTexture("assets/static_platform.png"); // Created this myself on Illustrator
    gPlatforms[0].setScale({160.0f, 80.0f});
    gPlatforms[0].setColliderDimensions({145.0f, 58.0f});
    gPlatforms[0].setPosition({115.0f, 595.0f});

    gPlatforms[1].setTexture("assets/static_platform.png"); // Repeated Asset
    gPlatforms[1].setScale({160.0f, 80.0f});
    gPlatforms[1].setColliderDimensions({145.0f, 58.0f});
    gPlatforms[1].setPosition({1165.0f, 595.0f});

    gPlatforms[2].setTexture("assets/moving_platform.png"); // Created this myself on Illustrator
    gPlatforms[2].setScale({235.0f, 75.0f});
    gPlatforms[2].setColliderDimensions({218.0f, 54.0f});
    gPlatforms[2].setPosition({640.0f, 335.0f});

    restart();
}

// processInput
void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
    if (gGameState != PLAYING)
    {
        if (IsKeyPressed(KEY_R)) restart();
        return;
    }

    Vector2 acceleration = {0.0f, GRAVITY};
    gThrusting = false;
    if (gFuel > 0.0f)
    {
        if (IsKeyDown(KEY_A)) acceleration.x -= SIDE_THRUST;
        if (IsKeyDown(KEY_D)) acceleration.x += SIDE_THRUST;
        if (IsKeyDown(KEY_W))
        {
            acceleration.y -= THRUST;
            gThrusting = true;
        }
    }
    gPlayer->setAcceleration(acceleration);
}

// update
void update()
{
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    if (deltaTime > 0.25f) deltaTime = 0.25f;
    gTimeAccumulator += deltaTime;

    while (gTimeAccumulator >= FIXED_TIMESTEP)
    {
        gTimeAccumulator -= FIXED_TIMESTEP;
        if (gGameState != PLAYING) continue;

        Vector2 acceleration = gPlayer->getAcceleration();
        if (acceleration.x != 0.0f || acceleration.y != GRAVITY)
        {
            gFuel -= FUEL_RATE * FIXED_TIMESTEP;
            if (gFuel <= 0.0f)
            {
                gFuel = 0.0f;
                gThrusting = false;
                gPlayer->setAcceleration({0.0f, GRAVITY});
            }
        }

        Vector2 movingPosition = gPlatforms[2].getPosition();
        movingPosition.x += gPlatformDirection * PLATFORM_SPEED * FIXED_TIMESTEP;
        if (movingPosition.x >= 1000.0f)
        {
            movingPosition.x = 1000.0f;
            gPlatformDirection = -1;
        }
        else if (movingPosition.x <= 280.0f)
        {
            movingPosition.x = 280.0f;
            gPlatformDirection = 1;
        }
        gPlatforms[2].setPosition(movingPosition);

        gPlayer->update(FIXED_TIMESTEP, gPlatforms, PLATFORM_COUNT);

        if (gPlayer->isCollidingTop() || gPlayer->isCollidingLeft() || gPlayer->isCollidingRight())
            gGameState = MISSION_FAILED;
        else if (gPlayer->isCollidingBottom())
            gGameState = MISSION_ACCOMPLISHED;

        Vector2 playerPosition = gPlayer->getPosition();
        Vector2 playerSize = gPlayer->getColliderDimensions();
        if (playerPosition.y + playerSize.y / 2.0f >= LAVA_TOP ||
            playerPosition.x - playerSize.x / 2.0f <= 0.0f ||
            playerPosition.x + playerSize.x / 2.0f >= SCREEN_WIDTH ||
            playerPosition.y - playerSize.y / 2.0f <= 0.0f)
            gGameState = MISSION_FAILED;

        gFlame->setPosition({playerPosition.x, playerPosition.y + 45.0f});
        gFlame->update(FIXED_TIMESTEP);
    }
}

// render
void render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(gBackground,
        {0.0f, 0.0f, static_cast<float>(gBackground.width), static_cast<float>(gBackground.height)},
        {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT)},
        {0.0f, 0.0f}, 0.0f, WHITE);

    for (int i = 0; i < PLATFORM_COUNT; i++) gPlatforms[i].render();

    if (gThrusting && gGameState == PLAYING) gFlame->render();

    gPlayer->render();

    Vector2 playerPosition = gPlayer->getPosition();
    Vector2 playerVelocity = gPlayer->getVelocity();
    Vector2 playerAcceleration = gPlayer->getAcceleration();

    float height = LAVA_TOP - (playerPosition.y + gPlayer->getColliderDimensions().y / 2.0f);

    if (height < 0.0f) height = 0.0f;

    DrawText(TextFormat("Fuel: %.1f%%", gFuel), 24, 24, 22, RAYWHITE);

    DrawText(TextFormat("Height: %.1f", height), 24, 56, 22, RAYWHITE);

    DrawText(TextFormat("Vertical Speed: %.1f", playerVelocity.y), 24, 88, 22, RAYWHITE);

    DrawText(TextFormat("Horizontal Speed: %.1f", playerVelocity.x), 24, 120, 22, RAYWHITE);

    DrawText(TextFormat("Horizontal Acceleration: %.1f", playerAcceleration.x), 24, 152, 22, RAYWHITE);

    if (gGameState == MISSION_ACCOMPLISHED)
    {
        DrawText("MISSION ACCOMPLISHED", 370, 80, 38, GREEN);
        DrawText("Press R to restart or Q to quit", 405, 132, 23, RAYWHITE);
    }

    else if (gGameState == MISSION_FAILED)
    {
        DrawText("MISSION FAILED", 478, 80, 38, RED);
        DrawText("Press R to restart or Q to quit", 405, 132, 23, RAYWHITE);
    }

    EndDrawing();
}

// shutdown
void shutdown()
{
    delete gPlayer;
    delete gFlame;
    delete[] gPlatforms;
    UnloadTexture(gBackground);
    CloseWindow();
}

// main
int main(void)
{
    initialise();
    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }
    shutdown();
    return 0;
}