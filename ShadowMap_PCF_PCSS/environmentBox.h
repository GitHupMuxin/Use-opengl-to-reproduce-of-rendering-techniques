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
	virtual void bindTexture(const std::vector<std::string>& faces);
	virtual void bindTexture(const std::string& path);
	virtual void Draw();
};

class EnvironmentBoxWithSphericalHarmonics : public EnvironmentBox
{
private:
	GLint SHOrder;
	std::vector<GLfloat> lightCoeffs;

	void loadLightCoeffs(const std::string& fileName);
public:
	EnvironmentBoxWithSphericalHarmonics(const std::string& fileName);
	std::vector<GLfloat>& getLightCoeffs();
};

typedef EnvironmentBox EBox;
typedef EnvironmentBoxWithSphericalHarmonics SHEBox;
