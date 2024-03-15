#pragma once
#include <GLAD/glad/glad.h>
#include "Texture.h"

class FrameBuffer
{
private:
	GLuint width, height;
public:
	GLuint fbo;
	GLuint rbo;

	FrameBuffer();
	
	void binding();
	void unBinding();
	void use();
	void bufferStorage(const GLint& width, const GLint& height, const GLenum& e = GL_DEPTH24_STENCIL8);
	void BindTexture(const Texture2D& texture, const GLenum& e = GL_COLOR_ATTACHMENT0);
	void check();

	~FrameBuffer();
};
