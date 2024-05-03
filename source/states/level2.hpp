#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/player-controller.hpp>
#include <systems/collision-detector.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>

#include "../common/ecs/transform.hpp"
#include <../common/systems/sound/sound.hpp>

#include <unordered_set>
#include <iostream>

// This state shows how to use the ECS framework and deserialization.
class Level2state : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::PlayerControllerSystem playerController;
    our::MovementSystem movementSystem;
    our::CollisionSystem collisionSystem;
    bool bombExplodes = false;
    bool goalScore = false;

    bool timeUp = false;

    int minutes = 0;
    int seconds = 30;
    int lives = 3;
    int goals = 0;

    time_t previousTime;
    time_t currentTime;

    int countDownTime = 3;
    bool countDownState = true;

    AudioPlayer soundSystem;

    void onInitialize() override
    {

        previousTime = time(NULL);
        countDownState = true;
        timeUp = false;
        countDownTime = 3;
        lives = 3;
        goals = 0;
        minutes = 0;
        seconds = 30;
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig(2)["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if (config.contains("world"))
        {
            world.deserialize(config["world"]);
        }

        // TODO: remove this if not used
        // world.focusCamera();

        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        playerController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);

        soundSystem.playSound("1", "./assets/audio/countdown.mp3");
    }

    void decreaseTime()
    {

        int totalTime = seconds + minutes * 60;
        totalTime--;

        if (totalTime == 0)
            timeUp = true;

        seconds = totalTime % 60;
        minutes = totalTime / 60;
    }

    void handleTimer()
    {
        std::string minutesString;
        std::string secondsString;

        if (minutes <= 9)
        {
            minutesString = "0";
        }
        minutesString += std::to_string(minutes);

        if (seconds <= 9)
        {
            secondsString = "0";
        }

        secondsString += std::to_string(seconds);

        getApp()->printTextCenter(minutesString + ":" + secondsString, 10, 3);
    }

    void handleCountDown()
    {
        if (countDownTime > 0)
        {
            countDownTime--;
        }
        else
        {
            countDownState = false;
        }
    }

    void onDraw(double deltaTime) override
    {

        currentTime = time(NULL);
        if (currentTime - previousTime > 1)
        {
            previousTime = time(NULL);
            if (!countDownState)
                decreaseTime();
            else
                handleCountDown();
        }

        if (countDownState)
        {
            getApp()->printTextCenter(std::to_string(countDownTime), 720 / 2, 6.0f);
        }
        handleTimer();

        getApp()->printTextLeft("Remaining Lives " + std::to_string(lives), 10, 3);
        getApp()->printTextRight("Goals " + std::to_string(goals), 10, 3);

        // Here, we just run a bunch of systems to control the world logic

        if (!countDownState)
        {
            collisionSystem.checkForCollisions(&world);
            bombExplodes = collisionSystem.checkForBombCollision(&world);
            goalScore = collisionSystem.checkForGoal(&world);
            playerController.update(&world, (float)deltaTime);
            movementSystem.update(&world, (float)deltaTime);
            cameraController.update(&world, (float)deltaTime);
            // And finally we use the renderer system to draw the scene

            // Get a reference to the keyboard object
        }
        renderer.render(&world);

        auto &keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape  key is pressed in this frame, go to the menu state
            getApp()->changeState("menu");
        }

        if (bombExplodes)
        {
            lives--;
            bombExplodes = false;
            soundSystem.playSound("4", "./assets/audio/bomb.mp3");
            unordered_set<our::Entity *> entities = world.getEntities();
            for (our::Entity *entity : entities)
            {
                entity->localTransform.position = entity->localTransform.initialPositionNew;
                entity->localTransform.rotation = entity->localTransform.initialRotationNew;
                entity->localTransform.scale = entity->localTransform.initialScaleNew;
            }

            vector<our::MovementComponent *> movements;
            for (our::Entity *entity : entities)
            {
                our::MovementComponent *movement = entity->getComponent<our::MovementComponent>();
                if (movement == nullptr)
                    continue;
                movements.push_back(movement);
            }
            for (unsigned int i = 0; i < movements.size(); i++)
            {
                movements[i]->current_velocity = 0;
            }
        }
        if (goalScore)
        {
            goals++;
            goalScore = false;
            soundSystem.playSound("2", "./assets/audio/sui.mp3");
            unordered_set<our::Entity *> entities = world.getEntities();
            for (our::Entity *entity : entities)
            {
                entity->localTransform.position = entity->localTransform.initialPositionNew;
                entity->localTransform.rotation = entity->localTransform.initialRotationNew;
                entity->localTransform.scale = entity->localTransform.initialScaleNew;
            }

            vector<our::MovementComponent *> movements;
            for (our::Entity *entity : entities)
            {
                our::MovementComponent *movement = entity->getComponent<our::MovementComponent>();
                if (movement == nullptr)
                    continue;
                movements.push_back(movement);
            }
            for (unsigned int i = 0; i < movements.size(); i++)
            {
                movements[i]->current_velocity = 0;
            }
        }

        if (timeUp)
        {
            countDownState = true;
            // soundSystem.playSound("5", "./assets/audio/messi_lose.mp3");

            getApp()->changeState("loading-screen");
        }
        if (lives <= 0)
        {
            countDownState = true;
            // soundSystem.playSound("5", "./assets/audio/messi_lose.mp3");

            getApp()->changeState("loading-screen");
        }
        if (goals >= 3)
        {
            countDownState = true;
            // soundSystem.playSound("3", "./assets/audio/messi.mp3");

            getApp()->changeState("loading-screen");
        }
    }

    void onDestroy() override
    {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        playerController.exit();
        // Clear the world
        world.clear();
        countDownState = true;
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }
};