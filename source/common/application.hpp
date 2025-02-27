#pragma once

#include <glm/vec2.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <string>
#include <unordered_map>
#include <type_traits>
#include <json/json.hpp>
#include <vector>
#include <stb/stb_image.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

namespace our
{

    // This struct handles window attributes: (title, size, isFullscreen).
    struct WindowConfiguration
    {
        std::string title;
        glm::i16vec2 size;
        bool isFullscreen;
    };

    class Application; // Forward declaration

    // This is the base class for all states
    // The application will be responsible for managing all scene functionality by calling the "on*" functions.
    class State
    {
        // Each scene will have a pointer to the application that owns it
        Application *application;
        friend Application;

    public:
        virtual std::string getStateName() = 0;
        virtual void onInitialize() {}           // Called once before the game loop.
        virtual void onImmediateGui() {}         // Called every frame to draw the Immediate GUI (if any).
        virtual void onDraw(double deltaTime) {} // Called every frame in the game loop passing the time taken to draw the frame "Delta time".
        virtual void onDestroy() {}              // Called once after the game loop ends for house cleaning.

        // Override these functions to get mouse and keyboard event.
        virtual void onKeyEvent(int key, int scancode, int action, int mods) {}
        virtual void onCursorMoveEvent(double x, double y) {}
        virtual void onCursorEnterEvent(int entered) {}
        virtual void onMouseButtonEvent(int button, int action, int mods) {}
        virtual void onScrollEvent(double x_offset, double y_offset) {}

        // Returns a pointer
        Application *getApp() { return application; }
    };

    // This class act as base class for all the Applications covered in the examples.
    // It offers the functionalities needed by all the examples.
    class Application
    {
    protected:
        GLFWwindow *window = nullptr; // Pointer to the window created by GLFW using "glfwCreateWindow()".

        Keyboard keyboard; // Instance of "our" keyboard class that handles keyboard functionalities.
        Mouse mouse;       // Instance of "our" mouse class that handles mouse functionalities.

        std::vector<nlohmann::json> configs; // A Json file that contains all application configuration

        std::unordered_map<std::string, State *> states; // This will store all the states that the application can run
        State *previousState = nullptr;
        State *currentState = nullptr; // This will store the current scene that is being run
        State *nextState = nullptr;    // If it is requested to go to another scene, this will contain a pointer to that scene

        // Virtual functions to be overrode and change the default behaviour of the application
        // according to the example needs.
        virtual void configureOpenGL();                       // This function sets OpenGL Window Hints in GLFW.
        virtual WindowConfiguration getWindowConfiguration(); // Returns the WindowConfiguration current struct instance.

        virtual void setupCallbacks(); // Sets-up the window callback functions from GLFW to our (Mouse/Keyboard) classes.

    public:
        // Create an application with following configuration
        Application(const std::vector<nlohmann::json> &configs) : configs(configs) {}
        // On destruction, delete all the states
        ~Application()
        {
            for (auto &it : states)
                delete it.second;
        }

        // This is the main class function that run the whole application (Initialize, Game loop, House cleaning).
        int run(int run_for_frames = 0);

        // Register a state for use by the application
        // The state is uniquely identified by its name
        // If the name is already used, the old name owner is deleted and the new state takes its place
        template <typename T>
        void registerState(std::string name)
        {
            static_assert(std::is_base_of<State, T>::value, "T must derive from our::State");
            auto it = states.find(name);
            if (it != states.end())
            {
                delete it->second;
            }
            State *scene = new T();
            scene->application = this;
            states[name] = scene;
        }

        // Tells the application to change its current state
        // The change will not be applied until the current frame ends
        void changeState(std::string name)
        {
            auto it = states.find(name);
            if (it != states.end())
            {
                previousState = currentState;
                nextState = it->second;
            }
        }

        void printTextCenter(std::string text, int height, float textSize)
        {

            ImGuiIO &io = ImGui::GetIO();

            io.FontGlobalScale = textSize;

            WindowConfiguration windowConfiguration = this->getWindowConfiguration();
            float window_width = windowConfiguration.size.x; // Get the width of the window
            float text_width = (float) text.size();                  // Get the width of the text

            ImVec2 text_pos = ImVec2((float)(window_width / 2 - 7 * io.FontGlobalScale * (text_width / 2)), (float)height); // Screen coordinates
            ImU32 text_color = IM_COL32(255, 255, 255, 255);                                                       // White color

            ImGui::GetBackgroundDrawList()->AddText(text_pos, text_color, text.c_str());
            ImGui::Render();
        }

        void printTextLeft(std::string text, int height, float textSize)
        {

            ImGuiIO &io = ImGui::GetIO();

            // ImFont *large_font = io.Fonts->AddFontFromFileTTF("./assets/fonts/Jost-700-Bold.ttf", 3.0f); // Large font
            // ImGui::PushFont(large_font);

            io.FontGlobalScale = textSize;

            WindowConfiguration windowConfiguration = this->getWindowConfiguration();
            float window_width = windowConfiguration.size.x; // Get the width of the window
            float text_width = (float)text.size();                  // Get the width of the text

            ImVec2 text_pos = ImVec2(10 + 304, (float)height);      // Screen coordinates
            ImU32 text_color = IM_COL32(255, 255, 255, 255); // White color

            ImGui::GetBackgroundDrawList()->AddText(text_pos, text_color, text.c_str());
            ImGui::Render();
        }

        void printTextRight(std::string text, int height, float textSize)
        {

            ImGuiIO &io = ImGui::GetIO();

            // ImFont *large_font = io.Fonts->AddFontFromFileTTF("./assets/fonts/Jost-700-Bold.ttf", 3.0f); // Large font
            // ImGui::PushFont(large_font);

            io.FontGlobalScale = textSize;

            WindowConfiguration windowConfiguration = this->getWindowConfiguration();
            float window_width = windowConfiguration.size.x; // Get the width of the window
            float text_width = (float)text.size();                  // Get the width of the text

            ImVec2 text_pos = ImVec2((float)(window_width - 7 * io.FontGlobalScale * text_width - 10 - 310), (float)height); // Screen coordinates
            ImU32 text_color = IM_COL32(255, 255, 255, 255);                                                        // White color

            ImGui::GetBackgroundDrawList()->AddText(text_pos, text_color, text.c_str());
            ImGui::Render();
        }

        GLuint GenerateSimpleTexture(int width, int height, const unsigned char *data)
        {
            GLuint texture_id;
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);

            // Setup texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Upload texture data
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            glBindTexture(GL_TEXTURE_2D, 0);

            return texture_id;
        }
        void drawTimer()
        {

            std::string timerImageLocation = "./assets/textures/timer.png";
            glm::ivec2 timerImageSize;
            int channels;
            unsigned char *timerImageData = stbi_load(timerImageLocation.c_str(), &timerImageSize.x, &timerImageSize.y, &channels, 4);

            GLuint texture_id = GenerateSimpleTexture(timerImageSize.x, timerImageSize.y, timerImageData);
            ImVec2 image_pos(350, 250); // Position for the image

            // ImGui::GetBackgroundDrawList()->AddImage(
            //     (void *)(intptr_t)texture_id,
            //     image_pos,
            //     ImVec2(image_pos.x + timerImageSize.x, image_pos.y + timerImageSize.y));
            ImGui::Render();
        }
        // Closes the Application
        void close()
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Class Getters.
        GLFWwindow *getWindow() { return window; }
        [[nodiscard]] const GLFWwindow *getWindow() const { return window; }
        Keyboard &getKeyboard() { return keyboard; }
        [[nodiscard]] const Keyboard &getKeyboard() const { return keyboard; }
        Mouse &getMouse() { return mouse; }
        [[nodiscard]] const Mouse &getMouse() const { return mouse; }

        [[nodiscard]] const nlohmann::json &getConfig(unsigned int configNumber = 0) const
        {
            if (configNumber < configs.size())
                return configs[configNumber];
            return configs[0];
        }

        // Get the size of the frame buffer of the window in pixels.
        glm::ivec2 getFrameBufferSize()
        {
            glm::ivec2 size;
            glfwGetFramebufferSize(window, &(size.x), &(size.y));
            return size;
        }

        // Get the window size. In most cases, it is equal to the frame buffer size.
        // But on some platforms, the framebuffer size may be different from the window size.
        glm::ivec2 getWindowSize()
        {
            glm::ivec2 size;
            glfwGetWindowSize(window, &(size.x), &(size.y));
            return size;
        }

        std::string getPrevStateName()
        {
            return previousState->getStateName();
        }
    };
}
