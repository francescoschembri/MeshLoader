#pragma once

#include "StatusManager.h"

#include <GLFW/glfw3.h>

constexpr int SCREEN_INITIAL_WIDTH = 800;
constexpr int SCREEN_INITIAL_HEIGHT = 800;

// keys
//wireframe
constexpr int WIREFRAME_KEY = GLFW_KEY_W;
//camera:
constexpr int RESET_CAMERA_KEY = GLFW_KEY_R;
constexpr int CAMERA_UP_KEY = GLFW_KEY_UP;
constexpr int CAMERA_DOWN_KEY = GLFW_KEY_DOWN;
constexpr int CAMERA_LEFT_KEY = GLFW_KEY_LEFT;
constexpr int CAMERA_RIGHT_KEY = GLFW_KEY_RIGHT;
//animation control
constexpr int PAUSE_KEY = GLFW_KEY_P;
//undo and redo
constexpr int UNDO_CHANGE_KEY = GLFW_KEY_Z;
constexpr int REDO_CHANGE_KEY = GLFW_KEY_Y;
//visual debugging
constexpr int INCREASE_CURR_BONE_ID = GLFW_KEY_SPACE;
constexpr int DECREASE_CURR_BONE_ID = GLFW_KEY_ENTER;

GLFWwindow* CreateWindow();

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void on_mouse_click_callback(GLFWwindow* window, int button, int action, int mods);

//mouse movement
inline void (*process_mouse_movement)(GLFWwindow*, float, float);
void rotate(GLFWwindow* window, float xpos, float ypos);
void update_mouse_last_pos(GLFWwindow* window, float xpos, float ypos);
void picking(GLFWwindow* window, float xpos, float ypos);
void tweak(GLFWwindow* window, float xpos, float ypos);