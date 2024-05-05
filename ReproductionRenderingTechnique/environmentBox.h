#pragma once
#include <GLAD/glad/glad.h>
#include <vector>
#include "Shader.h"

class BOXModel
{
protected:
	std::vector<GLfloat> vertices;
public:
	GLuint VAO, VBO;
	BOXModel();
	inline const std::vector<GLfloat>& getVertices() { return this->vertices; }
	virtual void Draw(const GLuint textureID, const Shader& shader, GLint lob = 0);
	virtual void render();
};


class EnvironmentBox
{
public:
	BOXModel model;
	GLuint textureID;
	Shader shader;
	EnvironmentBox();
	virtual void generateMipmap();
	virtual void bindTexture(const std::vector<std::string>& faces);
	virtual void bindTexture(const std::string& path);
	virtual void Draw(int textureLob = 0);
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
