#pragma once

#include <application.hpp>
#include <shader/shader.hpp>
#include <texture/texture2d.hpp>
#include <texture/texture-utils.hpp>
#include <material/material.hpp>
#include <mesh/mesh.hpp>
#include "./level1.hpp"
#include "./level2.hpp"
#include "./level3.hpp"
#include "./level4.hpp"

#include <functional>
#include <array>

// This struct is used to store the location and size of a button and the code it should execute when clicked
struct levelButton
{
    // The position (of the top-left corner) of the button and its size in pixels
    glm::vec2 position, size;
    // The function that should be excuted when the button is clicked. It takes no arguments and returns nothing.
    std::function<void()> action;

    // This function returns true if the given vector v is inside the button. Otherwise, false is returned.
    // This is used to check if the mouse is hovering over the button.
    bool isInside(const glm::vec2 &v) const
    {
        return position.x <= v.x && position.y <= v.y &&
               v.x <= position.x + size.x &&
               v.y <= position.y + size.y;
    }

    // This function returns the local to world matrix to transform a rectangle of size 1x1
    // (and whose top-left corner is at the origin) to be the button.
    glm::mat4 getLocalToWorld() const
    {
        return glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f)) *
               glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
    }
};

// This state shows how to use some of the abstractions we created to make a menu.
class LevelSelectState : public our::State
{

    // A meterial holding the menu shader and the menu texture to draw
    our::TexturedMaterial *menuMaterial;
    // A material to be used to highlight hovered buttons (we will use blending to create a negative effect).
    our::TintedMaterial *highlightMaterial;
    // A rectangle mesh on which the menu material will be drawn
    our::Mesh *rectangle;
    // A variable to record the time since the state is entered (it will be used for the fading effect).
    float time;
    // An array of the button that we can interact with
    std::array<levelButton, 6> buttons;

    bool levelSelected = false;
    bool soundCheck = true;

    void onInitialize() override
    {
        // First, we create a material for the menu's background

        soundSystem->playCurrentSound();
        menuMaterial = new our::TexturedMaterial();
        // Here, we load the shader that will be used to draw the background
        menuMaterial->shader = new our::ShaderProgram();
        menuMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        menuMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        menuMaterial->shader->link();
        // Then we load the menu texture
        menuMaterial->texture = our::texture_utils::loadImage("assets/textures/levelSelect.png");
        // Initially, the menu material will be black, then it will fade in
        menuMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        // Second, we create a material to highlight the hovered buttons
        highlightMaterial = new our::TintedMaterial();
        // Since the highlight is not textured, we used the tinted material shaders
        highlightMaterial->shader = new our::ShaderProgram();
        highlightMaterial->shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER);
        highlightMaterial->shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER);
        highlightMaterial->shader->link();
        // The tint is white since we will subtract the background color from it to create a negative effect.
        highlightMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        // To create a negative effect, we enable blending, set the equation to be subtract,
        // and set the factors to be one for both the source and the destination.
        highlightMaterial->pipelineState.blending.enabled = true;
        highlightMaterial->pipelineState.blending.equation = GL_FUNC_SUBTRACT;
        highlightMaterial->pipelineState.blending.sourceFactor = GL_ONE;
        highlightMaterial->pipelineState.blending.destinationFactor = GL_ONE;

        // Then we create a rectangle whose top-left corner is at the origin and its size is 1x1.
        // Note that the texture coordinates at the origin is (0.0, 1.0) since we will use the
        // projection matrix to make the origin at the the top-left corner of the screen.
        rectangle = new our::Mesh({
                                      {{0.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                      {{1.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                      {{1.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                      {{0.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                  },
                                  {
                                      0,
                                      1,
                                      2,
                                      2,
                                      3,
                                      0,
                                  });

        // Reset the time elapsed since the state is entered.
        time = 0;
        soundCheck = true;

        // Fill the positions, sizes and actions for the menu buttons
        // Note that we use lambda expressions to set the actions of the buttons.
        // A lambda expression consists of 3 parts:
        // - The capture list [] which is the variables that the lambda should remember because it will use them during execution.
        //      We store [this] in the capture list since we will use it in the action.
        // - The argument list () which is the arguments that the lambda should receive when it is called.
        //      We leave it empty since button actions receive no input.
        // - The body {} which contains the code to be executed.
        buttons[0].position = {180.0f, 157.0f};
        buttons[0].size = {293.0f, 185.0f};
        buttons[0].action = [this]()
        {
            menuMaterial->texture = our::texture_utils::loadImage("assets/textures/LoadingScreen.png");
            levelSelected = true;
            this->getApp()->changeState(Level1state::getStateName_s());
        };

        buttons[1].position = {483.0f, 157.0f};
        buttons[1].size = {292.0f, 185.0f};
        buttons[1].action = [this]()
        {
            menuMaterial->texture = our::texture_utils::loadImage("assets/textures/LoadingScreen.png");
            levelSelected = true;
            this->getApp()->changeState(Level2state::getStateName_s());
        };

        buttons[2].position = {786.0f, 157.0f};
        buttons[2].size = {292.0f, 185.0f};
        buttons[2].action = [this]()
        {
            menuMaterial->texture = our::texture_utils::loadImage("assets/textures/LoadingScreen.png");
            levelSelected = true;
            this->getApp()->changeState(Level3state::getStateName_s());
        };

        buttons[3].position = {226.0f, 353.0f};
        buttons[3].size = {262.0f, 166.0f};
        buttons[3].action = [this]()
        {
            menuMaterial->texture = our::texture_utils::loadImage("assets/textures/LoadingScreen.png");
            levelSelected = true;
            this->getApp()->changeState(Level4state::getStateName_s());
        };

        buttons[4].position = {498.0f, 353.0f};
        buttons[4].size = {265.0f, 166.0f};
        buttons[4].action = [this]()
        {
            menuMaterial->texture = our::texture_utils::loadImage("assets/textures/LoadingScreen.png");
            levelSelected = true;
            this->getApp()->changeState("level1");
        };

        buttons[5].position = {771.0f, 353.0f};
        buttons[5].size = {262.0f, 166.0f};
        buttons[5].action = [this]()
        {
            menuMaterial->texture = our::texture_utils::loadImage("assets/textures/LoadingScreen.png");
            levelSelected = true;
            this->getApp()->changeState("level1");
        };
    }

    void onDraw(double deltaTime) override
    {

        if (soundCheck)
            soundSystem->playCurrentSound();

        // Get a reference to the keyboard object
        auto &keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape key is pressed in this frame, return to loading screen
            getApp()->changeState("menu");
        }
        else if (keyboard.justPressed(GLFW_KEY_N))
        {
            soundSystem->playNextSound();
        }
        else if (keyboard.justPressed(GLFW_KEY_P))
        {
            soundSystem->playPreviousSound();
        }
        else if (keyboard.justPressed(GLFW_KEY_R))
        {
            soundSystem->playCurrentSound();
        }
        else if (keyboard.justPressed(GLFW_KEY_S))
        {
            soundSystem->stopAllSounds();
            soundCheck = false;
        }

        // Get a reference to the mouse object and get the current mouse position
        auto &mouse = getApp()->getMouse();
        glm::vec2 mousePosition = mouse.getMousePosition();

        // If the mouse left-button is just pressed, check if the mouse was inside
        // any menu button. If it was inside a menu button, run the action of the button.
        if (mouse.justPressed(0))
        {
            for (auto &button : buttons)
            {
                if (button.isInside(mousePosition))
                    button.action();
            }
        }

        // Get the framebuffer size to set the viewport and the create the projection matrix.
        glm::ivec2 size = getApp()->getFrameBufferSize();
        // Make sure the viewport covers the whole size of the framebuffer.
        glViewport(0, 0, size.x, size.y);

        // The view matrix is an identity (there is no camera that moves around).
        // The projection matrix applys an orthographic projection whose size is the framebuffer size in pixels
        // so that the we can define our object locations and sizes in pixels.
        // Note that the top is at 0.0 and the bottom is at the framebuffer height. This allows us to consider the top-left
        // corner of the window to be the origin which makes dealing with the mouse input easier.
        glm::mat4 VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        // The local to world (model) matrix of the background which is just a scaling matrix to make the menu cover the whole
        // window. Note that we defind the scale in pixels.
        glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        // First, we apply the fading effect.
        time += (float)deltaTime;
        menuMaterial->tint = glm::vec4(glm::smoothstep(0.00f, 2.00f, time));
        // Then we render the menu background
        // Notice that I don't clear the screen first, since I assume that the menu rectangle will draw over the whole
        // window anyway.
        menuMaterial->setup();
        menuMaterial->shader->set("transform", VP * M);
        rectangle->draw();

        // For every button, check if the mouse is inside it. If the mouse is inside, we draw the highlight rectangle over it.
        for (auto &button : buttons)
        {
            if (button.isInside(mousePosition))
            {
                if (!levelSelected)
                {
                    highlightMaterial->setup();
                    highlightMaterial->shader->set("transform", VP * button.getLocalToWorld());
                    rectangle->draw();
                }
                else
                {
                    levelSelected = false;
                }
            }
        }
    }

    void onDestroy() override
    {
        // Delete all the allocated resources
        delete rectangle;
        delete menuMaterial->texture;
        delete menuMaterial->shader;
        delete menuMaterial;
        delete highlightMaterial->shader;
        delete highlightMaterial;
    }

public:
    static std::string getStateName_s()
    {
        return "level-select";
    }

    std::string getStateName()
    {
        return "level-select";
    }
};