#pragma once
#include <GLAD/glad/glad.h>
#include <vector>
#include "material.h"

class Triangle
{
public:
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	std::vector<GLuint> positionIndex;
	Triangle();
	Triangle(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord);
	Triangle(float p[]);
};

class ObjectMesh
{
public:
	std::string meshName;
	GLuint materialIndex;
	std::vector<Triangle> triangle;
};


