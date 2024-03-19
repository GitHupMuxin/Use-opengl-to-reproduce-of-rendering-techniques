#pragma once
#include <GLAD/glad/glad.h>
#include <vector>
#include "Shader.h"

class EnvironmentBox
{
private:
	std::vector<GLfloat> vertices;
public:
	GLuint textureID;
	GLuint VAO, VBO;
	Shader shader;
	EnvironmentBox();
	void bindTexture(std::vector<std::string> faces);
	void Draw();
};
