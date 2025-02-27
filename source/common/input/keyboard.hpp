#pragma once

#include <GLFW/glfw3.h>
#include <cstring>

namespace our
{

    // A convenience class to read keyboard input
    class Keyboard
    {
    private:
        bool enabled; // Is this class enabled (allowed to read user input)
        bool currentKeyStates[GLFW_KEY_LAST + 1];
        bool previousKeyStates[GLFW_KEY_LAST + 1];

    public:
        // Enable this object and capture current keyboard state from window
        void enable(GLFWwindow *window)
        {
            enabled = true;
            for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
            {
                currentKeyStates[key] = previousKeyStates[key] = glfwGetKey(window, key);
            }
        }

        // Disable this object and clear the state
        void disable()
        {
            // TODO: check this as i think this should be added here
            enabled = false;

            for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
            {
                currentKeyStates[key] = previousKeyStates[key] = false;
            }
        }

        // update the keyboard state (moves current frame state to become the previous frame state)
        void update()
        {
            if (!enabled)
                return;
            std::memcpy(previousKeyStates, currentKeyStates, sizeof(previousKeyStates));
        }

        // Event functions called from GLFW callbacks in "application.cpp"
        void keyEvent(int key, int, int action, int)
        {
            if (!enabled)
                return;
            if (action == GLFW_PRESS)
            {
                currentKeyStates[key] = true;
            }
            else if (action == GLFW_RELEASE)
            {
                currentKeyStates[key] = false;
            }
        }

        // nodiscard keyword indicates that the return value shouldn't be ignored and the compiler gives an error in this case

        // - Is the key currently pressed
        /// - you can find keys name in the doc
        /// - https://www.glfw.org/docs/3.3/group__keys.html
        [[nodiscard]] bool isPressed(int key) const { return currentKeyStates[key]; }

        // - Was the key unpressed in the previous frame but became pressed in the current frame
        /// - you can find keys name in the doc
        /// - https://www.glfw.org/docs/3.3/group__keys.html
        [[nodiscard]] bool justPressed(int key) const { return currentKeyStates[key] && !previousKeyStates[key]; }

        // -  Was the key pressed in the previous frame but became unpressed in the current frame
        /// - you can find keys name in the doc
        /// - https://www.glfw.org/docs/3.3/group__keys.html
        [[nodiscard]] bool justReleased(int key) const { return !currentKeyStates[key] && previousKeyStates[key]; }

        [[nodiscard]] bool isEnabled() const { return enabled; }
        void setEnabled(bool enabled, GLFWwindow *window)
        {
            if (this->enabled != enabled)
            {
                if (enabled)
                    enable(window);
                else
                    disable();
            }
        }

        // Check if any key is currently pressed
        [[nodiscard]] bool anyKeyPressed() const
        {
            for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
            {
                if (currentKeyStates[key] && key != GLFW_KEY_ESCAPE)
                {
                    return true;
                }
            }
            return false;
        }
    };

}
