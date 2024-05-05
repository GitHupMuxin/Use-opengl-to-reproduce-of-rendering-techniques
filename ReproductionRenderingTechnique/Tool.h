#pragma once
#include <GLAD/glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xpos, double ypos);

void processInput(GLFWwindow* window);