#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light
{
public:
	glm::vec3 position;
	glm::vec3 lightIntansity;
	glm::mat4x4 model;
	glm::mat4x4 view;
	glm::mat4x4 projection;

};
