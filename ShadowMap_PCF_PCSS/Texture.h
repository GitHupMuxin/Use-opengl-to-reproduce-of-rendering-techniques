#pragma once
#include <GLAD/glad/glad.h>
#include <iostream>
#include <vector>

class Texture2D
{
public:
	GLuint id;
	GLint width, height;
	void init(const GLenum& format);
	void init(const GLchar* path);
	void Parameteri(const GLenum& target, const GLenum& value);
	void Parameteri(const std::vector<GLenum>& enumVec);
	void use() const;
	void unUse() const;

	Texture2D();
	Texture2D(const GLchar* path);
	Texture2D(const GLint& Width, const GLint& Height, const GLenum& format = GL_RGB);
};


