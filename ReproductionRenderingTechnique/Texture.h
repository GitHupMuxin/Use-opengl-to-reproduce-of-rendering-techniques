#pragma once
#include <GLAD/glad/glad.h>
#include <iostream>
#include <vector>
#include "Vertex.h"
#include "Shader.h"

class Texture2D
{
public:
	GLuint id;
	GLint width, height;
	GLenum format;
	void init(const GLchar* path, const GLenum& e);
	void generateMipmap();
	virtual void init(const GLenum& format, const GLenum& saveFromat = GL_RGBA, const GLenum& saveType = GL_UNSIGNED_BYTE);
	virtual void Parameteri(const GLenum& target, const GLenum& value);
	virtual void Parameteri(const std::vector<GLenum>& enumVec);
	virtual void use() const;
	virtual	void unUse() const;

	Texture2D();
	Texture2D(const GLuint& ID);
	Texture2D(const GLchar* path, const GLenum& e = GL_RGB);
	Texture2D(const GLint& Width, const GLint& Height, const GLenum& Fromat = GL_RGB, const GLenum& saveFromat = GL_RGBA, const GLenum& saveType = GL_UNSIGNED_BYTE);
};

class CubeTexture
{
public:
	GLuint id;
	GLint width, height;
	GLenum format;
	void init(const std::vector<std::string>& faces, const GLenum& e);
	void generateMipmap();
	virtual void init(const GLenum& format, const GLenum& saveFromat = GL_RGBA, const GLenum& saveType = GL_UNSIGNED_BYTE);
	virtual void Parameteri(const GLenum& target, const GLenum& value);
	virtual void use() const;
	virtual	void unUse() const;

	CubeTexture();
	CubeTexture(const GLuint& ID);
	CubeTexture(const std::vector<std::string>& faces, const GLenum& e = GL_RGB);
	CubeTexture(const GLint& Width, const GLint& Height, const GLenum& Fromat = GL_RGB, const GLenum& saveFromat = GL_RGBA, const GLenum& saveType = GL_UNSIGNED_BYTE);
};

class Texture2DMultisample : public Texture2D
{
private:
	void init();
public:
	GLuint samples;
	Texture2DMultisample(const GLint& Width, const GLint& Height, const GLint& Samples, const GLenum& Fromat = GL_RGB);
	void use() const;
	void unUse() const;
};




class ModelTexture : public Texture2D
{
public:
	std::string type;
	std::string path;
};

class ScreenRenderModel
{
	std::vector<Vertex> verteices;
	std::vector<GLuint> indeices;
public:	
	GLuint VAO, VBO, EBO;
	Shader shader;
	ScreenRenderModel();
	void Draw(const Texture2D& texture, const Shader& shader);
	void Draw(const Texture2D& texture, const GLint& level, const Shader& shader);
	void Draw(const Shader& shader);
};

