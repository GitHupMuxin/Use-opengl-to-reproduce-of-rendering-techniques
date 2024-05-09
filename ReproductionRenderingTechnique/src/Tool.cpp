#include "../head/Tool.h"
#include "../head/Camera.h"

extern Camera camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool rightMouse = false;
	static bool leftMouse = false;
	if (rightMouse)
	{
		GLfloat deletaX = xpos - camera.lastX;
		GLfloat deletaY = ypos - camera.lastY;
		float mutiple = 0.002f;
		camera.position += deletaY * camera.up * mutiple;
		camera.position += deletaX * camera.left * mutiple;
		camera.traget += deletaY * camera.up * mutiple;
		camera.traget += deletaX * camera.left * mutiple;
		camera.front = glm::normalize(camera.traget - camera.position);
	}

	if (leftMouse)
	{
		GLfloat deletaX = camera.lastX - xpos;
		GLfloat deletaY = camera.lastY - ypos;
		camera.yaw += deletaX * 0.1f;
		camera.pitch += deletaY * 0.1f;
		camera.yaw = camera.yaw > 360.0f ? camera.yaw - 360.0f : camera.yaw < -360.0 ? camera.yaw + 360.0f : camera.yaw;
		camera.pitch = camera.pitch > 89.0f ? 89.0f : camera.pitch < -89.0f ? -89.0f : camera.pitch;
		GLfloat x = -cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		GLfloat y = sin(glm::radians(camera.pitch));
		GLfloat z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		camera.position = camera.traget - glm::normalize(glm::vec3(x, y, z)) * camera.len;
		camera.front = glm::normalize(glm::vec3(x, y, z));
		camera.left = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camera.front);
		camera.up = glm::cross(camera.front, camera.left);
	}
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		rightMouse = true;
		camera.lastX = xpos;
		camera.lastY = ypos;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		rightMouse = false;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		leftMouse = true;
		camera.lastX = xpos;
		camera.lastY = ypos;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		leftMouse = false;
	}
}

void scroll_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (camera.fov >= 1.0f && camera.fov <= 45.0f)
		camera.fov -= ypos;
	camera.fov = camera.fov <= 1.0f ? 1.0f : camera.fov >= 45.0f ? 45.0f : camera.fov;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}