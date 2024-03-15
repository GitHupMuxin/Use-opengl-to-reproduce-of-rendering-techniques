#pragma once
#include <GLAD/glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Camera
{
public:
	float lastX;
	float lastY;
	float yaw;
	float pitch;
	float fov;
	float len;
	float speedWeight;

	GLFWwindow* window;

	glm::vec3 position;
	glm::vec3 traget;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 left;

	Camera();
	Camera(const glm::vec3& Position, const glm::vec3& Traget, const glm::vec3& Up, const GLfloat& Fov, const GLfloat& SpeedWeight);
	float getCameraSpeed();
	glm::mat4 getViewMat4();
	void init(GLFWwindow* window);
};

