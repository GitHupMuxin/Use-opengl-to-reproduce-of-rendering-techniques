#pragma once
#include <GLAD/glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Vertex.h"
#include "Shader.h"

class Light
{
public:
	glm::vec3 position;
	glm::vec3 lightIntansity;
	glm::mat4x4 view;
	glm::mat4x4 projection;

};

class PointLightRenderModel
{
private:
	std::vector<GLfloat> verteices;
public:
	glm::mat4x4 model;
	GLuint VAO, VBO;
	Shader shader;
	PointLightRenderModel();
	void Draw();
};
