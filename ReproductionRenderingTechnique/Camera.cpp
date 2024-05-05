#include "Camera.h"
#include "Tool.h"

#define MY_PI 3.1415926

Camera::Camera() { }

Camera::Camera(const glm::vec3& Position, const glm::vec3& Traget, const glm::vec3& Up, const GLfloat& Fov, const GLfloat& SpeedWeight)
	: position(Position), traget(Traget), up(glm::normalize(Up)), fov(Fov), speedWeight(SpeedWeight)
{
	this->front = glm::normalize(traget - position);
	this->left = glm::cross(up, front);
	this->up = glm::cross(front, this->left);
	this->pitch = asin(this->front.y) / MY_PI * 180.0f;
	this->yaw = asin(this->front.z / cos(glm::radians(this->pitch))) * 180.0f / MY_PI;
	this->len = glm::length(traget - position);
}

void Camera::init(GLFWwindow* window)
{
	this->window = window;
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

float Camera::getCameraSpeed()
{
	return static_cast<GLfloat>(speedWeight);
}

glm::mat4 Camera::getViewMat4()
{
	return glm::lookAt(this->position, this->traget, this->up);
}


